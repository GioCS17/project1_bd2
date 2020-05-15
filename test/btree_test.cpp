#include <gtest/gtest.h>
#include <b_plus_tree.h>
#include <disk_manager.h>
#include <data_base_manager.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#define PAGE_SIZE  64
#define BTREE_ORDER   ((PAGE_SIZE - (2 * sizeof(long) + sizeof(int) +  2 * sizeof(long)) ) /  (sizeof(int) + sizeof(long)))
using namespace std::chrono;

struct DiskBasedBtree : public ::testing::Test
{
};


TEST_F(DiskBasedBtree, IndexingRandomElements) {
  bool trunc_file = true;
  std::shared_ptr<bd2::DiskManager> pm = std::make_shared<bd2::DiskManager>("btree.index", trunc_file);
  std::cout << "PAGE_SIZE: " << PAGE_SIZE << std::endl;
  std::cout << "BTREE_ORDER: " << BTREE_ORDER << std::endl;
  bd2::BPlusTree<char, BTREE_ORDER> bt(pm);

  std::string values = "poiuytrewqasdfghjklmnbvcxz";
  for(auto c : values) {
      if (c == 'u'){
          std::cout << "gaa" << std::endl;
      }
      bt.insert(c);
      bt.showTree();
  }
  bt.showTree();
  std::ostringstream out;
  bt.print(out);
  std::sort(values.begin(), values.end());
  EXPECT_EQ(out.str(), values.c_str());
}

TEST_F(DiskBasedBtree, Persistence) {
    std::shared_ptr<bd2::DiskManager> pm = std::make_shared<bd2::DiskManager>("btree.index");
    bd2::BPlusTree<char, BTREE_ORDER> bt(pm);
    std::string values = "1986432";
    for(auto c : values) {
        bt.insert(c);
        bt.showTree();
    }
    std::ostringstream out;
    bt.print(out);
    std::string all_values = "poiuytrewqasdfghjklmnbvcxz1986432";
    std::sort(all_values.begin(), all_values.end());
    EXPECT_EQ(out.str(), all_values.c_str());
}

TEST_F(DiskBasedBtree, IteratorFrom) {
  std::shared_ptr<bd2::DiskManager> pm = std::make_shared<bd2::DiskManager>("btree.index");
  using char_btree = bd2::BPlusTree<char, BTREE_ORDER>;
  using char_btree_iterator = bd2::BPlusTreeIterator<char, BTREE_ORDER>;
  char_btree bt(pm);
  char_btree_iterator iter = bt.begin();
  std:: string iter_values;
  for(; iter != bt.null(); iter++) {
      iter_values.push_back(*iter);
      std::cout << *iter << ", ";
  }
  std::cout << std::endl;
  std::string all_values = "poiuytrewqasdfghjklmnbvcxz1986432";
  std::sort(all_values.begin(), all_values.end());
  EXPECT_EQ(all_values, iter_values);
}

TEST_F(DiskBasedBtree, IteratorBack) {
    std::shared_ptr<bd2::DiskManager> pm = std::make_shared<bd2::DiskManager>("btree.index");
    using char_btree = bd2::BPlusTree<char, BTREE_ORDER>;
    using char_btree_iterator = bd2::BPlusTreeIterator<char, BTREE_ORDER>;
    char_btree bt(pm);
    char_btree_iterator iter = bt.end();
    std:: string iter_values;
    for(; iter != bt.null(); iter--) {
        iter_values.push_back(*iter);
        std::cout << *iter << ", ";
    }
    std::cout << std::endl;
    std::string all_values = "poiuytrewqasdfghjklmnbvcxz1986432";
    std::sort(all_values.begin(), all_values.end(), std::greater <>());
    EXPECT_EQ(all_values, iter_values);
}

TEST_F(DiskBasedBtree, IteratorFromRandomPositionFrom) {
    std::shared_ptr<bd2::DiskManager> pm = std::make_shared<bd2::DiskManager>("btree.index");
    using char_btree = bd2::BPlusTree<char, BTREE_ORDER>;
    using char_btree_iterator = bd2::BPlusTreeIterator<char, BTREE_ORDER>;
    char_btree bt(pm);
    long record_id;
    int key_pos;
    bt.find('j', record_id, key_pos);
    char_btree_iterator iter (pm, record_id, key_pos);
    std:: string iter_values;
    for(; iter != bt.null(); iter++) {
        iter_values.push_back(*iter);
        std::cout << *iter << ", ";
    }
    std::cout << std::endl;
    std::string all_values = "jklmnopqrstuvwxyz";
    std::sort(all_values.begin(), all_values.end());
    EXPECT_EQ(all_values, iter_values);
}

