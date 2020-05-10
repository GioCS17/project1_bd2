#include "controldisk.h"
#include <memory>
#include <iostream>
#include <vector>

namespace bd2{
    
template <class T, int ORDER>
class BPlusTree;

template <class T, int ORDER>
class BPlusTreeIterator;
    
template<typename T, int ORDER>
class Node{

  T keys [ORDER - 1];
  long children [ORDER];
  
  long n_keys = 0;
  bool is_leaf = false;

  long next  = -1; //link to the next node if is a leaf
  long prev = -1; //link to the previous node if is a leaf
  long page_id = -1; //page id on disk
  
  public:
    /**
     * @brief Construct a new Node object
      * 
     * @param p_id page id of the node on disk
     */
    Node(long p_id): page_id (p_id) {
      //Initialize all children pointers to -1
      for (int i = 0; i < ORDER; i++)
        children[i] = -1;
    };

    /**
     * @brief Construct a new Node object for leaf
     * 
     * @param p_id 
     * @param is_leaf 
     */
    Node(long p_id, bool is_leaf): page_id (p_id), is_leaf(is_leaf){
      //Initialize all children pointers to -1
      for (int i = 0; i < ORDER; i++)
        children[i] = -1;
    };

    /**
     * @brief Function to insert a key value in a given position
     * 
     * @param key_value 
     * @param pos 
     */
    void insertKeyInPosition(T key_value, int pos){
      //Move to the right until we find the pos of the key value
      for(int i = n_keys; i > pos; i--){
        keys[i] = keys[i - 1];
        children[i + 1] = children[i];
      }
      keys[pos] = key_value;
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
  using type_id = long;
  using node = bd2::Node<T,ORDER>;
  using page = std::shared_ptr<ControlDisk>;
  using iterator = bd2::BPlusTreeIterator<T, ORDER>;
  enum state { OVERFLOW, UNDERFLOW, NORMAL};

  page ctrl_disk;
  node* root;


  struct Root{
      long root_page_id = 1;
      long n_nodes = 0;
  } header;

  public:

    BPlusTree(page c_disk):ctrl_disk{c_disk}{
        if (!ctrl_disk->is_empty()){
            ctrl_disk->retrieve_record(header.root_page_id, header);
        }else{  //Init the control disk
            node root (header.root_page_id, true);
            ctrl_disk->write_record(root.page_id, root);
            header.n_nodes++;
            ctrl_disk->write_record(0, header);
        }
    }

    node createNode(){
        header.n_nodes++;
        node n(header.n_nodes); //tamaño
        ctrl_disk->write_record(0, header);
        return n;
    }

    node readNode(long p_id){
        node n(-1);
        ctrl_disk->retrieve_record(p_id, n);
        return n;
    }

    void writeNode(long p_id, node n){
        ctrl_disk->write_record(p_id, n);
    }

    void insert(T &value){
        node root = this->readNode(header.root_page_id);
        int state = this->insert(root, value);
        if (state == OVERFLOW) {
            this->splitRoot();
        }
    }
    
    int insert(node &ptr, T value){
        int pos = 0;
        // Equivalente a un for para encontrar la posición del valor dentro de el root
        while (pos < ptr.n_keys && ptr.keys[pos] < value)
            pos++;

        if (ptr.is_leaf){
            ptr.insertKeyInPosition(pos, value);
            this->writeNode(ptr.page_id, ptr);
        } else {
            //ya encontre el page_id del root, lo busco en sus hijos
            long page_id = ptr.children[pos];
            node child = readNode(page_id); //leo el hijo
            int state = insert(child, value);
            if (state == OVERFLOW){
                if (child.is_leaf){
                    this->splitLeaf(ptr, pos);
                }else{
                    this->splitNode(ptr, pos);
                }
            }
        }
        return ptr.isOverflow() ? OVERFLOW : NORMAL;
    }

    void splitRoot() {
        node ptr = this->readNode(this->header.root_page_id);
        node left = this->createNode();
        node right = this->createNode();

        int i = 0;
        int iter = 0;

        int mid = ptr.n_keys/2;
        int mid_key = ptr.keys[mid];

        if (ptr.is_leaf) {
            ptr.is_leaf = false;
            left.is_leaf = true;
            right.is_leaf = true;

            for (i = 0; iter < mid; i++) {
                left.keys[i] = ptr.keys[iter];
                left.children[i] = ptr.children[iter];
                left.n_keys++;

                ptr.keys[iter] = ptr.children[iter] = 0;
                iter++;
            }

            for (i = 0; iter < ORDER + 1; i++) {
                right.keys[i] = ptr.keys[iter];
                right.children[i] = ptr.children[iter];
                right.n_keys++;

                ptr.keys[iter] = ptr.children[iter] = 0;
                iter++;
            }

            left.next = right.page_id;
        } else {
            for (i = 0; iter < mid; i++) {
                left.keys[i] = ptr.keys[iter];
                left.children[i] = ptr.children[iter];
                left.n_keys++;

                ptr.keys[iter] = ptr.children[iter] = 0;
                iter++;
            }
            left.children[i] = ptr.children[iter];
            ptr.children[iter] = 0;
            iter++;

            for (i = 0; iter < ORDER + 1; i++) {
                right.keys[i] = ptr.keys[iter];
                right.children[i] = ptr.children[iter];
                right.n_keys++;

                ptr.keys[iter] = ptr.children[iter] = 0;
                iter++;
            }
            right.children[i] = ptr.children[iter];
            ptr.children[iter] = 0;
        }

        ptr.children[0] = left.page_id;
        ptr.children[1] = right.page_id;
        ptr.keys[0] = mid_key;
        ptr.n_keys = 1;

        this->writeNode(ptr.page_id, ptr);
        this->writeNode(left.page_id, left);
        this->writeNode(right.page_id, right);
    }

    void splitNode(node &parent, int pos) {
        node left = this->readNode(parent.children[pos]);
        node right = this->createNode();

        int iter = ORDER / 2 + 1;
        int middle = ORDER / 2;
        int i;

        left.keys[middle] = 0;
        for (i = 0; iter < ORDER + 1; i++) {
            right.children[i] = left.children[iter];
            right.keys[i] = left.keys[iter];
            right.n_keys++;

            left.keys[iter] = 0;
            left.children[iter] = 0;
            iter++;
        }
        right.children[i] = left.children[iter];
        left.children[iter] = 0;
        left.n_keys = middle; //actualizo left

        parent.insertKeyInPosition(pos, left.keys[middle]);
        parent.children[pos] = left.page_id;
        parent.children[pos+1] = right.page_id;

        this->writeNode(parent.page_id, parent);
        this->writeNode(left.page_id, left);
        this->writeNode(right.page_id, right);
    }

    void splitLeaf (node &parent, int pos){
        node left = readNode(parent.children[pos]);
        node right = createNode();
        right.is_leaf = true;

        int iter = ORDER/2 + 1;
        int i;

        for (i = 0; iter < ORDER + 1; i++) {
            right.children[i] = left.children[iter];
            right.keys[i] = left.keys[iter];
            right.n_keys++;
            left.children[iter] = left.keys[iter] = 0;
            //limpiando
            iter++;
        }
        left.n_keys = ORDER / 2;
        right.next = left.next;
        left.next = right.page_id;

        parent.insertKeyInPosition(pos, left.keys[ORDER / 2]);
        parent.children[pos] = left.page_id;
        parent.children[pos+1] = right.page_id;

        this->writeNode(parent.page_id, parent);
        this->writeNode(left.page_id, left);
        this->writeNode(right.page_id, right);
    }

    iterator find(const T &value) {
        node ptr = this->readNode(header.root_page_id);
        int pos;
        while (!ptr.is_leaf){
            pos = 0;
            while (pos < ptr.n_keys && ptr.keys[pos] < value){
                pos++;
            }
            ptr = this->readNode(ptr.children[pos]);
        }
        iterator result (this->pm, ptr.page_id);

        return result;
    }
    

    void showTree() {
        node root = readNode(header.root_page_id);
        showTree(root, 0);
        std::cout << "________________________\n";
    }



    void showTree(node &ptr, int level) {
        int i;
        for (i = ptr.n_keys - 1; i >= 0; i--) {
            if (!ptr.is_leaf && ptr.children[i + 1]) {
                node child = readNode(ptr.children[i + 1]);
                showTree(child, level + 1);
            }

            for (int k = 0; k < level; k++) {
                std::cout << "    ";
            }
            std::cout << ptr.keys[i] << "\n";
        }
        if (!ptr.is_leaf && ptr.children[i + 1]) {
            node child = readNode(ptr.children[i + 1]);
            showTree(child, level + 1);
        }
    }

    iterator begin(){
        node current = this->readNode(header.root_page_id);
        while (!current.is_leaf) {
            current = this->readNode(current.children[0]);
        }
        iterator result (this->pm, current.page_id);
        return result;

    }

    iterator end(){
        iterator result (this->pm, -1);
        return result;
    }


    ~BPlusTree(){
    }
  
};

