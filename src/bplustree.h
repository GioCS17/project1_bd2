#include "controldisk.h"
#include <memory>
#include <iostream>
#include <vector>
#include <cmath>

namespace bd2{
template <class T, int ORDER>
class BPlusTree;

template <class T, int ORDER>
class BPlusTreeIterator;

template<typename T, int ORDER>
class Node{

  T keys [ORDER + 1];
  long children [ORDER + 2];
  
  long n_keys = 0;
  bool is_leaf = false;

  long next_node  = -1; //link to the next node if is a leaf
  long prev_node = -1; //link to the previous node if is a leaf
  long page_id = -1; //page id on disk

  void initChildrensWithZeros(){
      for (int i = 0; i < ORDER + 2; i++)
            children[i] = 0;
  }

  public:



    /**
     * @brief Construct a new Node object
      * 
     * @param p_id page id of the node on disk
     */
    Node(long p_id){
        page_id = p_id;
        initChildrensWithZeros();
    };

    /**
     * @brief Construct a new Node object for leaf
     * 
     * @param p_id 
     * @param is_leaf 
     */
    Node(long p_id, bool is_leaf_flag){
        page_id = p_id;
        is_leaf = is_leaf_flag;
        initChildrensWithZeros();
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

    void updateKeyInposition(int pos, const T &key){
        keys[pos] = key;
    }

    /**
     * @brief Check is the node is in overflow
     * 
     * @return true overflow
     * @return false not overflow
     */
    bool isOverflow(){
      return n_keys > ORDER;
    }

    friend class BPlusTree<T, ORDER>;
    friend class BPlusTreeIterator<T,ORDER>;
};

template<typename T, int ORDER = 3>
class BPlusTree{

  using node = bd2::Node<T,ORDER>;
  using iterator = bd2::BPlusTreeIterator<T,ORDER>;
  using page = std::shared_ptr<ControlDisk>;
  enum state { OVERFLOW, NORMAL};

  page control_disk; // control disk of the index file

  struct Header{
      long root_page_id = 1;
      long n_nodes = 0;
  } header;
  public:

    BPlusTree(page c_disk){
        control_disk = c_disk;
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

    node createNode(long p_id, bool isLeaf){
        node new_node(p_id, isLeaf); //tamaño
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
        node root = readNode(header.root_page_id);
        int state = insert(root, value);
        if (state == OVERFLOW) {
            splitRoot();
        }
    }

    int insert(node &ptr, const T &value){
        int pos = 0;
        // Equivalente a un for para encontrar la posición del valor dentro de el root
        while (pos < ptr.n_keys && ptr.keys[pos] < value)
            pos++;

        if (ptr.is_leaf){
            ptr.insertKeyInPosition(pos, value);
            writeNode(ptr.page_id, ptr);
        } else {
            //ya encontre el page_id del root, lo busco en sus hijos
            long page_id = ptr.children[pos];
            node child = readNode(page_id); //leo el hijo
            int state = insert(child, value);
            if (state == OVERFLOW){

                splitLeaf(ptr, pos); //update index

            }
        }
        return ptr.isOverflow() ? OVERFLOW : NORMAL;
    }


    void splitRoot() {
        node ptr = readNode(header.root_page_id);
        node left = createNode(ptr.is_leaf);
        node right = createNode(ptr.is_leaf);

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

        if (ptr.is_leaf){
            left.keys[i] = ptr.keys[iter]; // left based split (save in node)
            left.n_keys++;
        }


        iter++; // the middle element
        for (i = 0; iter < ORDER + 1; i++) {
            right.children[i] = ptr.children[iter];
            right.keys[i] = ptr.keys[iter];
            right.n_keys++;
            iter++;
        }
        right.children[i] = ptr.children[iter];

        if (ptr.is_leaf){
            left.next_node = right.page_id;
            right.prev_node = left.page_id;
        }
        ptr.children[pos] = left.page_id; //first child link to the left node
        ptr.keys[0] = ptr.keys[ORDER / 2];
        ptr.children[pos + 1] = right.page_id;
        ptr.n_keys = 1;
        ptr.is_leaf = false;

        writeNode(ptr.page_id, ptr);
        writeNode(left.page_id, left);
        writeNode(right.page_id, right);
    }

    void splitLeaf (node &parent, int pos){
        node ptr = readNode(parent.children[pos]);
        node left = createNode(parent.children[pos], ptr.is_leaf);
        node right = createNode(ptr.is_leaf);

        int iter = 0;
        int i;
        for (i = 0; iter < ORDER / 2; i++) {
            left.children[i] = ptr.children[iter];
            left.keys[i] = ptr.keys[iter];
            left.n_keys++;
            iter++;
        }
        left.children[i] = ptr.children[iter];

        if (ptr.is_leaf) {
            left.keys[i] = ptr.keys[iter]; //left based split
            left.n_keys++;
            left.next_node = right.page_id;
            right.prev_node = left.page_id;
            right.next_node = parent.children[pos + 1];
            node temp = readNode(right.next_node);
            temp.prev_node = right.page_id;
            writeNode(temp.page_id, temp);
        }
        parent.insertKeyInPosition(pos, ptr.keys[iter]); //key promoted

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
            if (ptr.children[i + 1]) { //right child
                node child = readNode(ptr.children[i + 1]);
                showTree(child, level + 1);
            }

            for (int k = 0; k < level; k++) {
                std::cout << "    ";
            }
            std::cout << ptr.keys[i] << "\n";
        }
        if (ptr.children[i + 1]) {//left child
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
            if (ptr.is_leaf)
                out << ptr.keys[i];
        }
        if (ptr.children[i]) {
            node child = readNode(ptr.children[i]);
            print(child, level + 1, out);
        }
    }

