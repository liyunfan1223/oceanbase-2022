
#define USING_LOG_PREFIX SQL_ENG

#include "sql/engine/cmd/ob_load_data_direct_demo.h"
#include "observer/omt/ob_tenant_timezone_mgr.h"
#include "share/schema/ob_schema_getter_guard.h"
#include "share/tablet/ob_tablet_to_ls_operator.h"
#include "storage/tablet/ob_tablet_create_delete_helper.h"
#include "storage/tx_storage/ob_ls_service.h"

namespace oceanbase
{
namespace sql
{
using namespace blocksstable;
using namespace common;
using namespace lib;
using namespace observer;
using namespace share;
using namespace share::schema;

/**
 * ObLoadDataBuffer
 */

ObLoadDataBuffer::ObLoadDataBuffer()
  : allocator_(ObModIds::OB_SQL_LOAD_DATA), data_(nullptr), begin_pos_(0), end_pos_(0), capacity_(0)
{
}

ObLoadDataBuffer::~ObLoadDataBuffer()
{
  reset();
}

void ObLoadDataBuffer::reuse()
{
  begin_pos_ = 0;
  end_pos_ = 0;
}

void ObLoadDataBuffer::reset()
{
  allocator_.reset();
  data_ = nullptr;
  begin_pos_ = 0;
  end_pos_ = 0;
  capacity_ = 0;
}

int ObLoadDataBuffer::create(int64_t capacity)
{
  int ret = OB_SUCCESS;
  if (OB_UNLIKELY(nullptr != data_)) {
    ret = OB_INIT_TWICE;
    LOG_WARN("ObLoadDataBuffer init twice", KR(ret), KP(this));
  } else if (OB_UNLIKELY(capacity <= 0)) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("invalid args", KR(ret), K(capacity));
  } else {
    allocator_.set_tenant_id(MTL_ID());
    if (OB_ISNULL(data_ = static_cast<char *>(allocator_.alloc(capacity)))) {
      ret = OB_ALLOCATE_MEMORY_FAILED;
      LOG_WARN("fail to alloc memory", KR(ret), K(capacity));
    } else {
      capacity_ = capacity;
    }
  }
  return ret;
}

int ObLoadDataBuffer::squash()
{
  int ret = OB_SUCCESS;
  if (OB_UNLIKELY(nullptr == data_)) {
    ret = OB_NOT_INIT;
    LOG_WARN("ObLoadDataBuffer not init", KR(ret), KP(this));
  } else {
    const int64_t data_size = get_data_size();
    if (data_size > 0) {
      MEMMOVE(data_, data_ + begin_pos_, data_size);
    }
    begin_pos_ = 0;
    end_pos_ = data_size;
  }
  return ret;
}

/**
 * ObLoadSequentialFileReader
 */

ObLoadSequentialFileReader::ObLoadSequentialFileReader()
  : offset_(0), is_read_end_(false)
{
}

ObLoadSequentialFileReader::~ObLoadSequentialFileReader()
{
}

int ObLoadSequentialFileReader::open(const ObString &filepath)
{
  int ret = OB_SUCCESS;
  if (OB_FAIL(file_reader_.open(filepath, false))) {
    LOG_WARN("fail to open file", KR(ret));
  }
  return ret;
}

int ObLoadSequentialFileReader::read_next_buffer(ObLoadDataBuffer &buffer)
{
  int ret = OB_SUCCESS;
  if (OB_UNLIKELY(!file_reader_.is_opened())) {
    ret = OB_FILE_NOT_OPENED;
    LOG_WARN("file not opened", KR(ret));
  } else if (is_read_end_) {
    ret = OB_ITER_END;
  } else if (OB_LIKELY(buffer.get_remain_size() > 0)) {
    const int64_t buffer_remain_size = buffer.get_remain_size();
    int64_t read_size = 0;
    if (OB_FAIL(file_reader_.pread(buffer.end(), buffer_remain_size, offset_, read_size))) {
      LOG_WARN("fail to do pread", KR(ret));
    } else if (read_size == 0) {
      is_read_end_ = true;
      ret = OB_ITER_END;
    } else {
      offset_ += read_size;
      buffer.produce(read_size);
    }
  }
  return ret;
}

/**
 * ObLoadCSVPaser
 */

ObLoadCSVPaser::ObLoadCSVPaser()
  : allocator_(ObModIds::OB_SQL_LOAD_DATA), collation_type_(CS_TYPE_INVALID), is_inited_(false)
{
}

ObLoadCSVPaser::~ObLoadCSVPaser()
{
  reset();
}

void ObLoadCSVPaser::reset()
{
  allocator_.reset();
  collation_type_ = CS_TYPE_INVALID;
  row_.reset();
  err_records_.reset();
  is_inited_ = false;
}

int ObLoadCSVPaser::init(const ObDataInFileStruct &format, int64_t column_count,
                         ObCollationType collation_type)
{
  int ret = OB_SUCCESS;
  if (IS_INIT) {
    ret = OB_INIT_TWICE;
    LOG_WARN("ObLoadCSVPaser init twice", KR(ret), KP(this));
  } else if (OB_FAIL(csv_parser_.init(format, column_count, collation_type))) {
    LOG_WARN("fail to init csv parser", KR(ret));
  } else {
    allocator_.set_tenant_id(MTL_ID());
    ObObj *objs = nullptr;
    if (OB_ISNULL(objs = static_cast<ObObj *>(allocator_.alloc(sizeof(ObObj) * column_count)))) {
      ret = OB_ALLOCATE_MEMORY_FAILED;
      LOG_WARN("fail to alloc memory", KR(ret));
    } else {
      new (objs) ObObj[column_count];
      row_.cells_ = objs;
      row_.count_ = column_count;
      collation_type_ = collation_type;
      is_inited_ = true;
    }
  }
  return ret;
}

int ObLoadCSVPaser::get_next_row(ObLoadDataBuffer &buffer, const ObNewRow *&row)
{
  int ret = OB_SUCCESS;
  row = nullptr;
  if (buffer.empty()) {
    ret = OB_ITER_END;
  } else {
    const char *str = buffer.begin();
    const char *end = buffer.end();
    int64_t nrows = 1;
    if (OB_FAIL(csv_parser_.scan(str, end, nrows, nullptr, nullptr, unused_row_handler_,
                                 err_records_, false))) {
      LOG_WARN("fail to scan buffer", KR(ret));
    } else if (OB_UNLIKELY(!err_records_.empty())) {
      ret = err_records_.at(0).err_code;
      LOG_WARN("fail to parse line", KR(ret));
    } else if (0 == nrows) {
      ret = OB_ITER_END;
    } else {
      buffer.consume(str - buffer.begin());
      const ObIArray<ObCSVGeneralParser::FieldValue> &field_values_in_file =
        csv_parser_.get_fields_per_line();
      for (int64_t i = 0; i < row_.count_; ++i) {
        const ObCSVGeneralParser::FieldValue &str_v = field_values_in_file.at(i);
        ObObj &obj = row_.cells_[i];
        if (str_v.is_null_) {
          obj.set_null();
        } else {
          obj.set_string(ObVarcharType, ObString(str_v.len_, str_v.ptr_));
          obj.set_collation_type(collation_type_);
        }
      }
      row = &row_;
    }
  }
  return ret;
}

/**
 * ObLoadDatumRow
 */

ObLoadDatumRow::ObLoadDatumRow()
  : allocator_(ObModIds::OB_SQL_LOAD_DATA), capacity_(0), count_(0), datums_(nullptr)
{
}

ObLoadDatumRow::~ObLoadDatumRow()
{
}

void ObLoadDatumRow::reset()
{
  allocator_.reset();
  capacity_ = 0;
  count_ = 0;
  datums_ = nullptr;
}

int ObLoadDatumRow::init(int64_t capacity)
{
  int ret = OB_SUCCESS;
  if (OB_UNLIKELY(capacity <= 0)) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("invalid args", KR(ret), K(capacity));
  } else {
    reset();
    allocator_.set_tenant_id(MTL_ID());
    if (OB_ISNULL(datums_ = static_cast<ObStorageDatum *>(
                    allocator_.alloc(sizeof(ObStorageDatum) * capacity)))) {
      ret = OB_ALLOCATE_MEMORY_FAILED;
      LOG_WARN("fail to alloc memory", KR(ret));
    } else {
      new (datums_) ObStorageDatum[capacity];
      capacity_ = capacity;
      count_ = capacity;
    }
  }
  return ret;
}

