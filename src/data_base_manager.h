/**
 * @file data_base_manager.h
 * @author Juan Vargas Castillo (juan.vargas@utec.edu.pe)
 * @author Giordano Alvitez Falcón (giordano.alvitez@utec.edu.pe)
 * @author Roosevelt.Ubaldo Chavez (roosevelt.ubaldo@utec.edu.pe)
 * @brief Database manager, it permit to insert records using indexes
 * like Static Hashing or B+Tree. It is possible to insert without
 * indexes
 * @version 0.1
 * @date 2020-05-15
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#include "b_plus_tree.h"
#include "statichashing.h"
#include <string>
#include <fstream>
#include <sstream>
#include <utility>
#include <thread>

#define B_ORDER 1000

namespace bd2 {
/**
 * @brief Database Manager object
 * 
 * @tparam Record structure of the record to be inserted
 * @tparam Key the type of the record key 
 * @tparam 10000 global depth of the static hashing
 * @tparam 20 
 */
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

        /**
         * @brief Construct a new Data Base object
         * 
         * @param k_index type of index to be selected, 
         * (0) B+Tree (1)Static Hashing (else) Without Index
         */
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

        /**
         * @brief Construct a new Data Base object
         * 
         * @param idxMan disk manager for the index
         * @param recMan disk manager for the records
         * @param _n_records number of records
         * @param k_index type of index
         */
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

        /**
         * @brief Insert without index
         * 
         * @param record record to be inserted 
         */
        void insertWithoutIndex(Record &record) {
            recordManager->write_record(n_records, record);
            n_records++;
        }

        /**
         * @brief Sequential search without indexes
         * 
         * @param record record in which we are going to store the result
         * @param key_value value to be finded
         * @param disk_access quantity of disk access
         */
        void findWithoutIndex(Record &record, Key key_value, int &disk_access){
            disk_access = 0;
            for (int i = 0; i < n_records; i++){
                recordManager->retrieve_record(i, record);
                disk_access++;
                if (record.id == key_value){
                    std::cout << "Disk access: " << disk_access << std::endl;
                    break;
                }
            }
        }

        /**
         * @brief Load data to the Database from an external file
         * 
         * @param filename filename of the data
         */
        void loadFromExternalFile(const std::string &filename) {
            std::fstream fileIn;
            fileIn.open(filename, std::ios::in | std::ios::binary);
            Record r;
            while (fileIn.read((char *) &r, sizeof(r))) {
                //r.show();
                if (kind_of_index == 0)
                    insertWithBPlusTreeIndex(r, r.id, false);
                else if (kind_of_index == 1) {
                    //std::cout<<"ID register::"<<r.id<<std::endl;
                    insertWithStaticHashing(r);
                }else{
                    insertWithoutIndex(r);
                }
            }
            fileIn.close();
        }

        /**
         * @brief Insert with B+Tree index
         * 
         * @param record record to be inserted
         * @param key_value key value
         * @param checkIsTheKeyExist bool to check if the key already exist
         * @return true insert successfull
         * @return false insertion wrong
         */
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

        /**
         * @brief Read a record with B+Tree index
         * 
         * @param record record in which we are going to store the result
         * @param key_value key of the record finded
         * @return true the key exist
         * @return false the key doesn't exist
         */
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


        /**
         * @brief Make a Range Search using B+Tree
         * 
         * @param vector_record Vector in which we are going to store the result
         * @param first first key value
         * @param last last key value
         * @return true successfull
         * @return false wrong
         */
        bool readRecordRange (std::vector<Record> &vector_record, Key first, Key last){
            std::vector <long> pos_records = index.range_search (first, last);
            for (long pos_ : pos_records){
                if (pos_ != -1) {
                    Record new_r;
                    recordManager->retrieve_record(pos_, new_r);
                    vector_record.push_back (new_r);
                }
            }
            if (vector_record.size () > 0)
                return true;
            return false;
        }

        /**
         * @brief Show the B+Tree Index to the console
         * 
         */
        void showTreeIndex() {
            index.showTree();
        }

        //Statis Hashing Methods

        /**
         * @brief Insetion with Static Hashing
         * 
         * @param record record to be inserted
         */
        void insertWithStaticHashing(Record &record) {
            recordManager->write_record(n_records, record);
            indexSH.insert(n_records, record.id);
            n_records++;
        }

        /**
         * @brief Read a record with Static Hashing
         * 
         * @param record 
         * @param key_value 
         * @return true 
         * @return false 
         */
        bool readRecord_SH(Record &record,Key key_value){
          long record_pos = indexSH.search(key_value);
          if(record_pos!=-1){
            recordManager->retrieve_record(record_pos,record);
            return true;
          }
          retur
          n false;
        }
        void showStaticHashingIndex() {
            indexSH.print();
        }

        //Methos With Threads
        void insertWithThreads(int size,int n_threads){
            std::vector<std::thread> threads;
           // for(int i=0;i<)
        }
        void insertThread(long begin,long end){
            //recordManager->write_record(n_records, record);
            //indexSH.insert(n_records, record.id);
            n_records++;
        }
    };
}
