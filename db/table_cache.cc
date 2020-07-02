// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "db/table_cache.h"

#include "db/filename.h"
#include "leveldb/env.h"
#include "leveldb/table.h"
#include "table/merger.h"
#include "util/coding.h"

namespace leveldb {

struct TableAndFile {
  RandomAccessFile* file;
  std::vector<Table*> tables;
};

static void DeleteEntry(const Slice& key, void* value) {
  TableAndFile* tf = reinterpret_cast<TableAndFile*>(value);
  for (int i = 0; i < tf->tables.size(); ++ i)
    delete tf->tables[i];
  delete tf->file;
  delete tf;
}

static void UnrefEntry(void* arg1, void* arg2) {
  Cache* cache = reinterpret_cast<Cache*>(arg1);
  Cache::Handle* h = reinterpret_cast<Cache::Handle*>(arg2);
  cache->Release(h);
}

TableCache::TableCache(const std::string& dbname, const Options& options,
                       int entries)
    : env_(options.env),
      dbname_(dbname),
      options_(options),
      cache_(NewLRUCache(entries)) {}

TableCache::~TableCache() { delete cache_; }

Status TableCache::FindTable(uint64_t file_number, uint64_t file_size,
                             uint32_t table_number, Cache::Handle** handle) {
  Status s;
  char buf[sizeof(file_number)];
  EncodeFixed64(buf, file_number);
  Slice key(buf, sizeof(buf));
  *handle = cache_->Lookup(key);
  if (*handle == nullptr) {
    std::string fname = TableFileName(dbname_, file_number);
    RandomAccessFile* file = nullptr;
    std::vector<Table*> tables;
    s = env_->NewRandomAccessFile(fname, &file);
    if (!s.ok()) {
      std::string old_fname = SSTTableFileName(dbname_, file_number);
      if (env_->NewRandomAccessFile(old_fname, &file).ok()) {
        s = Status::OK();
      }
    }
    if (s.ok()) {
      s = Table::Open(options_, file, file_size, table_number, tables);
    }

    if (!s.ok()) {
      for (int i = 0; i < tables.size(); ++ i)
        assert(tables[i] == nullptr);
      delete file;
      // We do not cache error results so that if the error is transient,
      // or somebody repairs the file, we recover automatically.
    } else {
      TableAndFile* tf = new TableAndFile;
      tf->file = file;
      tf->tables = tables;
      *handle = cache_->Insert(key, tf, 1, &DeleteEntry);
    }
  }
  return s;
}

Iterator* TableCache::NewIterator(const ReadOptions& options,
                                  uint64_t file_number, uint64_t file_size, 
                                  uint32_t table_number, Table*** tableptr) {
  if (tableptr != nullptr) {
    for (size_t i = 0; i < table_number; ++ i) {
      if (tableptr[i] != nullptr)
        *(tableptr[i]) = nullptr;
    }
  }

  Cache::Handle* handle = nullptr;
  Status s = FindTable(file_number, file_size, table_number, &handle);
  if (!s.ok()) {
    return NewErrorIterator(s);
  }
  std::vector<Table*> tables = reinterpret_cast<TableAndFile*>(
                                                cache_->Value(handle))->tables;
  std::vector<Iterator*> table_iterators;
  for (int i = 0; i < tables.size(); ++ i) {
    Iterator* iter = tables[i]->NewIterator(options);
    table_iterators.push_back(iter);
    iter->RegisterCleanup(&UnrefEntry, cache_, handle);
  }
  // TODO(floating): Verify that whether the key is encoded with sequence number
  Iterator* result = NewMergingIterator(options_.comparator, 
                                        &table_iterators[0], 
                                        table_iterators.size());
  if (tableptr != nullptr) {
    for (size_t i = 0; i < tables.size(); ++ i) {
      *(tableptr[i]) = tables[i];
    }    
  }
  return result;
}

Status TableCache::Get(const ReadOptions& options, uint64_t file_number,
                       uint64_t file_size, uint32_t table_number, 
                       const Slice& k, void* arg,
                       void (*handle_result)(void*, const Slice&,
                                             const Slice&)) {
  Cache::Handle* handle = nullptr;
  Status s = FindTable(file_number, file_size, table_number, &handle);
  if (s.ok()) {
    std::vector<Table*> t_list = reinterpret_cast<TableAndFile*>(
                                                cache_->Value(handle))->tables;
    for (int i = 0; i < t_list.size(); ++ i) {
      // TODO(floating): process it based on the range 
      s = t_list[i]->InternalGet(options, k, arg, handle_result);
      if (s.ok()) // DEBUG: verify the condition
        break;
    }
    cache_->Release(handle);
  }
  return s;
}

void TableCache::Evict(uint64_t file_number) {
  char buf[sizeof(file_number)];
  EncodeFixed64(buf, file_number);
  cache_->Erase(Slice(buf, sizeof(buf)));
}

}  // namespace leveldb