int64_t ObLoadDatumRow::get_deep_copy_size() const
{
  int64_t size = 0;
  size += sizeof(ObStorageDatum) * count_;
  for (int64_t i = 0; i < count_; ++i) {
    size += datums_[i].get_deep_copy_size();
  }
  return size;
}

int ObLoadDatumRow::deep_copy(const ObLoadDatumRow &src, char *buf, int64_t len, int64_t &pos)
{
  int ret = OB_SUCCESS;
  if (OB_UNLIKELY(!src.is_valid())) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("invalid args", KR(ret), K(src));
  } else {
    reset();
    ObStorageDatum *datums = nullptr;
    const int64_t datum_cnt = src.count_;
    datums = new (buf + pos) ObStorageDatum[datum_cnt];
    pos += sizeof(ObStorageDatum) * datum_cnt;
    for (int64_t i = 0; OB_SUCC(ret) && i < datum_cnt; ++i) {
      if (OB_FAIL(datums[i].deep_copy(src.datums_[i], buf, len, pos))) {
        LOG_WARN("fail to deep copy storage datum", KR(ret), K(src.datums_[i]));
      }
    }
    if (OB_SUCC(ret)) {
      capacity_ = datum_cnt;
      count_ = datum_cnt;
      datums_ = datums;
    }
  }
  return ret;
}

DEF_TO_STRING(ObLoadDatumRow)
{
  int64_t pos = 0;
  J_OBJ_START();
  J_KV(K_(capacity), K_(count));
  if (nullptr != datums_) {
    J_ARRAY_START();
    for (int64_t i = 0; i < count_; ++i) {
      databuff_printf(buf, buf_len, pos, "col_id=%ld:", i);
      pos += datums_[i].storage_to_string(buf + pos, buf_len - pos);
      databuff_printf(buf, buf_len, pos, ",");
    }
    J_ARRAY_END();
  }
  J_OBJ_END();
  return pos;
}

OB_DEF_SERIALIZE(ObLoadDatumRow)
{
  int ret = OB_SUCCESS;
  OB_UNIS_ENCODE_ARRAY(datums_, count_);
  return ret;
}

OB_DEF_DESERIALIZE(ObLoadDatumRow)
{
  int ret = OB_SUCCESS;
  int64_t count = 0;
  OB_UNIS_DECODE(count);
  if (OB_SUCC(ret)) {
    if (OB_UNLIKELY(count <= 0)) {
      ret = OB_ERR_UNEXPECTED;
      LOG_WARN("unexpected count", K(count));
    } else if (count > capacity_ && OB_FAIL(init(count))) {
      LOG_WARN("fail to init", KR(ret));
    } else {
      OB_UNIS_DECODE_ARRAY(datums_, count);
      count_ = count;
    }
  }
  return ret;
}

OB_DEF_SERIALIZE_SIZE(ObLoadDatumRow)
{
  int64_t len = 0;
  OB_UNIS_ADD_LEN_ARRAY(datums_, count_);
  return len;
}

/**
 * ObLoadDatumRowCompare
 */

ObLoadDatumRowCompare::ObLoadDatumRowCompare()
  : result_code_(OB_SUCCESS), rowkey_column_num_(0), datum_utils_(nullptr), is_inited_(false)
{
}

ObLoadDatumRowCompare::~ObLoadDatumRowCompare()
{
}

int ObLoadDatumRowCompare::init(int64_t rowkey_column_num, const ObStorageDatumUtils *datum_utils)
{
  int ret = OB_SUCCESS;
  if (IS_INIT) {
    ret = OB_INIT_TWICE;
    LOG_WARN("ObLoadDatumRowCompare init twice", KR(ret), KP(this));
  } else if (OB_UNLIKELY(rowkey_column_num <= 0 || nullptr == datum_utils)) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("invalid args", KR(ret), K(rowkey_column_num), KP(datum_utils));
  } else {
    rowkey_column_num_ = rowkey_column_num;
    datum_utils_ = datum_utils;
    is_inited_ = true;
  }
  return ret;
}

