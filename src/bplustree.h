#include "controldisk.h"
#include <memory>
#include <iostream>
#include <vector>

namespace bd2{
template <class T, int ORDER>
class BPlusTree;

template<typename T, int ORDER>
class Node{

  T keys [ORDER + 1];
  long children [ORDER + 2];
  
  long n_keys = 0;
  bool is_leaf = false;

  long next  = -1; //link to the next node if is a leaf
  long prev = -1; //link to the previous node if is a leaf
  long page_id = -1; //page id on disk
  
  public:

    void initChildrensWithZeros(){
        for (int i = 0; i < ORDER + 2; i++)
            children[i] = 0;
    }

    Node();
    /**
     * @brief Construct a new Node object
      * 
     * @param p_id page id of the node on disk
     */
    Node(long p_id){
        this->page_id = p_id;
        this->initChildrensWithZeros();
    };

    /**
     * @brief Construct a new Node object for leaf
     * 
     * @param p_id 
     * @param is_leaf 
     */
    Node(long p_id, bool is_leaf_flag){
        this->page_id = p_id;
        this->is_leaf = is_leaf_flag;
        this->initChildrensWithZeros();
    };

    /**
     * @brief Function to insert a key value in a given position
     * 
     * @param key_value 
     * @param pos 
     */
    void insertKeyInPosition(int pos, const T &key){
      //Move to the right until we find the pos of the key value
      for(int i = n_keys; i > pos; i--){
        keys[i] = keys[i - 1];
        children[i + 1] = children[i];
      }
      keys[pos] = key;
      children[pos + 1] = children [pos];
      n_keys += 1;
    };

    /**
     * @brief Check is the node is in overflow
     * 
     * @return true overflow
     * @return false not overflow
     */
    bool isOverflow(){
      return n_keys > ORDER;
    }

    /**
     * @brief Set the next node object
     * 
     * @param next_page_id 
     */
    void setNextNode(long next_page_id){
      next = next_page_id;
    }

    void setPrevNode(long prev_page_id){
        prev = prev_page_id;
    }

    friend class BPlusTree<T, ORDER>;
};

template<typename T, int ORDER = 3>
class BPlusTree{

  using node = bd2::Node<T,ORDER>;
  using page = std::shared_ptr<ControlDisk>;
  enum state { OVERFLOW, UNDERFLOW, NORMAL};

  page control_disk; // control disk of the index file

  struct Header{
      long root_page_id = 1;
      long n_nodes = 0;
  } header;
  public:

    BPlusTree(page c_disk){
        this->control_disk = c_disk;
        if (!control_disk->is_empty())
            control_disk->retrieve_record(0, header);
        else{  //Init the control disk
            node root (header.root_page_id, true);
            control_disk->write_record(root.page_id, root);
            header.n_nodes++;
            control_disk->write_record(0, header);
        }
    }

    node createNode(bool isLeaf){
        header.n_nodes++;
        control_disk->write_record(0, header);
        node new_node(header.n_nodes, isLeaf); //tamaño
        return new_node;
    }

    node readNode(long p_id){
        node new_node(-1);
        control_disk->retrieve_record(p_id, new_node);
        return new_node;
    }

    void writeNode(long p_id, node n){
        control_disk->write_record(p_id, n);
    }

    void insert(const T &value){
        node root = this->readNode(header.root_page_id);
        int state = this->insert(root, value);
        if (state == OVERFLOW) {
            this->splitRoot();
        }
    }

    int insert(node &ptr, const T &value){
        int pos = 0;
        // Equivalente a un for para encontrar la posición del valor dentro de el root
        while (pos < ptr.n_keys && ptr.keys[pos] < value)
            pos++;

        if (!ptr.is_leaf){
            //ya encontre el page_id del root, lo busco en sus hijos
            long page_id = ptr.children[pos];
            node child = readNode(page_id); //leo el hijo
            int state = insert(child, value);
            if (state == OVERFLOW){
                splitLeaf(ptr, pos);
            }
        } else {
            ptr.insertKeyInPosition(pos, value);
            writeNode(ptr.page_id, ptr);
        }
        return ptr.isOverflow() ? OVERFLOW : NORMAL;
    }


