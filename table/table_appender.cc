#include "leveldb/table_appender.h"
#include <iostream>
#include "leveldb/env.h"

namespace leveldb {

TableAppender::TableAppender(const Options& options, RandomAccessFile* readfile, 
                            uint64_t offset, uint32_t footerlist_size, 
                            WritableFile* appendfile, uint32_t table_number)
    : TableBuilder(options, appendfile, offset), 
      origin_footerlist_offset_(offset), 
      origin_footerlist_size_(footerlist_size),
      readfile_(readfile),
      table_number_(table_number) { 
char footerlist_space[origin_footerlist_size_];
  Slice footerlist_input;
  s = readfile_->Read(origin_footerlist_offset_, origin_footerlist_size_,
                            &footerlist_input, footerlist_space);
  s = footerlist.DecodeFrom(&footerlist_input, table_number_);
}

TableAppender::~TableAppender() { }

Status TableAppender::InitialFooter(BlockHandle metaindex_block_handle, 
                                        BlockHandle index_block_handle,
                                        std::string& footer_encoding) {
  if (!s.ok()) {
    return s;
  }
  Footer footer;
  footer.set_metaindex_handle(metaindex_block_handle);
  footer.set_index_handle(index_block_handle);
  footerlist.append_new_footer(footer);
  //std::cout<<"YES:"<<index_block_handle.offset()<<" "<<index_block_handle.size()<<" "<<FileSize()<<std::endl;
  footerlist.EncodeTo(&footer_encoding);
  //std::cout<<footerlist.handle_list.size()<<" "<<footer_encoding.size()<<std::endl;
  return s;
}


}  // namespace leveldb