bool ObLoadDatumRowCompare::operator()(const ObLoadDatumRow *lhs, const ObLoadDatumRow *rhs)
{
  int ret = OB_SUCCESS;
  int cmp_ret = 0;
  if (IS_NOT_INIT) {
    ret = OB_NOT_INIT;
    LOG_WARN("ObDirectLoadDatumRowCompare not init", KR(ret), KP(this));
  } else if (OB_ISNULL(lhs) || OB_ISNULL(rhs) ||
             OB_UNLIKELY(!lhs->is_valid() || !rhs->is_valid())) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("invalid args", KR(ret), KPC(lhs), KPC(rhs));
  } else if (OB_UNLIKELY(lhs->count_ < rowkey_column_num_ || rhs->count_ < rowkey_column_num_)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected column count", KR(ret), KPC(lhs), KPC(rhs), K_(rowkey_column_num));
  } else {
    if (OB_FAIL(lhs_rowkey_.assign(lhs->datums_, rowkey_column_num_))) {
      LOG_WARN("fail to assign datum rowkey", KR(ret), K(lhs), K_(rowkey_column_num));
    } else if (OB_FAIL(rhs_rowkey_.assign(rhs->datums_, rowkey_column_num_))) {
      LOG_WARN("fail to assign datum rowkey", KR(ret), K(rhs), K_(rowkey_column_num));
    } else if (OB_FAIL(lhs_rowkey_.compare(rhs_rowkey_, *datum_utils_, cmp_ret))) {
      LOG_WARN("fail to compare rowkey", KR(ret), K(rhs_rowkey_), K(rhs_rowkey_), KP(datum_utils_));
    }
  }
  if (OB_FAIL(ret)) {
    result_code_ = ret;
  }
  return cmp_ret < 0;
}

/**
 * ObLoadRowCaster
 */

ObLoadRowCaster::ObLoadRowCaster()
  : column_count_(0),
    collation_type_(CS_TYPE_INVALID),
    cast_allocator_(ObModIds::OB_SQL_LOAD_DATA),
    is_inited_(false)
{
}

ObLoadRowCaster::~ObLoadRowCaster()
{
}

int ObLoadRowCaster::init(const ObTableSchema *table_schema,
                          const ObIArray<ObLoadDataStmt::FieldOrVarStruct> &field_or_var_list)
{
  int ret = OB_SUCCESS;
  if (IS_INIT) {
    ret = OB_INIT_TWICE;
    LOG_WARN("ObLoadRowCaster init twice", KR(ret));
  } else if (OB_UNLIKELY(nullptr == table_schema || field_or_var_list.empty())) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("invalid args", KR(ret), KP(table_schema), K(field_or_var_list));
  } else if (OB_FAIL(OTTZ_MGR.get_tenant_tz(MTL_ID(), tz_info_.get_tz_map_wrap()))) {
    LOG_WARN("fail to get tenant time zone", KR(ret));
  } else if (OB_FAIL(init_column_schemas_and_idxs(table_schema, field_or_var_list))) {
    LOG_WARN("fail to init column schemas and idxs", KR(ret));
  } else if (OB_FAIL(datum_row_.init(table_schema->get_column_count()))) {
    LOG_WARN("fail to init datum row", KR(ret));
  } else {
    column_count_ = table_schema->get_column_count();
    collation_type_ = table_schema->get_collation_type();
    cast_allocator_.set_tenant_id(MTL_ID());
    is_inited_ = true;
  }
  return ret;
}

int ObLoadRowCaster::init_column_schemas_and_idxs(
  const ObTableSchema *table_schema,
  const ObIArray<ObLoadDataStmt::FieldOrVarStruct> &field_or_var_list)
{
  int ret = OB_SUCCESS;
  ObSEArray<ObColDesc, 64> column_descs;
  if (OB_FAIL(table_schema->get_column_ids(column_descs))) {
    LOG_WARN("fail to get column descs", KR(ret), KPC(table_schema));
  } else {
    bool found_column = true;
    for (int64_t i = 0; OB_SUCC(ret) && OB_LIKELY(found_column) && i < column_descs.count(); ++i) {
      const ObColDesc &col_desc = column_descs.at(i);
      const ObColumnSchemaV2 *col_schema = table_schema->get_column_schema(col_desc.col_id_);
      if (OB_ISNULL(col_schema)) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("unexpected null column schema", KR(ret), K(col_desc));
      } else if (OB_UNLIKELY(col_schema->is_hidden())) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("unexpected hidden column", KR(ret), K(i), KPC(col_schema));
      } else if (OB_FAIL(column_schemas_.push_back(col_schema))) {
        LOG_WARN("fail to push back column schema", KR(ret));
      } else {
        found_column = false;
      }
      // find column in source data columns
      for (int64_t j = 0; OB_SUCC(ret) && OB_LIKELY(!found_column) && j < field_or_var_list.count();
           ++j) {
        const ObLoadDataStmt::FieldOrVarStruct &field_or_var_struct = field_or_var_list.at(j);
        if (col_desc.col_id_ == field_or_var_struct.column_id_) {
          found_column = true;
          if (OB_FAIL(column_idxs_.push_back(j))) {
            LOG_WARN("fail to push back column idx", KR(ret), K(column_idxs_), K(i), K(col_desc),
                     K(j), K(field_or_var_struct));
          }
        }
      }
    }
    if (OB_SUCC(ret) && OB_UNLIKELY(!found_column)) {
      ret = OB_NOT_SUPPORTED;
      LOG_WARN("not supported incomplete column data", KR(ret), K(column_idxs_), K(column_descs),
               K(field_or_var_list));
    }
  }
  return ret;
}

int ObLoadRowCaster::get_casted_row(const ObNewRow &new_row, const ObLoadDatumRow *&datum_row)
{
  int ret = OB_SUCCESS;
  if (IS_NOT_INIT) {
    ret = OB_NOT_INIT;
    LOG_WARN("ObLoadRowCaster not init", KR(ret));
  } else {
    const int64_t extra_col_cnt = ObMultiVersionRowkeyHelpper::get_extra_rowkey_col_cnt();
    cast_allocator_.reuse();
    for (int64_t i = 0; OB_SUCC(ret) && i < column_idxs_.count(); ++i) {
      int64_t column_idx = column_idxs_.at(i);
      if (OB_UNLIKELY(column_idx < 0 || column_idx >= new_row.count_)) {
        ret = OB_ERR_UNEXPECTED;
        LOG_WARN("unexpected column idx", KR(ret), K(column_idx), K(new_row.count_));
      } else {
        const ObColumnSchemaV2 *column_schema = column_schemas_.at(i);
        const ObObj &src_obj = new_row.cells_[column_idx];
        ObStorageDatum &dest_datum = datum_row_.datums_[i];
        if (OB_FAIL(cast_obj_to_datum(column_schema, src_obj, dest_datum))) {
          LOG_WARN("fail to cast obj to datum", KR(ret), K(src_obj));
        }
      }
    }
    if (OB_SUCC(ret)) {
      datum_row = &datum_row_;
    }
  }
  return ret;
}