    void splitRoot() {
        node ptr = readNode(header.root_page_id);
        node left = createNode(true);
        node right = createNode(true);
        if (header.n_nodes > 4){ //verificar condicion
            left = createNode(false);
            right = createNode(false);
        }

        int i; //for child
        int iter = 0;//for keys
        int pos = 0;

        for (i = 0; iter < ORDER / 2; i++) {
            left.children[i] = ptr.children[iter];
            left.keys[i] = ptr.keys[iter];
            left.n_keys++;
            iter++;
        }
        left.children[i] = ptr.children[iter];

        /*
        left.keys[i] = ptr.keys[iter]; // left based split
        left.n_keys++;
        */
        iter++; // the middle element
        for (i = 0; iter < ORDER + 1; i++) {
            right.children[i] = ptr.children[iter];
            right.keys[i] = ptr.keys[iter];
            right.n_keys++;
            iter++;
        }
        right.children[i] = ptr.children[iter];

        ptr.children[pos] = left.page_id; //first child link to the left node
        ptr.keys[0] = ptr.keys[ORDER / 2];
        ptr.children[pos + 1] = right.page_id;
        ptr.n_keys = 1;
        ptr.is_leaf = false;

        writeNode(ptr.page_id, ptr);
        writeNode(left.page_id, left);
        writeNode(right.page_id, right);
    }

    void splitNode(node &parent, int pos) {
        node ptr = readNode(parent.children[pos]);
        node left = createNode(false);
        node right = createNode(false);

        int iter = 0;
        int i;

        for (i = 0; iter < ORDER / 2; i++) {
            left.children[i] = ptr.children[iter];
            left.keys[i] = ptr.keys[iter];
            left.n_keys++;
            iter++;
        }
        left.children[i] = ptr.children[iter];

        /*
        left.keys[i] = ptr.keys[iter]; //left based split
        left.n_keys++;
        */

        parent.insertKeyInPosition(pos, ptr.keys[iter]); //key promovida

        iter++; // the middle element

        for (i = 0; iter < ORDER + 1; i++) {
            right.children[i] = ptr.children[iter];
            right.keys[i] = ptr.keys[iter];
            right.n_keys++;

            iter++;
        }
        right.children[i] = ptr.children[iter];

        parent.children[pos] = left.page_id;
        parent.children[pos + 1] = right.page_id;
        parent.is_leaf = false;

        writeNode(parent.page_id, parent);
        writeNode(left.page_id, left);
        writeNode(right.page_id, right);
    }

    void splitLeaf (node &parent, int pos){
        node ptr = readNode(parent.children[pos]);
        node left = createNode(true);
        node right = createNode(true);

        int iter = 0;
        int i;

        for (i = 0; iter < ORDER / 2; i++) {
            left.children[i] = ptr.children[iter];
            left.keys[i] = ptr.keys[iter];
            left.n_keys++;
            iter++;
        }
        left.children[i] = ptr.children[iter];

        /*
        left.keys[i] = ptr.keys[iter]; //left based split
        left.n_keys++;
        */

        parent.insertKeyInPosition(pos, ptr.keys[iter]); //key promovida

        iter++; // the middle element

        for (i = 0; iter < ORDER + 1; i++) {
            right.children[i] = ptr.children[iter];
            right.keys[i] = ptr.keys[iter];
            right.n_keys++;

            iter++;
        }
        right.children[i] = ptr.children[iter];

        parent.children[pos] = left.page_id;
        parent.children[pos + 1] = right.page_id;
        parent.is_leaf = false;

        writeNode(parent.page_id, parent);
        writeNode(left.page_id, left);
        writeNode(right.page_id, right);
    }

    void showTree() {
        node root = readNode(header.root_page_id);
        showTree(root, 0);
        std::cout << "________________________\n";
    }

    void showTree(node &ptr, int level) {
        int i;
        for (i = ptr.n_keys - 1; i >= 0; i--) {
            if (ptr.children[i + 1]) {
                node child = readNode(ptr.children[i + 1]);
                showTree(child, level + 1);
            }

            for (int k = 0; k < level; k++) {
                std::cout << "    ";
            }
            std::cout << ptr.keys[i] << "\n";
        }
        if (ptr.children[i + 1]) {
            node child = readNode(ptr.children[i + 1]);
            showTree(child, level + 1);
        }
    }

    void print(std::ostream& out) {
        node root = readNode(header.root_page_id);
        print(root, 0, out);
    }

    void print(node &ptr, int level, std::ostream& out) {
        int i;
        for (i = 0; i < ptr.n_keys; i++) {
            if (ptr.children[i]) {
                node child = readNode(ptr.children[i]);
                print(child, level + 1, out);
            }
            out << ptr.keys[i];
        }
        if (ptr.children[i]) {
            node child = readNode(ptr.children[i]);
            print(child, level + 1, out);
        }
    }


    ~BPlusTree(){
    }
  
};

}
