#include <gtest/gtest.h>
#include <b_plus_tree.h>
#include <disk_manager.h>
#include <data_base.h>
#include <vector>

#define PAGE_SIZE  64
#define BTREE_ORDER   ((PAGE_SIZE - (2 * sizeof(long) + sizeof(int) +  2 * sizeof(long)) ) /  (sizeof(int) + sizeof(long)))


struct DiskBasedBtree : public ::testing::Test
{
};


TEST_F(DiskBasedBtree, IndexingRandomElements) {
  bool trunc_file = true;
  std::shared_ptr<bd2::DiskManager> pm = std::make_shared<bd2::DiskManager>("btree.index", trunc_file);
  std::cout << "PAGE_SIZE: " << PAGE_SIZE << std::endl;
  std::cout << "BTREE_ORDER: " << BTREE_ORDER << std::endl;
  bd2::BPlusTree<char, BTREE_ORDER> bt(pm);

  std::string values = "zxcnmvfjda";
  for(auto c : values) {
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
    std::string values = "be1986432";
    for(auto c : values) {
        bt.insert(c);
        bt.showTree();
    }
    std::ostringstream out;
    bt.print(out);
    std::string all_values = "zxcnmvfjdabe1986432";
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
  std::string all_values = "zxcnmvfjdabe1986432";
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
    std::string all_values = "zxcnmvfjdabe1986432";
    std::sort(all_values.begin(), all_values.end(), std::greater <>());
    EXPECT_EQ(all_values, iter_values);
}



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
    db.insertWithBPlusTreeIndex(p,p.id);
    Student p1 {20, true,"Luis","Fujimori",10};
    db.insertWithBPlusTreeIndex(p1, p1.id);
    Student p2 {30, true,"Martin","Lapierre",120};
    db.insertWithBPlusTreeIndex(p2, p2.id);
    Student p3 {40, false,"Heider","McDonalds",110};
    db.insertWithBPlusTreeIndex(p3, p3.id);
    Student p4 {50, true,"Juan","Schmitchel",122};
    db.insertWithBPlusTreeIndex(p4, p4.id);
    Student p5 {60, false,"Peter","Schwaun",101};
    db.insertWithBPlusTreeIndex(p5, p5.id);
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


TEST_F(DiskBasedBtree, InsertFromCSV){
    int n = 10;
    struct Default{
        long id;
        std::vector<std::string> args;
        void show(){
            std::cout << "pk: " << id << std::endl;
            for (int i = 0; i < args.size(); i++){
                std::cout << "args " << i + 1 << ": " << args[i] << std::endl;
            }
            std::cout << "--------------------------------" << std::endl;
        }
    };
    bd2::DataBase<Default, long> db = bd2::DataBase<Default, long>();
    db.loadFromExternalFile("input.txt",1 , false);
    Default d;
    db.readRecord(d, 100);
    d.show();
    EXPECT_EQ(d.id, 100);
}