int ObLoadRowCaster::cast_obj_to_datum(const ObColumnSchemaV2 *column_schema, const ObObj &obj,
                                       ObStorageDatum &datum)
{
  int ret = OB_SUCCESS;
  ObDataTypeCastParams cast_params(&tz_info_);
  ObCastCtx cast_ctx(&cast_allocator_, &cast_params, CM_NONE, collation_type_);
  const ObObjType expect_type = column_schema->get_meta_type().get_type();
  ObObj casted_obj;
  if (obj.is_null()) {
    casted_obj.set_null();
  } else if (is_oracle_mode() && (obj.is_null_oracle() || 0 == obj.get_val_len())) {
    casted_obj.set_null();
  } else if (is_mysql_mode() && 0 == obj.get_val_len() && !ob_is_string_tc(expect_type)) {
    ObObj zero_obj;
    zero_obj.set_int(0);
    if (OB_FAIL(ObObjCaster::to_type(expect_type, cast_ctx, zero_obj, casted_obj))) {
      LOG_WARN("fail to do to type", KR(ret), K(zero_obj), K(expect_type));
    }
  } else {
    if (OB_FAIL(ObObjCaster::to_type(expect_type, cast_ctx, obj, casted_obj))) {
      LOG_WARN("fail to do to type", KR(ret), K(obj), K(expect_type));
    }
  }
  if (OB_SUCC(ret)) {
    if (OB_FAIL(datum.from_obj_enhance(casted_obj))) {
      LOG_WARN("fail to from obj enhance", KR(ret), K(casted_obj));
    }
  }
  return ret;
}

/**
 * ObLoadExternalSort
 */

ObLoadExternalSort::ObLoadExternalSort()
  : allocator_(ObModIds::OB_SQL_LOAD_DATA), is_closed_(false), is_inited_(false)
{
}

ObLoadExternalSort::~ObLoadExternalSort()
{
  external_sort_.clean_up();
}

int ObLoadExternalSort::init(const ObTableSchema *table_schema, int64_t mem_size,
                             int64_t file_buf_size)
{
  int ret = OB_SUCCESS;
  if (IS_INIT) {
    ret = OB_INIT_TWICE;
    LOG_WARN("ObLoadExternalSort init twice", KR(ret), KP(this));
  } else if (OB_UNLIKELY(nullptr == table_schema)) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("invalid args", KR(ret), KP(table_schema));
  } else {
    allocator_.set_tenant_id(MTL_ID());
    const int64_t rowkey_column_num = table_schema->get_rowkey_column_num();
    ObArray<ObColDesc> multi_version_column_descs;
    if (OB_FAIL(table_schema->get_multi_version_column_descs(multi_version_column_descs))) {
      LOG_WARN("fail to get multi version column descs", KR(ret));
    } else if (OB_FAIL(datum_utils_.init(multi_version_column_descs, rowkey_column_num,
                                         is_oracle_mode(), allocator_))) {
      LOG_WARN("fail to init datum utils", KR(ret));
    } else if (OB_FAIL(compare_.init(rowkey_column_num, &datum_utils_))) {
      LOG_WARN("fail to init compare", KR(ret));
    } else if (OB_FAIL(external_sort_.init(mem_size, file_buf_size, 0, MTL_ID(), &compare_))) {
      LOG_WARN("fail to init external sort", KR(ret));
    } else {
      is_inited_ = true;
    }
  }
  return ret;
}

int ObLoadExternalSort::append_row(const ObLoadDatumRow &datum_row)
{
  int ret = OB_SUCCESS;
  if (IS_NOT_INIT) {
    ret = OB_NOT_INIT;
    LOG_WARN("ObLoadExternalSort not init", KR(ret), KP(this));
  } else if (OB_UNLIKELY(is_closed_)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected closed external sort", KR(ret));
  } else if (OB_FAIL(external_sort_.add_item(datum_row))) {
    LOG_WARN("fail to add item", KR(ret));
  }
  return ret;
}

int ObLoadExternalSort::close()
{
  int ret = OB_SUCCESS;
  if (IS_NOT_INIT) {
    ret = OB_NOT_INIT;
    LOG_WARN("ObLoadExternalSort not init", KR(ret), KP(this));
  } else if (OB_UNLIKELY(is_closed_)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected closed external sort", KR(ret));
  } else if (OB_FAIL(external_sort_.do_sort(true))) {
    LOG_WARN("fail to do sort", KR(ret));
  } else {
    is_closed_ = true;
  }
  return ret;
}

int ObLoadExternalSort::get_next_row(const ObLoadDatumRow *&datum_row)
{
  int ret = OB_SUCCESS;
  if (IS_NOT_INIT) {
    ret = OB_NOT_INIT;
    LOG_WARN("ObLoadExternalSort not init", KR(ret), KP(this));
  } else if (OB_UNLIKELY(!is_closed_)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected not closed external sort", KR(ret));
  } else if (OB_FAIL(external_sort_.get_next_item(datum_row))) {
    LOG_WARN("fail to get next item", KR(ret));
  }
  return ret;
}

/**
 * ObLoadSSTableWriter
 */

ObLoadSSTableWriter::ObLoadSSTableWriter()
  : rowkey_column_num_(0),
    extra_rowkey_column_num_(0),
    column_count_(0),
    is_closed_(false),
    is_inited_(false),
    closed_thread_(0)
{
}

ObLoadSSTableWriter::~ObLoadSSTableWriter()
{
}

int ObLoadSSTableWriter::init(const ObTableSchema *table_schema)
{
  int ret = OB_SUCCESS;
  if (IS_INIT) {
    ret = OB_INIT_TWICE;
    LOG_WARN("ObLoadSSTableWriter init twice", KR(ret), KP(this));
  } else if (OB_UNLIKELY(nullptr == table_schema)) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("invalid args", KR(ret), KP(table_schema));
  } else {
    tablet_id_ = table_schema->get_tablet_id();
    rowkey_column_num_ = table_schema->get_rowkey_column_num();
    extra_rowkey_column_num_ = ObMultiVersionRowkeyHelpper::get_extra_rowkey_col_cnt();
    column_count_ = table_schema->get_column_count();
    ObLocationService *location_service = nullptr;
    bool is_cache_hit = false;
    ObLSService *ls_service = nullptr;
    ObLS *ls = nullptr;
    if (OB_ISNULL(location_service = GCTX.location_service_)) {
      ret = OB_ERR_SYS;
      LOG_WARN("location service is null", KR(ret), KP(location_service));
    } else if (OB_FAIL(
                 location_service->get(MTL_ID(), tablet_id_, INT64_MAX, is_cache_hit, ls_id_))) {
      LOG_WARN("fail to get ls id", KR(ret), K(tablet_id_));
    } else if (OB_ISNULL(ls_service = MTL(ObLSService *))) {
      ret = OB_ERR_SYS;
      LOG_ERROR("ls service is null", KR(ret));
    } else if (OB_FAIL(ls_service->get_ls(ls_id_, ls_handle_, ObLSGetMod::STORAGE_MOD))) {
      LOG_WARN("fail to get ls", KR(ret), K(ls_id_));
    } else if (OB_ISNULL(ls = ls_handle_.get_ls())) {
      ret = OB_ERR_UNEXPECTED;
      LOG_ERROR("ls should not be null", KR(ret));
    } else if (OB_FAIL(ls->get_tablet(tablet_id_, tablet_handle_))) {
      LOG_WARN("fail to get tablet handle", KR(ret), K(tablet_id_));
    } else if (OB_FAIL(init_sstable_index_builder(table_schema))) {
      LOG_WARN("fail to init sstable index builder", KR(ret));
    } else if (OB_FAIL(init_data_store_desc(table_schema))) {
      LOG_WARN("fail to init macro block writer", KR(ret));
    }
    if (OB_SUCC(ret)) {
      table_key_.table_type_ = ObITable::MAJOR_SSTABLE;
      table_key_.tablet_id_ = tablet_id_;
      table_key_.log_ts_range_.start_log_ts_ = 0;
      table_key_.log_ts_range_.end_log_ts_ = ObTimeUtil::current_time_ns();
      for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        datum_rows_[i].init(column_count_ + extra_rowkey_column_num_);
        datum_rows_[i].row_flag_.set_flag(ObDmlFlag::DF_INSERT);
        datum_rows_[i].mvcc_row_flag_.set_last_multi_version_row(true);
        datum_rows_[i].storage_datums_[rowkey_column_num_].set_int(-1); // fill trans_version
        datum_rows_[i].storage_datums_[rowkey_column_num_ + 1].set_int(0); // fill sql_no
      }
      is_inited_ = true;
      table_schema_ = table_schema;
    }
  }
  return ret;
}

