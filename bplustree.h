#include "controldisk.h"
#include <memory>
#include <iostream>
#include <vector>

namespace bd2{

template<typename T, int ORDER = 3>
class bplustree{

  using type_pk = typename T::type_pk;
  using type_id = long;

  struct Node{
    type_id id;
    long size;

    type_id left_node;
    type_id right_node;

    bool is_leaf;

    type_id values[ORDER];
    Node* children[ORDER+1];

    Node(const type_id _id) : id{_id}{
      size=0;
      is_leaf=true;
      left_node=right_node=-1;
    }
  };

  enum state { OVERFLOW, UNDERFLOW, NORMAL};

  std::shared_ptr<controldisk> ctrl_disk;

  Node* root;

  public:
    bplustree(std::shared_ptr<controldisk> cdisk):ctrl_disk{cdisk}{
    }
    ~bplustree(){
    }
  
};

}
