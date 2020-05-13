#include <gtest/gtest.h>
#include <b_plus_tree.h>
#include <disk_manager.h>
#include <data_base.h>

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

TEST_F(DiskBasedBtree, InsertNumbers) {
    std::shared_ptr<bd2::DiskManager> pm = std::make_shared<bd2::DiskManager>("btreen.index");
    using char_btree = bd2::BPlusTree<int, BTREE_ORDER>;
    using char_btree_iterator = bd2::BPlusTreeIterator<int, BTREE_ORDER>;
    char_btree bt(pm);
    int number[10] = {9,8,7,6,5,4,3,2,1,0};
    for (int i = 0; i < 10; i++){
        bt.insert(number[i], i);
        bt.showTree();
    }
    bt.showTree();
    char_btree_iterator iter = bt.end();
    int numbersa[10];
    int pi = 9;
    for(; iter != bt.null(); iter--) {
        numbersa[pi] = *iter;
        std::cout << *iter << ", ";
        pi--;
    }
    std::cout << std::endl;

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

        }
    };
    bd2::DataBase<Student, long> db = bd2::DataBase<Student, long> ();


    Student p {10, false,"alex","orihuela",150};
    //p.show();
    db.insertWithBPlusTreeIndex(p,p.id);
    Student p1 {20, true,"Luis","sanchez",100};
    //p1.show();
    db.insertWithBPlusTreeIndex(p1, p1.id);
    Student p2 =  db.readRecord(10);
    p2.show();

}