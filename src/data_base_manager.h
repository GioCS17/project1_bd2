#include "b_plus_tree.h"
#include "statichashing.h"
#include <string>
#include <fstream>
#include <sstream>
#include <utility>

#define B_ORDER 3

namespace bd2 {

    template<typename Record, typename Key, int gd = 10000, int fd = 20>
    class DataBase {
        using diskManager = std::shared_ptr<bd2::DiskManager>;
        using btree = bd2::BPlusTree<Key, B_ORDER>;
        using staticHashing = bd2::StaticHashing<Key, gd, fd>;
        long n_records;
        diskManager indexManager;
        diskManager recordManager;
        diskManager bucketManager;
        btree index;
        staticHashing indexSH;
        int kind_of_index;
    public:
        DataBase(int k_index = 0) {
            n_records = 0;
            recordManager = std::make_shared<bd2::DiskManager>("data.bin", true);
            kind_of_index = k_index;
            if (k_index == 0) {
                indexManager = std::make_shared<bd2::DiskManager>("data.index", true);
                index = btree(indexManager);
            }
            if (k_index == 1) {
                bucketManager = std::make_shared<bd2::DiskManager>("bucket.bin", true);
                indexSH = staticHashing(bucketManager, recordManager);
            }
        }

        DataBase(diskManager idxMan, diskManager recMan, int _n_records, int k_index = 0) {
            recordManager = std::move(recMan);
            kind_of_index = k_index;
            if (kind_of_index == 0){
                indexManager = std::move(idxMan);
                index = btree(indexManager);
            }
            if (kind_of_index == 1){
                bucketManager = idxMan;
                indexSH = staticHashing (bucketManager, recordManager);
            }
            n_records = _n_records;
        }

        void insertWithoutIndex(Record &record) {
            recordManager->write_record(n_records, record);
            n_records++;
        }

        void loadFromExternalFile(const std::string &filename) {
            std::fstream fileIn;
            fileIn.open(filename, std::ios::in | std::ios::binary);
            Record r;
            while (fileIn.read((char *) &r, sizeof(r))) {
                //r.show();
                if (kind_of_index == 0)
                    insertWithBPlusTreeIndex(r, r.id, false);

                else if (kind_of_index == 1)
                    insertWithStaticHashing(r);
            }
            fileIn.close();
        }

        bool insertWithBPlusTreeIndex(Record &record, Key &key_value, bool checkIsTheKeyExist) {
            if (checkIsTheKeyExist) {
                if (!index.isKeyPresent(key_value)) {
                    index.insert(key_value, n_records);
                    recordManager->write_record(n_records, record);
                    n_records++;
                    return true;
                }
                return false;
            } else {
                index.insert(key_value, n_records);
                recordManager->write_record(n_records, record);
                n_records++;
                return true;
            }
        }

        bool readRecord(Record &record, Key key_value) {
            int disk_access = 0;
            long record_pos = index.getRecordIdByKeyValue(key_value, disk_access);
            if (record_pos != -1) {
                recordManager->retrieve_record(record_pos, record);
                std::cout << "Disk access: " << disk_access << std::endl;
                return true;
            }
            return false;
        }


        void showTreeIndex() {
            index.showTree();
        }

        //Statis Hashing Methods
        void insertWithStaticHashing(Record &record) {
            recordManager->write_record(n_records, record);
            indexSH.insert(n_records, record.id);
            n_records++;
        }
        bool readRecord_SH(Record &record,Key key_value){
          long record_pos = indexSH.search(key_value);
          if(record_pos!=-1){
            recordManager->retrieve_record(record_pos,record);
            return true;
          }
          return false;
        }
        void showStaticHashingIndex() {
            indexSH.print();
        }
    };
}
