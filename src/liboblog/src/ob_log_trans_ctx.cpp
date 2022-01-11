/**
 * Copyright (c) 2021 OceanBase
 * OceanBase CE is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan PubL v2.
 * You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PubL v2 for more details.
 */

#define USING_LOG_PREFIX OBLOG

#include "ob_log_trans_ctx.h"

#include "lib/allocator/ob_mod_define.h"        // ObModIds

#include "ob_log_part_mgr.h"                    // IObLogPartMgr
#include "ob_log_trans_ctx_mgr.h"               // IObLogTransCtxMgr

#define _TCTX_STAT(level, tag, args...) _OBLOG_LOG(level, "[STAT] [TRANS_CTX] " tag, ##args)
#define _TCTX_ISTAT(tag, args...) _TCTX_STAT(INFO, tag, ##args)
#define _TCTX_DSTAT(tag, args...) _TCTX_STAT(DEBUG, tag, ##args)

using namespace oceanbase::common;
using namespace oceanbase::transaction;

namespace oceanbase
{
namespace liboblog
{
TransCtxSortElement::TransCtxSortElement() :
    trans_ctx_host_(NULL),
    trans_id_(),
    global_trans_version_(OB_INVALID_VERSION)
{
}

TransCtxSortElement::~TransCtxSortElement()
{
  reset();
}

void TransCtxSortElement::reset()
{
  trans_ctx_host_ = NULL;
  trans_id_.reset();
  global_trans_version_ = OB_INVALID_VERSION;
}

int TransCtxSortElement::init(TransCtx &trans_ctx_host,
    const transaction::ObTransID &trans_id,
    const int64_t global_trans_version)
{
  int ret = OB_SUCCESS;

  if (OB_UNLIKELY(! trans_id.is_valid())
      || OB_UNLIKELY(OB_INVALID_VERSION == global_trans_version)) {
    LOG_ERROR("invalid argument", K(trans_id), K(global_trans_version));
  } else {
    trans_ctx_host_ = &trans_ctx_host;
    trans_id_ = trans_id;
    global_trans_version_ = global_trans_version;
  }

  return ret;
}

bool TransCtxSortElement::TransCtxCmp::operator() (const TransCtxSortElement &tx1, const TransCtxSortElement &tx2) const
{
  bool bool_ret = false;

  if (tx1.global_trans_version_ != tx2.global_trans_version_) {
    bool_ret = tx1.global_trans_version_ > tx2.global_trans_version_;
  } else {
    bool_ret = tx1.trans_id_.compare(tx2.trans_id_);
  }

  return bool_ret;
}

TransCtx::TransCtx() :
    host_(NULL),
    state_(TRANS_CTX_STATE_INVALID),
    trans_id_(),
    trans_id_str_(),
    major_version_str_(),
    trx_sort_elem_(),
    seq_(0),
    participants_(NULL),
    participant_count_(0),
    ready_participant_objs_(NULL),
    ready_participant_count_(0),
    br_committer_queue_cond_(),
    total_br_count_(0),
    committed_br_count_(0),
    valid_part_trans_task_count_(0),
    revertable_participant_count_(0),
    allocator_(ObModIds::OB_LOG_TRANS_CTX, PAGE_SIZE),
    lock_()
{}

TransCtx::~TransCtx()
{
}

void TransCtx::reset()
{
  destroy_participant_array_();

  host_ = NULL;
  state_ = TRANS_CTX_STATE_INVALID;
  trans_id_.reset();
  trans_id_str_.reset();
  major_version_str_.reset();
  trx_sort_elem_.reset();
  seq_ = 0;

  participants_ = NULL;
  participant_count_ = 0;
  ready_participant_objs_ = NULL;
  ready_participant_count_ = 0;

  total_br_count_ = 0;
  committed_br_count_ = 0;
  valid_part_trans_task_count_ = 0;
  revertable_participant_count_ = 0;

  allocator_.reset();
}

int TransCtx::set_trans_id(const transaction::ObTransID &trans_id)
{
  int ret = OB_SUCCESS;
  if (! trans_id.is_valid()) {
    ret = OB_INVALID_ARGUMENT;
  } else {
    trans_id_ = trans_id;
  }

  return ret;
}

int TransCtx::get_state() const
{
  return state_;
}

int TransCtx::set_state(const int target_state)
{
  int ret = OB_SUCCESS;
  if (! is_state_valid(target_state)) {
    ret = OB_INVALID_ARGUMENT;
  } else {
    state_ = static_cast<int8_t>(target_state);
  }

  return ret;
}

int TransCtx::set_ready_participant_count(const int64_t count)
{
  int ret = OB_SUCCESS;
  if (OB_UNLIKELY(count < 0)) {
    ret = OB_INVALID_ARGUMENT;
  } else {
    ready_participant_count_ = count;
  }

  return ret;
}

int64_t TransCtx::get_revertable_participant_count() const
{
  return revertable_participant_count_;
}

int TransCtx::set_ready_participant_objs(PartTransTask *part_trans_task)
{
  int ret = OB_SUCCESS;
  if (OB_ISNULL(part_trans_task)) {
    ret = OB_INVALID_ARGUMENT;
  } else {
    ready_participant_objs_ = part_trans_task;
  }

  return ret;
}

int TransCtx::init(IObLogTransCtxMgr *host)
{
  int ret = OB_SUCCESS;

  if (OB_ISNULL(host)) {
    LOG_ERROR("IObLogTransCtxMgr is NULL");
    ret = OB_INVALID_ARGUMENT;
  } else {
    host_ = host;
  }

  return ret;
}

int TransCtx::prepare(PartTransTask &part_trans_task,
    IObLogPartMgr &part_mgr,
    const bool print_participant_not_serve_info,
    volatile bool &stop_flag,
    bool &need_discard)
{
  int ret = OB_SUCCESS;
  const ObTransID &trans_id = part_trans_task.get_trans_id();

  // Requires locking conditions
  ObSpinLockGuard guard(lock_);
  need_discard = false;

  if (state_ < TRANS_CTX_STATE_INVALID) {
    // Context is deprecated and can no longer be used
    ret = OB_INVALID_ERROR;
  }
  // PREPARE is required only if the status is INVALID
  else if (TRANS_CTX_STATE_INVALID == state_) {
    // Requires that the partitioned transaction responsible for Prepare must be serviced, otherwise the transaction context will be deprecated
    if (OB_FAIL(prepare_(part_trans_task, part_mgr, print_participant_not_serve_info,
            stop_flag))) {
      if (OB_INVALID_ERROR == ret) {
        // The partition transaction is not serviced and the transaction context is discarded
        need_discard = true;
        state_ = TRANS_CTX_STATE_DISCARDED;
        _TCTX_ISTAT("[DISCARD] TRANS_ID=%s PART=%s TRANS_CTX=%p", to_cstring(trans_id),
            to_cstring(part_trans_task.get_partition()), this);
        // reset return value
        ret = OB_SUCCESS;
      } else if (OB_IN_STOP_STATE != ret) {
        LOG_ERROR("prepare trans_ctx fail", KR(ret), K(part_trans_task));
      }
    }
  } else {
    // Other cases indicate that it has been Prepared and returns normal
  }

  return ret;
}

int TransCtx::add_participant(PartTransTask &part_trans_task,
                              bool &is_part_trans_served,
                              bool &is_all_participants_ready)
{
  int ret = OB_SUCCESS;
  const ObTransID &trans_id = part_trans_task.get_trans_id();

  // Requires locking conditions
  ObSpinLockGuard guard(lock_);

  is_part_trans_served = true;
  is_all_participants_ready = false;

  if (state_ <= TRANS_CTX_STATE_INVALID) {
    LOG_ERROR("state not match which is not PREPARED or higher", "state", print_state());
    ret = OB_STATE_NOT_MATCH;
  } else if (state_ >= TRANS_CTX_STATE_PARTICIPANT_READY) {
    // Participants have been added, no later arrivals will be served
    is_part_trans_served = false;
    _TCTX_DSTAT("[PART_NOT_SERVE] [DELAY_COMING] TRANS_ID=%s PART=%s "
        "LOG_ID=%ld LOG_TSTAMP=%ld TRNAS_CTX_STATE=%s",
        to_cstring(trans_id), to_cstring(part_trans_task.get_partition()),
        part_trans_task.get_prepare_log_id(), part_trans_task.get_timestamp(),
        print_state());
  }
  // Attempt to add participants to the participant READY list
  else if (OB_FAIL(add_ready_participant_(part_trans_task, is_part_trans_served, is_all_participants_ready))) {
    LOG_ERROR("add_ready_participant_ fail", KR(ret), K(part_trans_task), K(is_part_trans_served),
        K(is_all_participants_ready));
  }

  return ret;
}

int TransCtx::prepare_(PartTransTask &host,
    IObLogPartMgr &part_mgr,
    const bool print_participant_not_serve_info,
    volatile bool &stop_flag)
{
  int ret = OB_SUCCESS;

  if (OB_UNLIKELY(TRANS_CTX_STATE_INVALID != state_)) {
    ret = OB_STATE_NOT_MATCH;
  } else if (OB_UNLIKELY(NULL != participants_) || OB_UNLIKELY(participant_count_ > 0)) {
    LOG_ERROR("participants info are not empty", K(participants_), K(participant_count_));
    ret = OB_ERR_UNEXPECTED;
  } else {
    const ObTransID &trans_id = host.get_trans_id();
    const ObPartitionKey &host_pkey = host.get_partition();
    int64_t host_prepare_log_timestmap = host.get_timestamp();
    int64_t host_prepare_log_id = host.get_prepare_log_id();
    const ObPartitionLogInfo *part_array = host.get_participants();
    int64_t part_count = host.get_participant_count();
    const int64_t global_trans_version = host.get_global_trans_version();
    // default serve
    bool is_serving_host_part = true;

    if (OB_UNLIKELY(part_count <= 0) || OB_ISNULL(part_array)) {
      LOG_ERROR("invalid participant array", K(part_count), K(part_array), K(host));
      ret = OB_ERR_UNEXPECTED;
    }
    // first determine whether to service the host partition transaction
    // if not, simply discard the transaction context.
    // This ensures that a valid transaction context must have been created by a valid participant, and thus ensure that the transaction context must be processed
    else if (OB_FAIL(inc_part_trans_count_on_serving_(is_serving_host_part,
                                                 part_mgr,
                                                 trans_id,
                                                 host_pkey,
                                                 host_prepare_log_id,
                                                 host_prepare_log_timestmap,
                                                 print_participant_not_serve_info,
                                                 stop_flag))) {
      if (OB_IN_STOP_STATE != ret) {
        LOG_ERROR("inc_part_trans_count_on_serving_ fail", KR(ret), K(host_pkey),
            K(host_prepare_log_id), K(host_prepare_log_timestmap));
      }
    } else if (! is_serving_host_part) {
      // host partition not serve
      ret = OB_INVALID_ERROR;
    }
    // Initialize the participant array on the premise of the host partition service
    else if (OB_FAIL(init_participant_array_(part_count))) {
      LOG_ERROR("init participant array fail", KR(ret), K(part_count));
    } else if (OB_FAIL(trx_sort_elem_.init(*this, trans_id, global_trans_version))) {
      LOG_ERROR("trx_sort_elem_ init fail", KR(ret), K(trans_id), K(global_trans_version));
    } else {
      trans_id_ = trans_id;

      // Prepare the list of participants in the case of a host partitioned transaction service, the list of participants must contain host partitioned transactions
      // Check if each participant is served and add the served participants to the participant list
      for (int64_t index = 0; OB_SUCC(ret) && index < part_count; index++) {
        const ObPartitionLogInfo &pinfo = part_array[index];

        if (! pinfo.is_valid()) {
          LOG_ERROR("participant info is invalid", K(pinfo), K(index), K(part_array));
          ret = OB_INVALID_DATA;
        } else {
          bool is_host_pkey = (host_pkey == pinfo.get_partition());

          // host partition must serve
          bool is_serving = is_host_pkey;

          if (! is_serving && OB_FAIL(inc_part_trans_count_on_serving_(is_serving,
                                                                       part_mgr,
                                                                       trans_id,
                                                                       pinfo.get_partition(),
                                                                       pinfo.get_log_id(),
                                                                       pinfo.get_log_timestamp(),
                                                                       print_participant_not_serve_info,
                                                                       stop_flag))) {
            if (OB_IN_STOP_STATE != ret) {
              LOG_ERROR("inc_part_trans_count_on_serving_ fail", KR(ret), K(pinfo));
            }
          } else if (is_serving) {
            // Add participant information for the service
            new (participants_ + participant_count_) TransPartInfo(pinfo.get_partition());
            participant_count_++;
          }

          if (! is_serving) {
            if (print_participant_not_serve_info) {
              _TCTX_ISTAT("[PREPARE_PART] [PART_NOT_SERVE] TRANS_ID=%s PART=%s LOG_ID=%ld "
                  "LOG_TSTAMP=%ld PARTICIPANT_INDEX=%ld/%ld",
                  to_cstring(trans_id), to_cstring(pinfo.get_partition()), pinfo.get_log_id(),
                  pinfo.get_log_timestamp(), index + 1, part_count);
            } else if (REACH_TIME_INTERVAL(PRINT_LOG_INTERVAL)) {
              _TCTX_ISTAT("[PREPARE_PART] [PART_NOT_SERVE] TRANS_ID=%s PART=%s LOG_ID=%ld "
                  "LOG_TSTAMP=%ld PARTICIPANT_INDEX=%ld/%ld",
                  to_cstring(trans_id), to_cstring(pinfo.get_partition()), pinfo.get_log_id(),
                  pinfo.get_log_timestamp(), index + 1, part_count);
            } else {
              // do nothing
            }
          } else {
            _TCTX_DSTAT("[PREPARE_PART] TRANS_ID=%s PART=%s LOG_ID=%ld LOG_TSTAMP=%ld "
                "PARTICIPANT_INDEX=%ld/%ld IS_SERVING=%d",
                to_cstring(trans_id), to_cstring(pinfo.get_partition()), pinfo.get_log_id(),
                pinfo.get_log_timestamp(), index + 1, part_count, is_serving);
          }
        }
      } // for

      // If the configuration requires participants to be sorted, the sorting operation is performed
      // The purpose is to ensure that the list of participants iterated out is the same in a restart cluster scenario
      if (OB_SUCC(ret)
          && OB_UNLIKELY(NULL != host_ && host_->need_sort_participant())
          && participant_count_ > 0) {
        std::sort(participants_, participants_ + participant_count_, TransPartInfoCompare());
      }

      _TCTX_DSTAT("[PREPARE] TRANS_ID=%s SERVED_PARTICIPANTS=%ld/%ld",
          to_cstring(trans_id), participant_count_, part_count);
    }

    // None of the partition participants are served, additional case
    if (OB_SUCC(ret)) {
      if (OB_UNLIKELY(participant_count_ <= 0)) {
        LOG_ERROR("get participants count unexpected failed", K(participant_count_));
        ret = OB_ERR_UNEXPECTED;
      } else if (OB_FAIL(init_trans_id_str_(trans_id))) {
        LOG_ERROR("init_trans_id_str_ fail", KR(ret), K(trans_id));
      } else {
        switch_state_(TRANS_CTX_STATE_PREPARED);
      }
    }
  }

  return ret;
}

// Initialize trans_id_str per transaction context to avoid Committer to_cstring ObTransID impacting commit performance
int TransCtx::init_trans_id_str_(const transaction::ObTransID &trans_id)
{
  int ret = OB_SUCCESS;
  // Use the maximum value for the buf length of the ObTransID, 1024 is large enough
  static const int64_t TRANS_ID_BUF_LENGTH = 1024;
  char trans_id_buf[TRANS_ID_BUF_LENGTH];
  int64_t pos = 0;

  if (OB_FAIL(common::databuff_printf(trans_id_buf, TRANS_ID_BUF_LENGTH, pos, "%s", to_cstring(trans_id)))) {
    LOG_ERROR("databuff_printf fail", K(ret), K(trans_id), K(trans_id_buf), K(TRANS_ID_BUF_LENGTH), K(pos));
  } else if (OB_UNLIKELY(pos <= 0 || pos >= TRANS_ID_BUF_LENGTH)) {
    LOG_ERROR("local buf pos is not valid", K(pos), K(TRANS_ID_BUF_LENGTH), K(trans_id));
    ret = OB_ERR_UNEXPECTED;
  } else {
    int64_t buf_len = pos + 1;
    char *buf = static_cast<char*>(allocator_.alloc(buf_len));

    if (OB_ISNULL(buf)) {
      LOG_ERROR("allocator_ alloc for trans id str fail", K(buf), K(buf_len));
      ret = OB_ALLOCATE_MEMORY_FAILED;
    } else {
      MEMCPY(buf, trans_id_buf, pos);
      buf[pos] = '\0';

      trans_id_str_.assign(buf, static_cast<int32_t>(pos));
    }
  }

  return ret;
}

void TransCtx::switch_state_(const int target_state)
{
  int ret = OB_SUCCESS;

  state_ = static_cast<int8_t>(target_state);

  if (NULL != host_) {
    if (OB_FAIL(host_->update_stat_info(target_state))) {
      LOG_ERROR("update stat info failed", KR(ret), K(target_state));
    }
  }
}

int TransCtx::add_ready_participant_(PartTransTask &part_trans_task,
                                     bool &is_part_trans_served,
                                     bool &is_all_participants_ready)
{
  int ret = OB_SUCCESS;

  if (OB_UNLIKELY(TRANS_CTX_STATE_PREPARED != state_)) {
    ret = OB_STATE_NOT_MATCH;
  } else {
    is_all_participants_ready = false;

    // Check if the partition is in the participants list and if it exists, set the participants to the participants array
    bool existed = false;
    for (int64_t index = 0; OB_SUCC(ret) && ! existed && index < participant_count_; index++) {
      if (part_trans_task.get_partition() == participants_[index].pkey_) {
        existed = true;
        participants_[index].obj_ = &part_trans_task;
      }
    }

    if (OB_SUCC(ret)) {
      // If the partitioned transaction is not in the list of participants, the partitioned transaction is no longer in service
      if (! existed) {
        is_part_trans_served = false;
      }
      // If the partition transaction is in the participant list, add it to the READY list
      else {
        ready_participant_count_++;
        is_all_participants_ready = (ready_participant_count_ == participant_count_);

        // If all participants are clustered, link them together in order to construct a list of participant objects
        if (is_all_participants_ready && OB_FAIL(build_ready_participants_list_())) {
          LOG_ERROR("build_ready_participants_list_ fail", KR(ret), K(is_all_participants_ready),
              K(ready_participant_count_), K(participant_count_));
        } else {
          // All participants are READY to advance to the current state
          if (is_all_participants_ready) {
            switch_state_(TRANS_CTX_STATE_PARTICIPANT_READY);
          }

        _TCTX_DSTAT("[ADD_PART] TRANS_ID=%s READY_PARTS=%ld/%ld READY=%d",
            to_cstring(trans_id_), ready_participant_count_,
            participant_count_, is_all_participants_ready);
        }
      }
    }
  }

  return ret;
}

int TransCtx::build_ready_participants_list_()
{
  int ret = OB_SUCCESS;
  if (OB_ISNULL(participants_)) {
    LOG_ERROR("invalid participants", K(participants_));
    ret = OB_ERR_UNEXPECTED;
  } else {
    // Linked list of participants
    for (int64_t index = 0; OB_SUCCESS == ret && index < participant_count_; index++) {
      if (OB_ISNULL(participants_[index].obj_)) {
        LOG_ERROR("participant object is NULL", K(participants_[index]), K(index),
            K(participant_count_), K(state_), K(trans_id_));
        ret = OB_ERR_UNEXPECTED;
      } else if (index < participant_count_ - 1) {
        participants_[index].obj_->set_next_task(participants_[index + 1].obj_);
      } else {
        participants_[index].obj_->set_next_task(NULL);
      }
    }

    // set ready obj list
    if (OB_SUCCESS == ret) {
      ready_participant_objs_ = participants_[0].obj_;
    }
  }
  return ret;
}

int TransCtx::inc_part_trans_count_on_serving_(bool &is_serving,
                                               IObLogPartMgr &part_mgr,
                                               const ObTransID &trans_id,
                                               const ObPartitionKey &key,
                                               const uint64_t prepare_log_id,
                                               const int64_t prepare_log_timestamp,
                                               const bool print_participant_not_serve_info,
                                               volatile bool &stop_flag)
{
  int ret = OB_SUCCESS;
  if (OB_UNLIKELY(!trans_id.is_valid())
      || OB_UNLIKELY(!key.is_valid())
      || OB_UNLIKELY(prepare_log_timestamp <= 0)) {
    ret = OB_INVALID_ARGUMENT;
  } else {
    RETRY_FUNC(stop_flag, (part_mgr), inc_part_trans_count_on_serving,
        is_serving,
        key,
        prepare_log_id,
        prepare_log_timestamp,
        print_participant_not_serve_info,
        DATA_OP_TIMEOUT);

    if (OB_SUCC(ret)) {
      _TCTX_DSTAT("[INC_TRANS_COUNT] IS_SERVING=%d TRANS_ID=%s PART=%s LOG_ID=%ld LOG_TSTAMP=%ld",
          is_serving, to_cstring(trans_id), to_cstring(key), prepare_log_id, prepare_log_timestamp);
    }
  }

  return ret;
}

int TransCtx::sequence(const int64_t seq)
{
  int ret = OB_SUCCESS;
  ObSpinLockGuard guard(lock_);

  if (OB_UNLIKELY(seq < 0)) {
    LOG_ERROR("invalid argument", K(seq));
    ret = OB_INVALID_ARGUMENT;
  } else if (TRANS_CTX_STATE_PARTICIPANT_READY != state_) {
    LOG_ERROR("state not match which is not DEP_PARSED", "state", print_state());
    ret = OB_STATE_NOT_MATCH;
  } else {
    seq_ = seq;
    switch_state_(TRANS_CTX_STATE_SEQUENCED);

    // Update the global sequence number and Schema version for each participant
    PartTransTask *task = ready_participant_objs_;
    while (NULL != task) {
      task->set_global_trans_seq(seq);
      task = task->next_task();
    }

    _TCTX_DSTAT("[SEQUENCE] COMMIT_VERSION=%ld TRANS_ID=%s SEQ=%ld",
        trx_sort_elem_.get_global_trans_version(),
        to_cstring(trans_id_),
        seq_);
  }

  return ret;
}

int TransCtx::wait_data_ready(const int64_t timeout,
    volatile bool &stop_flag)
{
  int ret = OB_SUCCESS;
  int64_t end_time = get_timestamp() + timeout;

  if (OB_UNLIKELY(TRANS_CTX_STATE_SEQUENCED != state_)) {
    LOG_ERROR("state is not match which is not SEQUENCED", "state", print_state());
    ret = OB_STATE_NOT_MATCH;
  } else {
    PartTransTask *part = ready_participant_objs_;
    while (OB_SUCC(ret) && NULL != part) {
      RETRY_FUNC(stop_flag, (*part), wait_data_ready, DATA_OP_TIMEOUT);

      int64_t left_time = end_time - get_timestamp();
      if (left_time <= 0) {
        //_TCTX_ISTAT()
        OBLOG_LOG(INFO, "wait_data_ready timeout", KPC(this), KPC(part));
      }

      if (OB_SUCC(ret)) {
        part = part->next_task();
      }
    } // while
  }

  if (OB_SUCC(ret)) {
    switch_state_(TRANS_CTX_STATE_DATA_READY);
  }

  return ret;
}

int TransCtx::commit()
{
  int ret = OB_SUCCESS;
  ObSpinLockGuard guard(lock_);

  if (OB_ISNULL(ready_participant_objs_)) {
    LOG_ERROR("ready participant objs is null",
        KPC(ready_participant_objs_), KPC(this));
    ret = OB_ERR_UNEXPECTED;
  } else {
    const bool is_ddl_trans = ready_participant_objs_->is_ddl_trans();

    if (TRANS_CTX_STATE_DATA_READY != state_) {
      LOG_ERROR("state not match which is not DATA_READY", "state", print_state(), K(is_ddl_trans));
      ret = OB_STATE_NOT_MATCH;
    } else {
      _TCTX_DSTAT("[COMMIT] TRANS_ID=%s/%ld PARTICIPANTS=%ld SEQ=%ld ",
          to_cstring(trans_id_), trx_sort_elem_.get_global_trans_version(),
          ready_participant_count_, seq_);

      switch_state_(TRANS_CTX_STATE_COMMITTED);
    }
  }

  return ret;
}

int TransCtx::inc_revertable_participant_count(bool &all_participant_revertable)
{
  int ret = OB_SUCCESS;
  ObSpinLockGuard guard(lock_);

  if (OB_UNLIKELY(TRANS_CTX_STATE_COMMITTED != state_)) {
    LOG_ERROR("state is not match which is not COMMITTED", "state", print_state());
    ret = OB_STATE_NOT_MATCH;
  } else {
    int64_t result_count = ATOMIC_AAF(&revertable_participant_count_, 1);
    all_participant_revertable = (result_count == ready_participant_count_);

    if (result_count > ready_participant_count_) {
      LOG_ERROR("revertable participant count is larger than ready participant count",
          "revertable_participant_count", result_count,
          K(ready_participant_count_),
          K(*this));
      ret = OB_ERR_UNEXPECTED;
    }
  }

  return ret;
}

int TransCtx::revert_participants()
{
  int ret = OB_SUCCESS;
  ObSpinLockGuard guard(lock_);

  if (OB_UNLIKELY(TRANS_CTX_STATE_COMMITTED != state_)) {
    LOG_ERROR("TransCtx has not been sequenced, can not revert participants",
        "state", print_state());
    ret = OB_STATE_NOT_MATCH;
  } else if (OB_UNLIKELY(revertable_participant_count_ != ready_participant_count_)) {
    LOG_ERROR("revertable_participant_count does not equal to participant count",
        K(revertable_participant_count_), K(ready_participant_count_));
    ret = OB_STATE_NOT_MATCH;
  } else {
    _TCTX_DSTAT("[REVERT_PARTICIPANTS] TRANS_ID=%s PARTICIPANTS=%ld SEQ=%ld",
        to_cstring(trans_id_), participant_count_, seq_);

    // Note: All participants are recalled by external modules
    ready_participant_objs_ = NULL;
    ready_participant_count_ = 0;
    revertable_participant_count_ = 0;

    switch_state_(TRANS_CTX_STATE_PARTICIPANT_REVERTED);
  }

  return ret;
}

int TransCtx::lock()
{
  return lock_.lock();
}

int TransCtx::unlock()
{
  return lock_.unlock();
}

int TransCtx::get_tenant_id(uint64_t &tenant_id) const
{
  int ret = OB_SUCCESS;
  ObSpinLockGuard guard(lock_);
  if (OB_ISNULL(ready_participant_objs_)) {
    LOG_ERROR("ready participant objs is null, can not decide tenant id",
        KPC(ready_participant_objs_), KPC(this));
    ret = OB_ERR_UNEXPECTED;
  } else {
    // The tenant ID of the first participant is used as the tenant ID of the distributed transaction
    // TODO: support for cross-tenant transactions
    tenant_id = ready_participant_objs_->get_tenant_id();
  }
  return ret;
}

int TransCtx::init_participant_array_(const int64_t part_count)
{
  int ret = OB_SUCCESS;

  if (OB_UNLIKELY(part_count <= 0)) {
    LOG_ERROR("invalid argument", K(part_count));
    ret = OB_INVALID_ARGUMENT;
  } else if (OB_UNLIKELY(NULL != participants_)) {
    LOG_ERROR("participant array has been initialized", K(participants_), K(participant_count_));
    ret = OB_INIT_TWICE;
  } else {
    int64_t parts_alloc_size = part_count * sizeof(participants_[0]);

    participants_ = static_cast<TransPartInfo *>(allocator_.alloc(parts_alloc_size));

    if (OB_ISNULL(participants_)) {
      LOG_ERROR("allocate memory for participant array fail", K(part_count), K(parts_alloc_size));
      ret = OB_ALLOCATE_MEMORY_FAILED;
    } else {
      // Number of valid participants is 0
      participant_count_ = 0;
    }
  }

  return ret;
}

void TransCtx::destroy_participant_array_()
{
  if (NULL != participants_ && participant_count_ > 0) {
    for (int64_t index = 0; index < participant_count_; index++) {
      participants_[index].~TransPartInfo();
    }

    allocator_.free(participants_);
    participants_ = NULL;
    participant_count_ = 0;
  }
}

} // namespace liboblog
} // namespace oceanbase