int ObLoadSSTableWriter::init_sstable_index_builder(const ObTableSchema *table_schema)
{
  int ret = OB_SUCCESS;
  ObDataStoreDesc data_desc;
  if (OB_FAIL(data_desc.init(*table_schema, ls_id_, tablet_id_, MAJOR_MERGE, 1L))) {
    LOG_WARN("fail to init data desc", KR(ret));
  } else {
    data_desc.row_column_count_ = data_desc.rowkey_column_count_ + 1;
    data_desc.need_prebuild_bloomfilter_ = false;
    data_desc.col_desc_array_.reset();
    if (OB_FAIL(data_desc.col_desc_array_.init(data_desc.row_column_count_))) {
      LOG_WARN("fail to reserve column desc array", KR(ret));
    } else if (OB_FAIL(table_schema->get_rowkey_column_ids(data_desc.col_desc_array_))) {
      LOG_WARN("fail to get rowkey column ids", KR(ret));
    } else if (OB_FAIL(
                 ObMultiVersionRowkeyHelpper::add_extra_rowkey_cols(data_desc.col_desc_array_))) {
      LOG_WARN("fail to add extra rowkey cols", KR(ret));
    } else {
      ObObjMeta meta;
      meta.set_varchar();
      meta.set_collation_type(CS_TYPE_BINARY);
      ObColDesc col;
      col.col_id_ = static_cast<uint64_t>(data_desc.row_column_count_ + OB_APP_MIN_COLUMN_ID);
      col.col_type_ = meta;
      col.col_order_ = DESC;
      if (OB_FAIL(data_desc.col_desc_array_.push_back(col))) {
        LOG_WARN("fail to push back last col for index", KR(ret), K(col));
      }
    }
  }
  if (OB_SUCC(ret)) {
    if (OB_FAIL(sstable_index_builder_.init(data_desc))) {
      LOG_WARN("fail to init index builder", KR(ret), K(data_desc));
    }
  }
  return ret;
}

int ObLoadSSTableWriter::init_data_store_desc(const ObTableSchema *table_schema)
{
  int ret = OB_SUCCESS;
  if (OB_FAIL(data_store_desc_.init(*table_schema, ls_id_, tablet_id_, MAJOR_MERGE, 1))) {
    LOG_WARN("fail to init data_store_desc", KR(ret), K(tablet_id_));
  } else {
    data_store_desc_.sstable_index_builder_ = &sstable_index_builder_;
  }
  return ret;
}

int ObLoadSSTableWriter::init_macro_block_writer(uint64_t thread_id, blocksstable::ObMacroBlockWriter &macro_block_writer)
{
  int ret = OB_SUCCESS;
  if (OB_SUCC(ret)) {
    ObMacroDataSeq data_seq(thread_id * 100);
    // LOG_INFO("[DATA_DESC AND DATA_SEQ]", K(data_store_desc_), K(data_seq));
    if (OB_FAIL(macro_block_writer.open(data_store_desc_, data_seq))) {
      LOG_WARN("fail to init macro block writer", KR(ret), K(data_store_desc_), K(data_seq));
    }
  }
  return ret;
}

int ObLoadSSTableWriter::append_row(int thread_id, const ObLoadDatumRow &datum_row, blocksstable::ObMacroBlockWriter &macro_block_writer)
{
  int ret = OB_SUCCESS;
  // if (IS_NOT_INIT) {
  //   ret = OB_NOT_INIT;
  //   LOG_WARN("ObLoadSSTableWriter not init", KR(ret), KP(this));
  // } else if (OB_UNLIKELY(is_closed_)) {
  //   ret = OB_ERR_UNEXPECTED;
  //   LOG_WARN("unexpected closed external sort", KR(ret));
  // } else if (OB_UNLIKELY(!datum_row.is_valid() || datum_row.count_ != column_count_)) {
  //   ret = OB_INVALID_ARGUMENT;
  //   LOG_WARN("invalid args", KR(ret), K(datum_row), K(column_count_));
  {
    for (int64_t i = 0; i < column_count_; ++i) {
      if (i < rowkey_column_num_) {
        datum_rows_[thread_id].storage_datums_[i] = datum_row.datums_[i];
      } else {
        datum_rows_[thread_id].storage_datums_[i + extra_rowkey_column_num_] = datum_row.datums_[i];
      }
    }
    // LOG_INFO("try append row", K(datum_rows_[thread_id]));
    if (OB_FAIL(macro_block_writer.append_row(datum_rows_[thread_id]))) {
      LOG_WARN("fail to append row", KR(ret));
    }
  }
  return ret;
}

