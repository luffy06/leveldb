// Copyright (c) 2020 Shangyu Wu, Shenzhen. All rights reserved.
//
// TableAppender provides the interface used to append data to a Table.

#ifndef STORAGE_LEVELDB_INCLUDE_TABLE_APPENDER_H_
#define STORAGE_LEVELDB_INCLUDE_TABLE_APPENDER_H_

#include <stdint.h>

#include "leveldb/export.h"
#include "leveldb/options.h"
#include "leveldb/status.h"
#include "leveldb/table_build.h"

namespace leveldb {

class BlockBuilder;
class BlockHandle;
class RandomAccessFile;
class WritableFile;

class LEVELDB_EXPORT TableAppender : public TableBuilder {
 public:
  TableAppender(const Options& options, RandomAccessFile* readfile, 
                uint64_t offset, int footerlist_size, WritableFile* appendfile);

  TableAppender(const TableAppender&) = delete;
  TableAppender& operator=(const TableAppender&) = delete;

  // REQUIRES: Either Finish() or Abandon() has been called.
  ~TableAppender();

 private:
  Status InitialFooter(BlockHandle metaindex_block_handle, 
                        BlockHandle index_block_handle,
                        std::string& footer_encoding) override;

  uint64_t origin_footerlist_offset_;
  int origin_footerlist_size_;
  RandomAccessFile* readfile_;

};

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_INCLUDE_TABLE_APPENDER_H_