TEST_F(DiskBasedBtree, IteratorFromRandomPositionBack) {
    std::shared_ptr<bd2::DiskManager> pm = std::make_shared<bd2::DiskManager>("btree.index");
    using char_btree = bd2::BPlusTree<char, BTREE_ORDER>;
    using char_btree_iterator = bd2::BPlusTreeIterator<char, BTREE_ORDER>;
    char_btree bt(pm);
    long record_id;
    int key_pos;
    bt.find('9', record_id, key_pos);
    char_btree_iterator iter (pm, record_id, key_pos);
    std:: string iter_values;
    for(; iter != bt.null(); iter--) {
        iter_values.push_back(*iter);
        std::cout << *iter << ", ";
    }
    std::cout << std::endl;
    std::string all_values = "9684321";
    std::sort(all_values.begin(), all_values.end(), std::greater <>());
    EXPECT_EQ(all_values, iter_values);
}

/*
TEST_F(DiskBasedBtree, InsertCharIndex) {
    std::shared_ptr<bd2::DiskManager> pm = std::make_shared<bd2::DiskManager>("btree_char.index");
    using char_btree = bd2::BPlusTree<std::string, BTREE_ORDER>;
    char_btree bt(pm);
    std::string key = "alabama";
    bt.insert(key);
    bt.showTree();
}
*/


TEST_F(DiskBasedBtree, DatabaseInsert){
    struct Student {
        long  id;
        bool passed;
        char name[32];
        char surname[32];
        int  n_credits;

        void show(){
            std:: cout << "id: " << id << std::endl;
            std:: cout << "passed: " << passed << std::endl;
            std:: cout << "name: " << name << std::endl;
            std:: cout << "surname: " << surname << std::endl;
            std:: cout << "n_credits: " << n_credits << std::endl;
            std::cout << "--------------------------------" << std::endl;

        }
    };
    using database = bd2::DataBase<Student, long>;
    bd2::DataBase<Student, long> db = bd2::DataBase<Student, long>();
    Student p {10, false,"Alex","Kouri",150};
    db.insertWithBPlusTreeIndex(p,p.id, true);
    Student p1 {20, true,"Luis","Fujimori",10};
    db.insertWithBPlusTreeIndex(p1, p1.id, true);
    Student p2 {30, true,"Martin","Lapierre",120};
    db.insertWithBPlusTreeIndex(p2, p2.id, true);
    Student p3 {40, false,"Heider","McDonalds",110};
    db.insertWithBPlusTreeIndex(p3, p3.id, true);
    Student p4 {50, true,"Juan","Schmitchel",122};
    db.insertWithBPlusTreeIndex(p4, p4.id, true);
    Student p5 {60, false,"Peter","Schwaun",101};
    db.insertWithBPlusTreeIndex(p5, p5.id, true);
    for (int i = 10; i < 61; i = i + 10){
        Student p_last{};
        bool find = db.readRecord(p_last, i);
        if (find){
            std::cout << "****************************************" << std::endl;
            std::cout << "Key finded: " << i << std::endl;
            std::cout << "--------------------------------" << std::endl;
            std::cout << "Record obtained: "<< std::endl;
            p_last.show();
            EXPECT_EQ(i, p_last.id);
        }
    }
}
TEST_F(DiskBasedBtree, InsertFromCSV) {
    int n = 10;
    struct Default
    {
        int id;
        char description [249];
        char city [30];
        char state [2];
        char weather [35];

        void show(){
            std:: cout << "id: " << id << std::endl;
            std:: cout << "desc: " << description << std::endl;
            std:: cout << "city: " << city << std::endl;
            std:: cout << "state: " << state << std::endl;
            std:: cout << "weather: " << weather << std::endl;
            std:: cout << "-------------------------------" << std::endl;        }
    };
    bd2::DataBase<Default, int> db = bd2::DataBase<Default, int>();
    db.loadFromExternalFile("datashort.bin");
    Default d;
    db.readRecord(d, 2);
    d.show();
    EXPECT_EQ(d.id, 2);
}
struct Default
{
    int id;
    char description [249];
    char city [30];
    char state [4];
    char weather [35];

    void show(){
        std:: cout << "id: " << id << std::endl;
        std:: cout << "desc: " << description << std::endl;
        std:: cout << "city: " << city << std::endl;
        std:: cout << "state: " << state << std::endl;
        std:: cout << "weather: " << weather << std::endl;
        std:: cout << "-------------------------------" << std::endl;        }
};
TEST_F(DiskBasedBtree, Insert1k) {
    std::shared_ptr<bd2::DiskManager> data = std::make_shared<bd2::DiskManager>("btree.dat", true);
    std::shared_ptr<bd2::DiskManager> index = std::make_shared<bd2::DiskManager>("btree.index", true);
    bd2::DataBase<Default, int> db = bd2::DataBase<Default, int>(index, data, 0);
    db.loadFromExternalFile("data1k.bin");
    //db.showTreeIndex();
}

