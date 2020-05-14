#include "b_plus_tree.h"
#include <string>
#include <fstream>
#include <sstream>

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
      }

      DataBase(diskManager &idxMan, diskManager &recMan){
          indexManager = idxMan;
          recordManager = recMan;
          index = btree (indexManager);

      }
      void insertWithoutIndex(Record &record){
          recordManager->write_record(n_records, record);
          n_records++;
      }
      void loadFromExternalFile(const std::string& filename){
          std::fstream fileIn;
          fileIn.open(filename, std::ios::in | std::ios::binary);
          Record r;
          while (fileIn.read((char *)&r, sizeof(r))){
              //r.show();
              insertWithBPlusTreeIndex(r, r.id, false);
          }
          fileIn.close();
      }

    bool insertWithBPlusTreeIndex(Record &record, Key &key_value, bool checkIsTheKeyExist){
          if (checkIsTheKeyExist){
              if (!index.isKeyPresent(key_value)){
                  index.insert(key_value, n_records);
                  recordManager->write_record(n_records, record);
                  n_records++;
                  return true;
              }
              return false;
          }else{
              index.insert(key_value, n_records);
              recordManager->write_record(n_records, record);
              n_records++;
              return true;
          }
    }
      bool readRecord(Record &record, Key key_value){
          //index.showTree();
          long record_pos = index.getRecordIdByKeyValue(key_value);
          if (record_pos != -1 ){
              recordManager->retrieve_record(record_pos, record);
              return true;
          }
          return false;
      }

};
}