    template <class T, int ORDER = 3>
    class btreeIter{
    private:
        int pos;
        long node_id;
        using page = std::shared_ptr<ControlDisk>;
        page pm_it;
    public:
        friend class BPlusTree<T, ORDER>;
        using node = bd2::Node<T, ORDER>;

        btreeIter(page &pm, long node_id){
            this->pm_it = pm;
            this->node_id = node_id;
            this->pos = 0;
        }
        btreeIter(page &pm, long node_id, int pos){
            this->pm_it = pm;
            this->node_id = node_id;
            this->pos = pos;
        }
        btreeIter(page &pm, const btreeIter& other){
            this->pm_it = other.pm_it;
            this->node_id = other.node_id;
            this->pos = other.pos;
        }

        node readNode (long page_id){
            node nd (-1);
            (page_id == -1)? nd.next = -1: pm_it->retrieve_record(page_id, nd);
            return nd;
        }

        btreeIter& operator=(btreeIter other){
            this->pos = other.pos;
            this->node_id = other.node_id;
            return *this;
        }
        bool operator==(const btreeIter& other) {
            return (this->node_id == other.node_id) && (this->pos == other.pos);
        }
        bool operator!=(const btreeIter& other) {
            return !((*this) == other);
        }


        btreeIter& operator++(){ // postfix ++
            node temp = readNode(this->node_id);
            this->pos++;
            if (temp.n_keys <= this->pos){
                this->node_id = temp.next;
                this->pos = 0;
            }
            return *this;
        }

        btreeIter& operator++(int){ // prefix ++
            btreeIter result (pm_it, *this);
            ++(*this);
            return result;
        }

        // dereference operator
        T operator*() {
            node temp = readNode(this->node_id);
            return temp.keys[pos];
        }

    };

}