int ObLoadSSTableWriter::create_sstable()
{
  int ret = OB_SUCCESS;
  ObTableHandleV2 table_handle;
  SMART_VAR(ObSSTableMergeRes, merge_res)
  {
    const ObStorageSchema &storage_schema = tablet_handle_.get_obj()->get_storage_schema();
    int64_t column_count = 0;
    if (OB_FAIL(storage_schema.get_stored_column_count_in_sstable(column_count))) {
      LOG_WARN("fail to get stored column count in sstable", KR(ret));
    } else if (OB_FAIL(sstable_index_builder_.close(column_count, merge_res))) {
      LOG_WARN("fail to close sstable index builder", KR(ret));
    } else {
      ObTabletCreateSSTableParam create_param;
      create_param.table_key_ = table_key_;
      create_param.table_mode_ = storage_schema.get_table_mode_struct();
      create_param.index_type_ = storage_schema.get_index_type();
      create_param.rowkey_column_cnt_ = storage_schema.get_rowkey_column_num() +
                                        ObMultiVersionRowkeyHelpper::get_extra_rowkey_col_cnt();
      create_param.schema_version_ = storage_schema.get_schema_version();
      create_param.create_snapshot_version_ = 0;
      ObSSTableMergeRes::fill_addr_and_data(merge_res.root_desc_, create_param.root_block_addr_,
                                            create_param.root_block_data_);
      ObSSTableMergeRes::fill_addr_and_data(merge_res.data_root_desc_,
                                            create_param.data_block_macro_meta_addr_,
                                            create_param.data_block_macro_meta_);
      create_param.root_row_store_type_ = merge_res.root_desc_.row_type_;
      create_param.data_index_tree_height_ = merge_res.root_desc_.height_;
      create_param.index_blocks_cnt_ = merge_res.index_blocks_cnt_;
      create_param.data_blocks_cnt_ = merge_res.data_blocks_cnt_;
      create_param.micro_block_cnt_ = merge_res.micro_block_cnt_;
      create_param.use_old_macro_block_count_ = merge_res.use_old_macro_block_count_;
      create_param.row_count_ = merge_res.row_count_;
      create_param.column_cnt_ = merge_res.data_column_cnt_;
      create_param.data_checksum_ = merge_res.data_checksum_;
      create_param.occupy_size_ = merge_res.occupy_size_;
      create_param.original_size_ = merge_res.original_size_;
      create_param.max_merged_trans_version_ = merge_res.max_merged_trans_version_;
      create_param.contain_uncommitted_row_ = merge_res.contain_uncommitted_row_;
      create_param.compressor_type_ = merge_res.compressor_type_;
      create_param.encrypt_id_ = merge_res.encrypt_id_;
      create_param.master_key_id_ = merge_res.master_key_id_;
      create_param.data_block_ids_ = merge_res.data_block_ids_;
      create_param.other_block_ids_ = merge_res.other_block_ids_;
      MEMCPY(create_param.encrypt_key_, merge_res.encrypt_key_,
             OB_MAX_TABLESPACE_ENCRYPT_KEY_LENGTH);
      if (OB_FAIL(
            merge_res.fill_column_checksum(&storage_schema, create_param.column_checksums_))) {
        LOG_WARN("fail to fill column checksum for empty major", KR(ret), K(create_param));
      } else if (OB_FAIL(ObTabletCreateDeleteHelper::create_sstable(create_param, table_handle))) {
        LOG_WARN("fail to create sstable", KR(ret), K(create_param));
      } else {
        const int64_t rebuild_seq = ls_handle_.get_ls()->get_rebuild_seq();
        ObTabletHandle new_tablet_handle;
        ObUpdateTableStoreParam table_store_param(table_handle,
                                                  tablet_handle_.get_obj()->get_snapshot_version(),
                                                  false, &storage_schema, rebuild_seq, true, true);
        if (OB_FAIL(ls_handle_.get_ls()->update_tablet_table_store(tablet_id_, table_store_param,
                                                                   new_tablet_handle))) {
          LOG_WARN("fail to update tablet table store", KR(ret), K(tablet_id_),
                   K(table_store_param));
        }
      }
    }
  }
  return ret;
}

int ObLoadSSTableWriter::close_macro_block_writer(blocksstable::ObMacroBlockWriter &macro_block_writer)
{
  int ret = OB_SUCCESS;
  // if (IS_NOT_INIT) {
  //   ret = OB_NOT_INIT;
  //   LOG_WARN("ObLoadSSTableWriter not init", KR(ret), KP(this));
  // } else if (OB_UNLIKELY(is_closed_)) {
  //   ret = OB_ERR_UNEXPECTED;
  //   LOG_WARN("unexpected closed sstable writer", KR(ret));
  {
    if (OB_FAIL(macro_block_writer.close())) {
      LOG_WARN("fail to close macro block writer", KR(ret));
    }
  }
  return ret;
}

int ObLoadSSTableWriter::close()
{
  int ret = OB_SUCCESS;
  LOG_INFO("[CREATE_SSTABLE]");
  if (OB_FAIL(create_sstable())) {
    LOG_WARN("fail to create sstable", KR(ret));
  } else {
    is_closed_ = true;
  }
  return ret;
}

/**
 * ObLoadDataDirectDemo
 */

ObLoadDataDirectDemo::ObLoadDataDirectDemo()
{
}

ObLoadDataDirectDemo::~ObLoadDataDirectDemo()
{
}

int ObLoadDataDirectDemo::execute(ObExecContext &ctx, ObLoadDataStmt &load_stmt)
{
  int ret = OB_SUCCESS;
  if (OB_FAIL(inner_init(load_stmt))) {
    LOG_WARN("fail to inner init", KR(ret));
  } else if (OB_FAIL(do_load())) {
    LOG_WARN("fail to do load", KR(ret));
  }
  return ret;
}

