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



      void loadFromExternalFile(const std::string& filename, int n_fields,  bool isBinary){
          std::fstream fileIn;
          if (isBinary){
              std::cout << "gaaa";
          }else{
              fileIn.open(filename, std::ios::in);
              std::vector<std::string> row;
              std::string line, word;
              while(getline(fileIn, line)){
                  Record record;
                  row.clear();
                  std::stringstream s(line);
                  while (getline(s, word, ',')){
                      row.push_back(word);
                  }
                  long pk = stol(row[0]);
                  row.erase(row.begin());
                  record.id = pk;
                  record.args = row;
                  insertWithBPlusTreeIndex(record, record.id);

                  std::cout << "pk: " << record.id << std::endl;
                  for (int i = 0; i < row.size(); i++){
                      std::cout << "args " << i + 1 << ": " << row[i] << std::endl;
                  }
                  std::cout << "--------------------------------" << std::endl;
              }
              fileIn.close();
          }
      }

    bool insertWithBPlusTreeIndex(Record &record, Key &key_value){
        if (!index.findKey(key_value)){
            index.insert(key_value, n_records);
            recordManager->write_record(n_records, record);
            n_records++;
            return true;
        }
        return false;
    }




      bool readRecord(Record &record, Key key_value){
          long record_pos = index.findRecordId(key_value);
          if (record_pos != -1 ){
              recordManager->retrieve_record(record_pos, record);
              return true;
          }
          return false;
      }

};
}