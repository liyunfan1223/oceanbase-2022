#pragma once

#include "lib/file/ob_file.h"
#include "lib/timezone/ob_timezone_info.h"
#include "sql/engine/cmd/ob_load_data_impl.h"
#include "sql/engine/cmd/ob_load_data_parser.h"
#include "storage/blocksstable/ob_index_block_builder.h"
#include "storage/ob_parallel_external_sort.h"
#include "storage/tx_storage/ob_ls_handle.h"
#include "share/ob_thread_pool.h"

namespace oceanbase
{
namespace sql
{

class ObLoadDataBuffer
{
public:
  ObLoadDataBuffer();
  ~ObLoadDataBuffer();
  void reuse();
  void reset();
  int create(int64_t capacity);
  int squash();
  OB_INLINE char *data() const { return data_; }
  OB_INLINE char *begin() const { return data_ + begin_pos_; }
  OB_INLINE char *end() const { return data_ + end_pos_; }
  OB_INLINE bool empty() const { return end_pos_ == begin_pos_; }
  OB_INLINE int64_t get_data_size() const { return end_pos_ - begin_pos_; }
  OB_INLINE int64_t get_remain_size() const { return capacity_ - end_pos_; }
  OB_INLINE void consume(int64_t size) { begin_pos_ += size; }
  OB_INLINE void produce(int64_t size) { end_pos_ += size; }
private:
  common::ObArenaAllocator allocator_;
  char *data_;
  int64_t begin_pos_;
  int64_t end_pos_;
  int64_t capacity_;
};

class ObLoadSequentialFileReader
{
public:
  ObLoadSequentialFileReader();
  ~ObLoadSequentialFileReader();
  int open(const ObString &filepath);
  int read_next_buffer(ObLoadDataBuffer &buffer);
private:
  common::ObFileReader file_reader_;
  int64_t offset_;
  bool is_read_end_;
};

class ObLoadCSVPaser
{
public:
  ObLoadCSVPaser();
  ~ObLoadCSVPaser();
  void reset();
  int init(const ObDataInFileStruct &format, int64_t column_count,
           common::ObCollationType collation_type);
  int get_next_row(ObLoadDataBuffer &buffer, const common::ObNewRow *&row);
private:
  struct UnusedRowHandler
  {
    int operator()(common::ObIArray<ObCSVGeneralParser::FieldValue> &fields_per_line)
    {
      UNUSED(fields_per_line);
      return OB_SUCCESS;
    }
  };
private:
  common::ObArenaAllocator allocator_;
  common::ObCollationType collation_type_;
  ObCSVGeneralParser csv_parser_;
  common::ObNewRow row_;
  UnusedRowHandler unused_row_handler_;
  common::ObSEArray<ObCSVGeneralParser::LineErrRec, 1> err_records_;
  bool is_inited_;
};

class ObLoadDatumRow
{
  OB_UNIS_VERSION(1);
public:
  ObLoadDatumRow();
  ~ObLoadDatumRow();
  void reset();
  int init(int64_t capacity);
  int64_t get_deep_copy_size() const;
  int deep_copy(const ObLoadDatumRow &src, char *buf, int64_t len, int64_t &pos);
  OB_INLINE bool is_valid() const { return count_ > 0 && nullptr != datums_; }
  DECLARE_TO_STRING;
public:
  common::ObArenaAllocator allocator_;
  int64_t capacity_;
  int64_t count_;
  blocksstable::ObStorageDatum *datums_;
};

class ObLoadDatumRowCompare
{
public:
  ObLoadDatumRowCompare();
  ~ObLoadDatumRowCompare();
  int init(int64_t rowkey_column_num, const blocksstable::ObStorageDatumUtils *datum_utils);
  bool operator()(const ObLoadDatumRow *lhs, const ObLoadDatumRow *rhs);
  int get_error_code() const { return result_code_; }
public:
  int result_code_;
private:
  int64_t rowkey_column_num_;
  const blocksstable::ObStorageDatumUtils *datum_utils_;
  blocksstable::ObDatumRowkey lhs_rowkey_;
  blocksstable::ObDatumRowkey rhs_rowkey_;
  bool is_inited_;
};

class ObLoadRowCaster
{
public:
  ObLoadRowCaster();
  ~ObLoadRowCaster();
  int init(const share::schema::ObTableSchema *table_schema,
           const common::ObIArray<ObLoadDataStmt::FieldOrVarStruct> &field_or_var_list);
  int get_casted_row(const common::ObNewRow &new_row, const ObLoadDatumRow *&datum_row);
private:
  int init_column_schemas_and_idxs(
    const share::schema::ObTableSchema *table_schema,
    const common::ObIArray<ObLoadDataStmt::FieldOrVarStruct> &field_or_var_list);
  int cast_obj_to_datum(const share::schema::ObColumnSchemaV2 *column_schema,
                        const common::ObObj &obj, blocksstable::ObStorageDatum &datum);
private:
  common::ObArray<const share::schema::ObColumnSchemaV2 *> column_schemas_;
  common::ObArray<int64_t> column_idxs_; // Mapping of store columns to source data columns
  int64_t column_count_;
  common::ObCollationType collation_type_;
  ObLoadDatumRow datum_row_;
  common::ObArenaAllocator cast_allocator_;
  common::ObTimeZoneInfo tz_info_;
  bool is_inited_;
};

class ObLoadExternalSort
{
public:
  ObLoadExternalSort();
  ~ObLoadExternalSort();
  int init(const share::schema::ObTableSchema *table_schema, int64_t mem_size,
           int64_t file_buf_size);
  int append_row(const ObLoadDatumRow &datum_row);
  int close();
  int get_next_row(const ObLoadDatumRow *&datum_row);
private:
  common::ObArenaAllocator allocator_;
  blocksstable::ObStorageDatumUtils datum_utils_;
  ObLoadDatumRowCompare compare_;
  storage::ObExternalSort<ObLoadDatumRow, ObLoadDatumRowCompare> external_sort_;
  bool is_closed_;
  bool is_inited_;
};

class ObLoadSSTableWriter
{
  static const int64_t THREAD_POOL_SIZE = 32;
public:
  ObLoadSSTableWriter();
  ~ObLoadSSTableWriter();
  int init(const share::schema::ObTableSchema *table_schema);
  int append_row(int thread_id, const ObLoadDatumRow &datum_row, blocksstable::ObMacroBlockWriter &macro_block_writer);
  int close_macro_block_writer(blocksstable::ObMacroBlockWriter &macro_block_writer);
  int close();
  int init_macro_block_writer(uint64_t thread_id, blocksstable::ObMacroBlockWriter &macro_block_writer);
  blocksstable::ObDataStoreDesc data_store_desc_;
private:
  int init_sstable_index_builder(const share::schema::ObTableSchema *table_schema);
  int init_data_store_desc(const share::schema::ObTableSchema *table_schema);
  int create_sstable();
private:
  const ObTableSchema *table_schema_;
  common::ObTabletID tablet_id_;
  storage::ObTabletHandle tablet_handle_;
  share::ObLSID ls_id_;
  storage::ObLSHandle ls_handle_;
  int64_t rowkey_column_num_;
  int64_t extra_rowkey_column_num_;
  int64_t column_count_;
  storage::ObITable::TableKey table_key_;
  blocksstable::ObSSTableIndexBuilder sstable_index_builder_;
  // blocksstable::ObDatumRow datum_row_;
  blocksstable::ObDatumRow datum_rows_[THREAD_POOL_SIZE];
  bool is_closed_;
  bool is_inited_;
  int64_t closed_thread_;
};

class ObLoadDataDirectDemo : public ObLoadDataBase
{
  class MyThreadPool: public ObThreadPool
  {
    public:
    void run1() override;
    public:
    ObLoadDataDirectDemo * ob_load_data_direct_demo;
  };
  class MyThreadPool2: public ObThreadPool
  {
    public:
    void run1() override;
    public:
    ObLoadDataDirectDemo * ob_load_data_direct_demo;
  };
  static const int64_t MEM_BUFFER_SIZE = (256LL << 20);
  static const int64_t FILE_BUFFER_SIZE = (2LL << 20);
  static const int64_t ALLOCATOR_SIZE = FILE_BUFFER_SIZE << 2;
  static const int64_t THREAD_POOL_SIZE = 16;
  static const int64_t TOTAL_BUCKET_NUM = 32;
  static const int64_t SAMPLE_POOL_SIZE = 50000;
public:
  ObLoadDataDirectDemo();
  virtual ~ObLoadDataDirectDemo();
  int execute(ObExecContext &ctx, ObLoadDataStmt &load_stmt) override;
private:
  int inner_init(ObLoadDataStmt &load_stmt);
  int do_load();
  int generate_sample_datumrows();
  int get_bucket_index(const ObLoadDatumRow *datum_row, int &bucket_index, int thread_id);
  int init_csv_parser(ObLoadDataStmt &load_stmt);
  int init_buffer();
  int init_row_caster();
private:
  lib::ObMutex mutex_, mutex2_, mutex_for_bucket_[TOTAL_BUCKET_NUM];
  MyThreadPool thread_pool_;
  MyThreadPool2 thread_pool2_;
  int bucket_counter_[TOTAL_BUCKET_NUM];
  common::ObVector<ObLoadDatumRow *> datumrow_list_;
  common::ObSortedVector<ObLoadDatumRow *> sample_datumrows_;
  ObLoadDatumRowCompare compare_;
  // ObLoadDatumRowCompare compares_[THREAD_POOL_SIZE];
  common::ObArenaAllocator allocator_;
  // scommon::ObArenaAllocator allocators_[THREAD_POOL_SIZE];
  blocksstable::ObStorageDatumUtils datum_utils_;
  // blocksstable::ObStorageDatumUtils datum_utilss_[THREAD_POOL_SIZE];
  const share::schema::ObTableSchema *table_schema_ = nullptr;
  ObLoadDataStmt *load_stmt_;
  ObLoadCSVPaser csv_parser_;
  ObLoadSequentialFileReader file_reader_;
  ObLoadDataBuffer buffer_;
  ObLoadExternalSort external_sort_[TOTAL_BUCKET_NUM];
  ObLoadSSTableWriter sstable_writer_;
  int sample_inited_;
  int sample_count_;
};

} // namespace sql
} // namespace oceanbase