    iterator begin(){
        node temp = readNode(header.root_page_id);
        while (!temp.is_leaf)
            temp = readNode(temp.children[0]);
        iterator my_iter (control_disk, temp.page_id);
        return my_iter;
    }
    iterator last(){
        node temp = readNode(header.root_page_id);
        while (!temp.is_leaf){
            temp = readNode(temp.children[temp.n_keys]);
        }
        iterator my_iter (control_disk, temp.page_id);
        return my_iter;
    }

    iterator end(){
        iterator my_iter (control_disk, -1);
        return my_iter;
    }

    ~BPlusTree(){
    }

    void search (const T &val) {
        node root = readNode(header.root_page_id);
        int res = search (root, val);
        if (res == -1)
            std::cout << "Not found\n";
        else
            std::cout << "Found!\n";

    }

    int search (node &ptr, const T &val){
        int pos = 0;
        while (pos < ptr.n_keys && ptr.keys[pos] < val)
            pos++;

        if (!ptr.is_leaf){
            long page_id = ptr.children [pos];
            node child = readNode (page_id);
            return search (child, val);
        } else {
            if (ptr.keys [pos] != val){
                return -1;
            }else {
                long page_record = ptr.children [pos];
                return page_record;
            }
        }

    }

    std::vector<long> range_search (const T &first, const T &end){
        node root = readNode(header.root_page_id);
        std::vector <long> res;
        range_search (root, first, end, res);
        return res;
    }

    void range_search (node &ptr, const T &first, const T &second, std::vector <long> &res){
        int pos = 0;
        while (pos < ptr.n_keys && ptr.keys[pos] < first)
            pos++;

        if (!ptr.is_leaf){
            long page_id = ptr.children [pos];
            node child = readNode (page_id);
            range_search (child, first, second, res);
        } else {
            if (ptr.keys [pos] >= first){
                std::cout << ptr.keys [pos] << " - ";
                res.push_back (ptr.children [pos]);
            }
            pos++;
            node temp = ptr;
            while (temp.keys [pos] <= second && pos < temp.n_keys){
                std::cout << temp.keys [pos] << " - ";
                res.push_back (temp.children [pos]);
                pos++;
                if (pos == temp.n_keys ){
                    temp = readNode (temp.next_node);
                    pos = 0;
                }
            }
        }
    }

};

template <class T, int ORDER>
class BPlusTreeIterator{
    using node = bd2::Node<T, ORDER>;
    using page = std::shared_ptr<ControlDisk>;
    long node_page_id; //node page id on disk
    int keys_pos; //iterator for keys elements
    page control_disk;

    node readNode(long p_id){
        node new_node(-1);
        control_disk->retrieve_record(p_id, new_node);
        return new_node;
    }


public:
    friend class BPlusTree<T, ORDER>;


    BPlusTreeIterator(page &cd, long npi){
        control_disk = cd;
        node_page_id = npi;
        keys_pos = 0;
    }

    BPlusTreeIterator(page &cd, long npi, int keys_pos){
        control_disk = cd;
        node_page_id = npi;
        keys_pos = keys_pos;
    }

    BPlusTreeIterator(const BPlusTreeIterator & bpti){
        node_page_id = bpti.node_page_id;
        control_disk = bpti.control_disk;
        keys_pos = bpti.keys_pos;
    }

    BPlusTreeIterator& operator++(){
        keys_pos++;
        node temp = readNode(node_page_id);
        if (keys_pos >= temp.n_keys){    //if we reach the end of the keys, go to the next node
            node_page_id = temp.next_node;
            keys_pos = 0;
        }
        return *this;
    }
    BPlusTreeIterator operator++(int){
        BPlusTreeIterator tmp (*this);
        operator++();
        return tmp;
    }

    BPlusTreeIterator& operator--(){
        keys_pos--;
        node temp = readNode(node_page_id);
        node_page_id = temp.prev_node;
        if (keys_pos <= 0){    //if we reach the end of the keys, go to the next node
            node_page_id = temp.prev_node;
            keys_pos = temp.n_keys;
        }
        return *this;
    }

    BPlusTreeIterator operator--(int){
        BPlusTreeIterator tmp (*this);
        operator--();
        return tmp;
    }

    BPlusTreeIterator& operator=(const BPlusTreeIterator& bpti){
        keys_pos = bpti.keys_pos;
        node_page_id = bpti.node_page_id;
        control_disk = bpti.control_disk;
    }

    bool operator==(const BPlusTreeIterator& bpti){
        bool nodes_equal = (node_page_id == bpti.node_page_id);
        bool keys_pos_equal = (keys_pos == bpti.keys_pos);
        return nodes_equal && keys_pos_equal;
    }

    bool operator!=(const BPlusTreeIterator& bpti){
        return !((*this)==bpti);
    }

    T operator*(){
        node temp = readNode(node_page_id);
        return temp.keys[keys_pos];
    }

};

}