TEST_F(DiskBasedBtree, Insert10k) {
    std::shared_ptr<bd2::DiskManager> data = std::make_shared<bd2::DiskManager>("btree10k.dat", true);
    std::shared_ptr<bd2::DiskManager> index = std::make_shared<bd2::DiskManager>("btree10k.index", true);
    bd2::DataBase<Default, int> db = bd2::DataBase<Default, int>(index, data, 0);
    db.loadFromExternalFile("data10k.bin");
    //db.showTreeIndex();
}
TEST_F(DiskBasedBtree, Insert100k) {
    std::shared_ptr<bd2::DiskManager> data = std::make_shared<bd2::DiskManager>("btree100k.dat", true);
    std::shared_ptr<bd2::DiskManager> index = std::make_shared<bd2::DiskManager>("btree100k.index", true);
    bd2::DataBase<Default, int> db = bd2::DataBase<Default, int>(index, data, 0);
    db.loadFromExternalFile("data100k.bin");
    //db.showTreeIndex();
}

TEST_F(DiskBasedBtree, Insert1M) {
    std::shared_ptr<bd2::DiskManager> data = std::make_shared<bd2::DiskManager>("btree1M.dat", true);
    std::shared_ptr<bd2::DiskManager> index = std::make_shared<bd2::DiskManager>("btree1M.index", true);
    bd2::DataBase<Default, int> db = bd2::DataBase<Default, int>(index, data, 0);
    db.loadFromExternalFile("data1M.bin");
    //db.showTreeIndex();
}

TEST_F(DiskBasedBtree, Find1k) {
    std::shared_ptr<bd2::DiskManager> data = std::make_shared<bd2::DiskManager>("btree.dat", false);
    std::shared_ptr<bd2::DiskManager> index = std::make_shared<bd2::DiskManager>("btree.index", false);
    bd2::DataBase<Default, int> db = bd2::DataBase<Default, int>(index, data, 1000);
    Default r;
    srand(time(NULL));
    int idx = rand()%1000+1;

    db.readRecord(r, idx);
    std::cout << "Idx finded: " << idx << std::endl;
    r.show();
}

TEST_F(DiskBasedBtree, Find10k) {
    std::shared_ptr<bd2::DiskManager> data = std::make_shared<bd2::DiskManager>("btree10k.dat", false);
    std::shared_ptr<bd2::DiskManager> index = std::make_shared<bd2::DiskManager>("btree10k.index", false);
    bd2::DataBase<Default, int> db = bd2::DataBase<Default, int>(index, data, 10000);
    Default r;
    srand(time(NULL));
    int idx = rand()%10000+1;
    db.readRecord(r, idx);
    std::cout << "Idx finded: " << idx << std::endl;
    r.show();
}
TEST_F(DiskBasedBtree, Find100k) {
    std::shared_ptr<bd2::DiskManager> data = std::make_shared<bd2::DiskManager>("btree100k.dat", false);
    std::shared_ptr<bd2::DiskManager> index = std::make_shared<bd2::DiskManager>("btree100k.index", false);
    bd2::DataBase<Default, int> db = bd2::DataBase<Default, int>(index, data, 100000);
    Default r;
    srand(time(NULL));
    int idx = rand()%100000+1;
    db.readRecord(r, idx);
    std::cout << "Idx finded: " << idx << std::endl;
    r.show();
    //db.showTreeIndex();
}

TEST_F(DiskBasedBtree, Find1M) {
    std::shared_ptr<bd2::DiskManager> data = std::make_shared<bd2::DiskManager>("btree1M.dat", false);
    std::shared_ptr<bd2::DiskManager> index = std::make_shared<bd2::DiskManager>("btree1M.index", false);
    bd2::DataBase<Default, int> db = bd2::DataBase<Default, int>(index, data, 1000000);
    Default r;
    srand(time(NULL));
    int idx = rand()%1000000+1;
    db.readRecord(r, idx);
    std::cout << "Idx finded: " << idx << std::endl;
    r.show();
    //db.showTreeIndex();
}