int ObLoadDataDirectDemo::inner_init(ObLoadDataStmt &load_stmt)
{
  int ret = OB_SUCCESS;
  load_stmt_ = &load_stmt;
  const ObLoadArgument &load_args = load_stmt.get_load_arguments();
  const ObIArray<ObLoadDataStmt::FieldOrVarStruct> &field_or_var_list =
    load_stmt.get_field_or_var_list();
  const uint64_t tenant_id = load_args.tenant_id_;
  const uint64_t table_id = load_args.table_id_;
  ObSchemaGetterGuard schema_guard;
  allocator_.set_tenant_id(1);
  if (OB_FAIL(ObMultiVersionSchemaService::get_instance().get_tenant_schema_guard(tenant_id,
                                                                                  schema_guard))) {
    LOG_WARN("fail to get tenant schema guard", KR(ret), K(tenant_id));
  } else if (OB_FAIL(schema_guard.get_table_schema(tenant_id, table_id, table_schema_))) {
    LOG_WARN("fail to get table schema", KR(ret), K(tenant_id), K(table_id));
  } else if (OB_ISNULL(table_schema_)) {
    ret = OB_TABLE_NOT_EXIST;
    LOG_WARN("table not exist", KR(ret), K(tenant_id), K(table_id));
  } else if (OB_UNLIKELY(table_schema_->is_heap_table())) {
    ret = OB_NOT_SUPPORTED;
    LOG_WARN("not support heap table", KR(ret));
  }
  // init csv_parser_
  if (OB_FAIL(csv_parser_.init(load_stmt.get_data_struct_in_file(), field_or_var_list.count(),
                                  load_args.file_cs_type_))) {
    LOG_WARN("fail to init csv parser", KR(ret));
  }
  // init file_reader_
  if (OB_FAIL(file_reader_.open(load_args.full_file_path_))) {
    LOG_WARN("fail to open file", KR(ret), K(load_args.full_file_path_));
  }
  // init buffer_
  else {
    if (OB_FAIL(buffer_.create(FILE_BUFFER_SIZE))) {
      LOG_WARN("fail to create buffer", KR(ret));
    }
  }
  // // init row_caster_
  // if (OB_SUCCESS(ret)) {
  //     for (int i = 0; i < THREAD_POOL_SIZE; i++) {
  //     if (OB_FAIL(row_caster_[i].init(table_schema_, field_or_var_list))) {
  //       LOG_WARN("fail to init row caster", KR(ret));
  //     }
  //   }
  // }
  // init external_sort_
  for (int i = 0; i < TOTAL_BUCKET_NUM; i++) {
    if (OB_FAIL(external_sort_[i].init(table_schema_, MEM_BUFFER_SIZE, FILE_BUFFER_SIZE))) {
      LOG_WARN("fail to init row caster", KR(ret));
    }
  }
  // init sstable_writer_
  if (OB_FAIL(sstable_writer_.init(table_schema_))) {
    LOG_WARN("fail to init sstable writer", KR(ret));
  }
  const int64_t rowkey_column_num = table_schema_->get_rowkey_column_num();
  ObArray<ObColDesc> multi_version_column_descs;
  if (OB_FAIL(table_schema_->get_multi_version_column_descs(multi_version_column_descs))) {
    LOG_WARN("fail to get multi version column descs", KR(ret));
  } else if (OB_FAIL(datum_utils_.init(multi_version_column_descs, rowkey_column_num,
                                        is_oracle_mode(), allocator_))) {
    LOG_WARN("fail to init datum utils", KR(ret));
  } else if (OB_FAIL(compare_.init(rowkey_column_num, &datum_utils_))) {
    LOG_WARN("fail to init compare", KR(ret));
  }
  // for (int i = 0; i < THREAD_POOL_SIZE; i++) {
  //   allocators_[i].set_tenant_id(1);
  //   datum_utilss_[i].init(multi_version_column_descs, rowkey_column_num,
  //                     is_oracle_mode(), allocators_[i]);
  //   compares_[i].init(rowkey_column_num, &datum_utilss_[i]);
  // }
  sample_inited_ = false;
  sample_count_ = 0;
  thread_pool_.ob_load_data_direct_demo = this;
  thread_pool2_.ob_load_data_direct_demo = this;
  // memset(bucket_counter_, 0, sizeof(bucket_counter_));
  return ret;
}

void ObLoadDataDirectDemo::MyThreadPool::run1()
{ 
  
  ObTenantStatEstGuard stat_est_guard(MTL_ID());
  ObTenantBase *tenant_base = MTL_CTX();
  Worker::CompatMode mode = ((ObTenant *)tenant_base)->get_compat_mode();
  Worker::set_compatibility_mode(mode);
  uint64_t thread_id = get_thread_idx();
  LOG_INFO("[THREAD_ID]", K(thread_id));
  
  // do work

  int ret = OB_SUCCESS;
  const ObLoadDatumRow *datum_row = nullptr;

  ObLoadSSTableWriter & sstable_writer = ob_load_data_direct_demo->sstable_writer_;
  blocksstable::ObMacroBlockWriter macro_block_writer;
  sstable_writer.init_macro_block_writer(thread_id, macro_block_writer);
  int lower_bound = (TOTAL_BUCKET_NUM / THREAD_POOL_SIZE) * (thread_id);
  int upper_bound = thread_id == THREAD_POOL_SIZE - 1 ? TOTAL_BUCKET_NUM : (TOTAL_BUCKET_NUM / THREAD_POOL_SIZE) * (thread_id + 1);
  for (int i = lower_bound; i < upper_bound; i++) {
    // LOG_INFO("[BUCKET_COUNTER]", K(ob_load_data_direct_demo->bucket_counter_[i]));
    ObLoadExternalSort & external_sort = ob_load_data_direct_demo->external_sort_[i];
    if (OB_SUCC(ret)) {
      if (OB_FAIL(external_sort.close())) {
        LOG_WARN("fail to close external sort", KR(ret));
      }
    }
    while (OB_SUCC(ret)) {
      if (OB_FAIL(external_sort.get_next_row(datum_row))) {
        if (OB_UNLIKELY(OB_ITER_END != ret)) {
          LOG_WARN("fail to get next row", KR(ret));
        } else {
          ret = OB_SUCCESS;
          break;
        }
      } else {
        if (OB_FAIL(sstable_writer.append_row(thread_id, *datum_row, macro_block_writer))) {
          LOG_WARN("fail to append row", KR(ret));
        }
      }
    }
  }
  if (OB_SUCC(ret)) {
    if (OB_FAIL(sstable_writer.close_macro_block_writer(macro_block_writer))) {
      LOG_WARN("fail to close macro block writer", KR(ret));
    }
  }
  LOG_INFO("[THREAD_FINISH]", K(thread_id));
}

