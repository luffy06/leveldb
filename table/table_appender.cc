#include "leveldb/table_appender.h"

namespace leveldb {

TableAppender::TableAppender(const Options& options, RandomAccessFile* readfile, 
                            uint64_t offset, int footerlist_size, 
                            WritableFile* appendfile)
    : super(options, appendfile), 
      origin_footerlist_offset_(offset), 
      origin_footerlist_size_(footerlist_size),
      readfile_(readfile) { }

TableAppender::~TableAppender() { }

Status TableBuilder::InitialFooter(BlockHandle metaindex_block_handle, 
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
  s = footerlist.DecodeFrom(&footerlist_input);
  if (!s.ok()) {
    return s;
  }
  
  Footer footer;
  footer.set_metaindex_handle(metaindex_block_handle);
  footer.set_index_handle(index_block_handle);
  footer_list.append_new_footer(footer)
  std::string footer_encoding;
  footer_list.EncodeTo(&footer_encoding);
  return s;
}


}  // namespace leveldb
