#include "b_plus_tree.h"
#include <string>
#define B_ORDER 3

namespace bd2{

template<typename Record, typename Key>
class DataBase{
  using diskManager = std::shared_ptr<bd2::DiskManager>;
  using btree = bd2::BPlusTree<Key, B_ORDER>;
  long n_records;
  diskManager indexManager;
  diskManager recordManager;
  btree index;
  public:
      DataBase(){
          indexManager = std::make_shared<bd2::DiskManager>("data.index", true);
          recordManager = std::make_shared<bd2::DiskManager>("data.bin", true);
          index = btree (indexManager);
          n_records = 0;
          recordManager->write_record(0,n_records);
      }
      DataBase(diskManager &idxMan, diskManager &recMan){
          indexManager = idxMan;
          recordManager = recMan;
          index = btree (indexManager);
          recordManager->retrieve_record(0, n_records);
      }
      void insertWithoutIndex(Record &record){
          n_records++;
          recordManager->write_record(0, n_records);
          recordManager->write_record(n_records, record);
      }


      void insertWithBPlusTreeIndex(Record &record, Key &key_value){
          n_records++;
          recordManager->write_record(0, n_records);

          index.insert(key_value, n_records);
          index.showTree();

          recordManager->write_record(n_records, record);

      }


      Record readRecord(Key key_value){
          index.showTree();
          Record record;
          long pos = index.search(key_value);
          recordManager->retrieve_record(pos, record);
          return record;
      }

};
}