#include "gtest/gtest.h"
#include "db/db_impl.h"
#include "leveldb/cache.h"
#include "leveldb/db.h"
#include "util/testutil.h"
#include <vector>
#include <algorithm>
namespace leveldb {

class AutoFloatTest : public testing::Test {
 public:
  AutoFloatTest() {
    dbname_ = testing::TempDir() + "autocompact_test";
    tiny_cache_ = NewLRUCache(100);
    options_.block_cache = tiny_cache_;
    DestroyDB(dbname_, options_);
    options_.create_if_missing = true;
    options_.compression = kNoCompression;
    EXPECT_LEVELDB_OK(DB::Open(options_, dbname_, &db_));
  }

  ~AutoFloatTest() {
    delete db_;
    DestroyDB(dbname_, Options());
    delete tiny_cache_;
  }

  std::string Key(int i) {
    char buf[100];
    snprintf(buf, sizeof(buf), "key%06d", i);
    return std::string(buf);
  }

  uint64_t Size(const Slice& start, const Slice& limit) {
    Range r(start, limit);
    uint64_t size;
    db_->GetApproximateSizes(&r, 1, &size);
    return size;
  }

  void DoReads(int n);

 private:
  std::string dbname_;
  Cache* tiny_cache_;
  Options options_;
  DB* db_;
};

static const int kValueSize = 200 * 1024;
static const int kTotalSize = 100 * 1024 * 1024;
static const int kCount = kTotalSize / kValueSize;

void AutoFloatTest::DoReads(int n) {
  std::string value(kValueSize, 'x');
  DBImpl* dbi = reinterpret_cast<DBImpl*>(db_);

  std::vector<int> keys;
  for (int i = 0; i < kCount; ++ i) {
    keys.push_back(i);
  }

  // Shuffle keys
  random_shuffle(keys.begin(),keys.end());

  // Fill database
  for (int i = 0; i < keys.size(); i++) {
    ASSERT_LEVELDB_OK(db_->Put(WriteOptions(), Key(keys[i]), value));
  }
  // ASSERT_LEVELDB_OK(dbi->TEST_CompactMemTable());

  // Get everything
  for (int i = kCount - 1; i >= 0; -- i) {
    for (int j = 0; j < i + 1; ++ j) {
      std::string value_in_db;
      if(!db_->Get(ReadOptions(), Key(i), &value_in_db).ok()) while(1);
      ASSERT_LEVELDB_OK(db_->Get(ReadOptions(), Key(i), &value_in_db));
      ASSERT_EQ(value, value_in_db);
    }
  }

  // Test floating
  for (int i = 0; i < kCount; ++ i) {
    for (int j = 0; j < kCount - i; ++ j) {
      std::string value_in_db;
      if(!db_->Get(ReadOptions(), Key(i), &value_in_db).ok()) while(1);
      ASSERT_LEVELDB_OK(db_->Get(ReadOptions(), Key(i), &value_in_db));
      ASSERT_EQ(value, value_in_db);
    }
  }
}

TEST_F(AutoFloatTest, ReadAll) { DoReads(kCount); }

}  // namespace leveldb

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
