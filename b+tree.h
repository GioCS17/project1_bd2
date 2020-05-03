
namespace bd2{

template<typename T, int ORDER = 3>
class b+tree{

  using type_pk = typename T::type_pk;

  struct Node{
    long id;
    long size;

    long left_node;
    long right_node;

    bool is_leaf;

    long values[ORDER];
    long* children[ORDER+1];

    Node(const long _id) : id(_id){
      size=0;
      is_leaf=true;
    }
  };
};

}
