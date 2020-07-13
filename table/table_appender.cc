#include "leveldb/table_appender.h"

#include "leveldb/env.h"

namespace leveldb {

TableAppender::TableAppender(const Options& options, RandomAccessFile* readfile, 
                            uint64_t offset, uint32_t footerlist_size, 
                            WritableFile* appendfile, uint32_t table_number)
    : TableBuilder(options, appendfile), 
      origin_footerlist_offset_(offset), 
      origin_footerlist_size_(footerlist_size),
      readfile_(readfile),
      table_number_(table_number) { }

TableAppender::~TableAppender() { }

Status TableAppender::InitialFooter(BlockHandle metaindex_block_handle, 
                                        BlockHandle index_block_handle,
                                        std::string& footer_encoding) {
  char footerlist_space[origin_footerlist_size_];
  Slice footerlist_input;
  Status s = readfile_->Read(origin_footerlist_offset_, origin_footerlist_size_,
                            &footerlist_input, footerlist_space);
  if (!s.ok()) { 
    return s;
  }

  FooterList footerlist;
  s = footerlist.DecodeFrom(&footerlist_input, table_number_);
  if (!s.ok()) {
    return s;
  }
  
  Footer footer;
  footer.set_metaindex_handle(metaindex_block_handle);
  footer.set_index_handle(index_block_handle);
  footerlist.append_new_footer(footer);
  footerlist.EncodeTo(&footer_encoding);
  return s;
}


}  // namespace leveldb
