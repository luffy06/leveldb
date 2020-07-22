// Copyright (c) 2020 Shangyu Wu, Shenzhen. All rights reserved.
//
// TableAppender provides the interface used to append data to a Table.

#ifndef STORAGE_LEVELDB_INCLUDE_TABLE_APPENDER_H_
#define STORAGE_LEVELDB_INCLUDE_TABLE_APPENDER_H_

#include <stdint.h>

#include "leveldb/export.h"
#include "leveldb/options.h"
#include "leveldb/status.h"
#include "leveldb/table_builder.h"
#include "table/format.h"

namespace leveldb {

class BlockBuilder;
class BlockHandle;
class RandomAccessFile;
class WritableFile;

class LEVELDB_EXPORT TableAppender : public TableBuilder {
 public:
  TableAppender(const Options& options, RandomAccessFile* readfile, 
                uint64_t offset, uint32_t footerlist_size, 
                WritableFile* appendfile, uint32_t table_number);

  TableAppender(const TableAppender&) = delete;
  TableAppender& operator=(const TableAppender&) = delete;

  // REQUIRES: Either Finish() or Abandon() has been called.
  ~TableAppender();

  uint64_t ChangedFileSize() const {
    assert(FileSize() >= origin_footerlist_size_);
    return FileSize() - origin_footerlist_size_;
  }

 private:
  Status InitialFooter(BlockHandle metaindex_block_handle, 
                        BlockHandle index_block_handle,
                        std::string& footer_encoding) override;

  uint64_t origin_footerlist_offset_;
  uint32_t origin_footerlist_size_;
  uint32_t table_number_;
  RandomAccessFile* readfile_;
  FooterList footerlist_;
};

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_INCLUDE_TABLE_APPENDER_H_