TEST_F(DiskBasedBtree, Insert1k_WithoutIndex) {
    std::shared_ptr<bd2::DiskManager> data = std::make_shared<bd2::DiskManager>("btreew.dat", true);
    std::shared_ptr<bd2::DiskManager> index = std::make_shared<bd2::DiskManager>("btreew.index", true);
    bd2::DataBase<Default, int> db = bd2::DataBase<Default, int>(index, data, 0,-1);
    db.loadFromExternalFile("data1k.bin");
    //db.showTreeIndex();
}

TEST_F(DiskBasedBtree, Insert10k_WithoutIndex) {
    std::shared_ptr<bd2::DiskManager> data = std::make_shared<bd2::DiskManager>("btree10kw.dat", true);
    std::shared_ptr<bd2::DiskManager> index = std::make_shared<bd2::DiskManager>("btree10kw.index", true);
    bd2::DataBase<Default, int> db = bd2::DataBase<Default, int>(index, data, 0, -1);
    db.loadFromExternalFile("data10k.bin");
    //db.showTreeIndex();
}
TEST_F(DiskBasedBtree, Insert100k_WithoutIndex) {
    std::shared_ptr<bd2::DiskManager> data = std::make_shared<bd2::DiskManager>("btree100kw.dat", true);
    std::shared_ptr<bd2::DiskManager> index = std::make_shared<bd2::DiskManager>("btree100kw.index", true);
    bd2::DataBase<Default, int> db = bd2::DataBase<Default, int>(index, data, 0, -1);
    db.loadFromExternalFile("data100k.bin");
    //db.showTreeIndex();
}

TEST_F(DiskBasedBtree, Insert1M_WithoutIndex) {
    std::shared_ptr<bd2::DiskManager> data = std::make_shared<bd2::DiskManager>("btree1Mw.dat", true);
    std::shared_ptr<bd2::DiskManager> index = std::make_shared<bd2::DiskManager>("btree1Mw.index", true);
    bd2::DataBase<Default, int> db = bd2::DataBase<Default, int>(index, data, 0, -1);
    db.loadFromExternalFile("data1M.bin");
    //db.showTreeIndex();
}

TEST_F(DiskBasedBtree, Find1k_WithoutIndex) {
    std::shared_ptr<bd2::DiskManager> data = std::make_shared<bd2::DiskManager>("btreew.dat", false);
    std::shared_ptr<bd2::DiskManager> index = std::make_shared<bd2::DiskManager>("btreew.index", false);
    bd2::DataBase<Default, int> db = bd2::DataBase<Default, int>(index, data, 1000, -1);
    Default r;
    int d;
    db.findWithoutIndex(r,500,d);
    r.show();
}

TEST_F(DiskBasedBtree, Find10k_WithoutIndex) {
    std::shared_ptr<bd2::DiskManager> data = std::make_shared<bd2::DiskManager>("btree10kw.dat", false);
    std::shared_ptr<bd2::DiskManager> index = std::make_shared<bd2::DiskManager>("btree10kw.index", false);
    bd2::DataBase<Default, int> db = bd2::DataBase<Default, int>(index, data, 10000, -1);
    Default r;
    int d;
    db.findWithoutIndex(r,5000,d);
    r.show();
}
TEST_F(DiskBasedBtree, Find100k_WithoutIndex) {
    std::shared_ptr<bd2::DiskManager> data = std::make_shared<bd2::DiskManager>("btree100kw.dat", false);
    std::shared_ptr<bd2::DiskManager> index = std::make_shared<bd2::DiskManager>("btree100kw.index", false);
    bd2::DataBase<Default, int> db = bd2::DataBase<Default, int>(index, data, 100000, -1);
    Default r;
    int d;
    db.findWithoutIndex(r,50000,d);
    r.show();
}

TEST_F(DiskBasedBtree, Find1M_WithoutIndex) {
    std::shared_ptr<bd2::DiskManager> data = std::make_shared<bd2::DiskManager>("btree1M.dat", false);
    std::shared_ptr<bd2::DiskManager> index = std::make_shared<bd2::DiskManager>("btree1M.index", false);
    bd2::DataBase<Default, int> db = bd2::DataBase<Default, int>(index, data, 1000000, -1);
    Default r;
    int d;
    db.findWithoutIndex(r,500000,d);
    r.show();
}

TEST_F(DiskBasedBtree, StaticHashing) {
    std::shared_ptr<bd2::DiskManager> data = std::make_shared<bd2::DiskManager>("static1k.dat", true);
    std::shared_ptr<bd2::DiskManager> index = std::make_shared<bd2::DiskManager>("static1k.index", true);
    bd2::DataBase<Default, int> db = bd2::DataBase<Default, int>(index, data, 0, 1);
    db.loadFromExternalFile("data1k.bin");
    Default r;
    //db.readRecord_SH(r, 500);
}