void ObLoadDataDirectDemo::MyThreadPool2::run1()
{
  ObTenantStatEstGuard stat_est_guard(MTL_ID());
  ObTenantBase *tenant_base = MTL_CTX();
  Worker::CompatMode mode = ((ObTenant *)tenant_base)->get_compat_mode();
  Worker::set_compatibility_mode(mode);
  uint64_t thread_id = get_thread_idx();
  //do work

  ObLoadDataBuffer &buffer = ob_load_data_direct_demo->buffer_;
  ObLoadCSVPaser &csv_parser = ob_load_data_direct_demo->csv_parser_;
  ObLoadRowCaster row_caster;

  ObLoadDataStmt *load_stmt_ = ob_load_data_direct_demo->load_stmt_;
  const ObIArray<ObLoadDataStmt::FieldOrVarStruct> &field_or_var_list =
    load_stmt_->get_field_or_var_list();
  row_caster.init(ob_load_data_direct_demo->table_schema_, field_or_var_list);

  int ret = OB_SUCCESS;
  const ObNewRow *next_row = nullptr;
  ObNewRow *ob_row = nullptr;
  common::ObVector<ObNewRow *> ob_row_vec;

  const ObLoadDatumRow *datum_row = nullptr;
  ObLoadDatumRow *new_item = NULL;

  char *buf = NULL;
  char *buf2 = NULL;
  int &sample_count = ob_load_data_direct_demo->sample_count_;
  common::ObArenaAllocator allocator;
  allocator.set_tenant_id(1);
  buf2 = static_cast<char *>(allocator.alloc(ALLOCATOR_SIZE));
  while (OB_SUCC(ret)) {
    ob_load_data_direct_demo->mutex2_.lock();
    if (OB_FAIL(buffer.squash())) {
      LOG_WARN("fail to squash buffer", KR(ret));
    } else {
      if (OB_FAIL(ob_load_data_direct_demo->file_reader_.read_next_buffer(buffer))) {
        if (OB_UNLIKELY(OB_ITER_END != ret)) {
          LOG_WARN("fail to read next buffer", KR(ret));
        } else {
          if (OB_UNLIKELY(!buffer.empty())) {
            ret = OB_ERR_UNEXPECTED;
            LOG_WARN("unexpected incomplate data", KR(ret));
          }
          ret = OB_SUCCESS;
          ob_load_data_direct_demo->mutex2_.unlock();
          break;
        }
      }
    }
    int offset = 0;
    while (OB_SUCC(ret)) {
      if (OB_FAIL(csv_parser.get_next_row(buffer, next_row))) {
        if (OB_UNLIKELY(OB_ITER_END != ret)) {
          LOG_WARN("fail to get next row", KR(ret));
        } else {
          ret = OB_SUCCESS;
          break;
        }
      } else {
        const int64_t item_size = sizeof(ObNewRow) + next_row->get_deep_copy_size();
        int64_t buf_pos = sizeof(ObNewRow);
        ob_row = new (buf2 + offset) ObNewRow();
        ob_row->deep_copy(*next_row, buf2 + offset, item_size, buf_pos);
        ob_row_vec.push_back(ob_row);
        offset += item_size;
      }
    }
    ob_load_data_direct_demo->mutex2_.unlock();

    for (int i = 0; i < ob_row_vec.size(); i++) {
      if (OB_FAIL(row_caster.get_casted_row(*ob_row_vec[i], datum_row))) {
        LOG_WARN("fail to cast row", KR(ret));
      } else {
        int sample_inited = ob_load_data_direct_demo->sample_inited_;
        if (!sample_inited) {
          ob_load_data_direct_demo->mutex_.lock();
          sample_inited = ob_load_data_direct_demo->sample_inited_;
          if (sample_inited) {
            ob_load_data_direct_demo->mutex_.unlock();
          }
        }
        if (!sample_inited) {
          const int64_t item_size = sizeof(ObLoadDatumRow) + datum_row->get_deep_copy_size();
          int64_t buf_pos = sizeof(ObLoadDatumRow);
          buf = static_cast<char *>(ob_load_data_direct_demo->allocator_.alloc(item_size));
          new_item = new (buf) ObLoadDatumRow();
          new_item->deep_copy(*datum_row, buf, item_size, buf_pos);
          ob_load_data_direct_demo->datumrow_list_.push_back(new_item);
          sample_count++;
          if (sample_count == SAMPLE_POOL_SIZE) {
            ob_load_data_direct_demo->generate_sample_datumrows();
          }
          ob_load_data_direct_demo->mutex_.unlock();
        } else {
          int bucket_index = 0;
          ob_load_data_direct_demo->mutex_.lock();
          ob_load_data_direct_demo->get_bucket_index(datum_row, bucket_index, thread_id);
          ob_load_data_direct_demo->mutex_.unlock();
          ob_load_data_direct_demo->mutex_for_bucket_[bucket_index].lock();
          // ob_load_data_direct_demo->bucket_counter_[bucket_index]++;
          ob_load_data_direct_demo->external_sort_[bucket_index].append_row(*datum_row);
          ob_load_data_direct_demo->mutex_for_bucket_[bucket_index].unlock();
        }
      }
    }
    ob_row_vec.clear();
  }
  allocator.reset();
}

int ObLoadDataDirectDemo::do_load()
{
  int ret = OB_SUCCESS;
  thread_pool2_.set_thread_count(THREAD_POOL_SIZE);
  thread_pool2_.set_run_wrapper(MTL_CTX());
  LOG_INFO("[THREAD_POOL2] start.");
  thread_pool2_.start();
  LOG_INFO("[THREAD_POOL2] wait.");
  thread_pool2_.wait();
  LOG_INFO("[THREAD_POOL2] stop.");
  thread_pool2_.stop();
  LOG_INFO("[THREAD_POOL2] destroy.");
  thread_pool2_.destroy();

  if (!sample_inited_) {
    generate_sample_datumrows();
  }

  thread_pool_.set_thread_count(THREAD_POOL_SIZE);
  thread_pool_.set_run_wrapper(MTL_CTX());
  LOG_INFO("[THREAD_POOL] start.");
  thread_pool_.start();
  LOG_INFO("[THREAD_POOL] wait.");
  thread_pool_.wait();
  LOG_INFO("[THREAD_POOL] stop.");
  thread_pool_.stop();
  LOG_INFO("[THREAD_POOL] destroy.");
  thread_pool_.destroy();
  LOG_INFO("[MTL_ID]", K(MTL_ID()));
  if (OB_FAIL(sstable_writer_.close())) {
    LOG_WARN("fail to close sstable writer", KR(ret));
  }
  return ret;
}

int ObLoadDataDirectDemo::generate_sample_datumrows()
{
  int ret = OB_SUCCESS;
  if (sample_inited_) {
    LOG_WARN("duplicate inited.");
    return ret;
  }
  sample_inited_ = true;
  std::sort(datumrow_list_.begin(), datumrow_list_.end(), compare_);
  for (int i = 1; i < TOTAL_BUCKET_NUM; i++) {
    sample_datumrows_.push_back(datumrow_list_[(datumrow_list_.size() - 1) / TOTAL_BUCKET_NUM * i]);
  }
  for (int i = 0; i < datumrow_list_.size(); i++) {
    int bucket_index = 0;
    get_bucket_index(datumrow_list_[i], bucket_index, -1);
    // bucket_counter_[bucket_index]++;
    external_sort_[bucket_index].append_row(*datumrow_list_[i]);
  }
  return ret;
}

int ObLoadDataDirectDemo::get_bucket_index(const ObLoadDatumRow *datum_row, int &bucket_index, int thread_id)
{
  int ret = OB_SUCCESS;
  // auto &compare = thread_id == -1 ? compare_ : compares_[thread_id];
  auto &compare = compare_;
  auto it = sample_datumrows_.lower_bound(datum_row, compare);
  bucket_index = it - sample_datumrows_.begin();
  return ret;
}

} // namespace sql
} // namespace oceanbase