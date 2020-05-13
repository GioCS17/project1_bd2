/**
 * @file b_plus_tree.h
 * @author Juan Vargas Castillo (juan.vargas@utec.edu.pe)
 * @author Giordano Alvitez Falcón (giordano.alvitez@utec.edu.pe)
 * @author Roosevelt.Ubaldo Chavez (roosevelt.ubaldo@utec.edu.pe)
 * @brief B+Tree Index Implementation based on the starting template
 * for a B-Tree implementation by Alexander Ocsa in ADA 2019-2
 * @version 0.1
 * @date 2020-05-12
 * @copyright Copyright (c) 2020
 * 
 */

#pragma once
#include "disk_manager.h"
#include "b_plus_tree_node.h"
#include "b_plus_tree_iterator.h"
#include <memory>
#include <iostream>
#include <vector>
#include <cmath>

namespace bd2{

/**
 * @brief BPlusTree class
 * 
 * @tparam T type of the key value
 * @tparam 3 default b+tree order 
 */
template<typename T, int ORDER = 3>
class BPlusTree{

  using node = bd2::Node<T,ORDER>;
  using iterator = bd2::BPlusTreeIterator<T,ORDER>;
  using diskManager = std::shared_ptr<DiskManager>;

  enum state { OVERFLOW, NORMAL}; //state of the node insertion

  diskManager disk_manager; // disk manager of the index file

  //struct to store the starting node and the number of existing nodes
  struct Header{
      long disk_id = 1;
      long n_nodes = 0;
  } header;

  protected:

    /**
     * @brief Create a Node object by a isLeaf flag
     *
     * @param isLeaf if the node is a leaf node
     * @return node node created
     */
    node createNode(bool isLeaf){
        header.n_nodes++; //position of the new node
        disk_manager->write_record(0, header);
        node new_node(header.n_nodes, isLeaf);
        return new_node;
    }

    /**
     * @brief Create a Node object by a disk_id and isLeaf flag,
     * Its create a node with the same disk id of a previous one,
     * it is used in the split function to overwrite an splitted
     * node with just the left child values
     *
     * @param disk_id the disk id of the previous node
     * @param isLeaf is the node leaf?
     * @return node node created
     */
    node createNode(long disk_id, bool isLeaf){
        node new_node(disk_id, isLeaf);
        return new_node;
    }


    /**
     * @brief Read a node from disk by a given disk id position
     *
     * @param disk_id
     * @return node
     */
    node readNode(long disk_id){
        node new_node(-1);
        disk_manager->retrieve_record(disk_id, new_node);
        return new_node;
    }

    /**
     * @brief Write a node to disk by a given disk id position
     *
     * @param disk_id
     * @param n
     */
    void writeNode(long disk_id, node n){
        disk_manager->write_record(disk_id, n);
    }

    /**
     * @brief Insert a value to a given node, to do this, it search
     * right position for the value, and insert on it if is a leaf node,
     * else call insert of a value to a child
     *
     * @param ptr_node
     * @param value
     * @return int
     */
    int insert(node &ptr_node, const T &value, const long record_id){
        int pos = 0;

        //find position on node
        while (pos < ptr_node.n_keys && ptr_node.keys[pos] < value)
            pos++;

        if (ptr_node.is_leaf){
            ptr_node.insertKeyInPosition(pos, value, record_id);       //insert if is a leaf node
            writeNode(ptr_node.disk_id, ptr_node);
        } else {        //search for the child node to insert
            long page_id = ptr_node.children[pos];
            node child = readNode(page_id);
            int state = insert(child, value, record_id);
            if (state == OVERFLOW){
                splitNode(ptr_node, pos);
            }
        }
        return ptr_node.isOverflow() ? OVERFLOW : NORMAL; //the insertion status
    }

    /**
     * @brief split the root node in left and write child, by a left based split
     *
     */
    void splitRoot() {
        node ptr_node = readNode(header.disk_id);
        node left_node = createNode(ptr_node.is_leaf);
        node right_node = createNode(ptr_node.is_leaf);

        int iter_child; //for child
        int iter_keys = 0;//for keys
        int pos = 0;

        for (iter_child = 0; iter_keys < ORDER / 2; iter_child++) { //copy to left node
            left_node.children[iter_child] = ptr_node.children[iter_keys];
            left_node.keys[iter_child] = ptr_node.keys[iter_keys];
            left_node.records_id[iter_child] = ptr_node.records_id[iter_keys];
            left_node.n_keys++;
            iter_keys++;
        }
        left_node.children[iter_child] = ptr_node.children[iter_keys];

        if (ptr_node.is_leaf){ //if is the first split of the root node
            left_node.keys[iter_child] = ptr_node.keys[iter_keys]; //left base split
            left_node.records_id[iter_child] = left_node.records_id[iter_keys];
            left_node.n_keys++;
        }

        iter_keys++;

        for (iter_child = 0; iter_keys < ORDER + 1; iter_child++) {  //copy to right node
            right_node.children[iter_child] = ptr_node.children[iter_keys];
            right_node.keys[iter_child] = ptr_node.keys[iter_keys];
            right_node.records_id[iter_child] = right_node.records_id[iter_keys];
            right_node.n_keys++;
            iter_keys++;
        }

        right_node.children[iter_child] = ptr_node.children[iter_keys];

        if (ptr_node.is_leaf){ //link the children nodes if is the first split
            left_node.next_node = right_node.disk_id;
            right_node.prev_node = left_node.disk_id;
        }


        //update children array
        ptr_node.children[pos] = left_node.disk_id;

        ptr_node.keys[0] = ptr_node.keys[ORDER / 2];
        ptr_node.records_id[0] = ptr_node.records_id[ORDER/2];
        ptr_node.children[pos + 1] = right_node.disk_id;
        ptr_node.n_keys = 1;
        ptr_node.is_leaf = false;

        writeNode(ptr_node.disk_id, ptr_node);
        writeNode(left_node.disk_id, left_node);
        writeNode(right_node.disk_id, right_node);
    }


    /**
     * @brief split a node in left and write child, by a left based split
     *
     * @param parent_node parent of the node to be splitted
     * @param pos position of the node to be splitted in the parent node
     */
    void splitNode (node &parent_node, int pos){
        node ptr_node = readNode(parent_node.children[pos]);
        node left_node = createNode(parent_node.children[pos], ptr_node.is_leaf);
        node right_node = createNode(ptr_node.is_leaf);

        int iter_child;
        int iter_keys = 0;
        for (iter_child = 0; iter_keys < ORDER / 2; iter_child++) {
            left_node.children[iter_child] = ptr_node.children[iter_keys];
            left_node.keys[iter_child] = ptr_node.keys[iter_keys];
            left_node.records_id[iter_child] = ptr_node.records_id[iter_keys];
            left_node.n_keys++;
            iter_keys++;
        }
        left_node.children[iter_child] = ptr_node.children[iter_keys];

        if (ptr_node.is_leaf) { //if the node splitted is leaf
            left_node.keys[iter_child] = ptr_node.keys[iter_keys]; //left based split
            left_node.records_id[iter_child] = ptr_node.records_id[iter_keys];
            left_node.n_keys++;

            //update the next and previous nodes
            left_node.next_node = right_node.disk_id;
            right_node.prev_node = left_node.disk_id;
            right_node.next_node = parent_node.children[pos + 1];

            //update the previous node of the next node before split
            node temp = readNode(right_node.next_node);
            temp.prev_node = right_node.disk_id;
            writeNode(temp.disk_id, temp);
        }
        parent_node.insertKeyInPosition(pos, ptr_node.keys[iter_keys], ptr_node.records_id[iter_keys]); //key promoted

        iter_keys++;

        for (iter_child = 0; iter_keys < ORDER + 1; iter_child++) {
            right_node.children[iter_child] = ptr_node.children[iter_keys];
            right_node.keys[iter_child] = ptr_node.keys[iter_keys];
            right_node.records_id[iter_child] = right_node.records_id[iter_keys];
            right_node.n_keys++;

            iter_keys++;
        }
        right_node.children[iter_child] = ptr_node.children[iter_keys];
        parent_node.children[pos] = left_node.disk_id;
        parent_node.children[pos + 1] = right_node.disk_id;
        parent_node.is_leaf = false; //update the parent to non leaf


        writeNode(parent_node.disk_id, parent_node);
        writeNode(left_node.disk_id, left_node);
        writeNode(right_node.disk_id, right_node);
    }

public:
    /**
     *@brief Default constructor
     * */
    BPlusTree(){};

    /**
     * @brief Construct a new BPlusTree object by a disk manager object
     * 
     * @param d_manager disk manager to write and read access on file
     */
    BPlusTree(diskManager d_manager){
        disk_manager = d_manager;

        if (!disk_manager->is_empty())
            disk_manager->retrieve_record(0, header);
        else{  
            //Init the file with the header info
            node root (header.disk_id, true);
            disk_manager->write_record(root.disk_id, root);
            header.n_nodes++;
            disk_manager->write_record(0, header);
        }
    }
    /**
     * @brief Insert operation of a value, it calls to another insert
     * function to store the value to a specific node and returns is
     * a overflow occurs, if that true calls to split
     *
     * @param value
     */
    void insert(const T &value, const long record_id = -1){
        node root = readNode(header.disk_id);
        int state = insert(root, value, record_id);
        if (state == OVERFLOW) {
            splitRoot();
        }
    }


    /**
     * @brief Print the tree values to the console
     * 
     */
    void showTree() {
        node root = readNode(header.disk_id);
        showTree(root, 0);
        std::cout << "________________________\n";
    }

    /**
     * @brief Print the tree values to the console by a tree level
     * @param ptr_node ndoe to be printed
     * @param tree_level tree level
     */
    void showTree(node &ptr_node, int tree_level) {
        int i;
        for (i = ptr_node.n_keys - 1; i >= 0; i--) {
            if (ptr_node.children[i + 1]) { //right child
                node child = readNode(ptr_node.children[i + 1]);
                showTree(child, tree_level + 1);
            }
            for (int k = 0; k < tree_level; k++) {
                std::cout << "    ";
            }
            std::cout << ptr_node.keys[i] << "\n";
        }
        if (ptr_node.children[i + 1]) {//left child
            node child = readNode(ptr_node.children[i + 1]);
            showTree(child, tree_level + 1);
        }
    }
    /**
    * @brief Print just the leaf node values to the console, it is used just for testing
    *
    * @param out
    */
    void print(std::ostream& out) {
        node root = readNode(header.disk_id);
        print(root, 0, out);
    }

    /**
     * @brief Print just the leaf node values to the console
     * by a given tree level, it is used just for testing
     *
     * @param out
     */
    void print(node &ptr_node, int tree_level, std::ostream& out) {
        int i;
        for (i = 0; i < ptr_node.n_keys; i++) {
            if (ptr_node.children[i]) {
                node child = readNode(ptr_node.children[i]);
                print(child, tree_level + 1, out);
            }
            if (ptr_node.is_leaf)
                out << ptr_node.keys[i];
        }
        if (ptr_node.children[i]) {
            node child = readNode(ptr_node.children[i]);
            print(child, tree_level + 1, out);
        }
    }


    /**
     * @brief Returns an iterator with the first  leaf node
     * 
     * @return iterator 
     */
    iterator begin(){
        node temp = readNode(header.disk_id);
        while (!temp.is_leaf)
            temp = readNode(temp.children[0]);
        iterator my_iter (disk_manager, temp.disk_id);
        return my_iter;
    }

    /**
     * @brief Returns an iterator with the last leaf node
     * 
     * @return iterator 
     */
    iterator end(){
        node temp = readNode(header.disk_id);
        while (!temp.is_leaf){
            temp = readNode(temp.children[temp.n_keys]);
        }
        iterator my_iter (disk_manager, temp.disk_id, temp.n_keys - 1);
        return my_iter;
    }

    /**
     * @brief Create a null iterator to check if we exceed the last or first node
     * 
     * @return iterator 
     */
    iterator null(){
        iterator my_iter (disk_manager, -1);
        return my_iter;
    }

    ~BPlusTree(){
    }

    /**
     * @brief 
     * 
     * @param val 
     */
    long search (const T &val) {
        node root = readNode(header.disk_id);
        long res = search (root, val);
        return res;

        /*if (res == -1)
            std::cout << "Not found\n";
        else
            std::cout << "Found!\n";
        */
    }

    /**
     * @brief 
     * 
     * @param ptr 
     * @param val 
     * @return int 
     */
    long search (node &ptr, const T &val){
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
                long page_record = ptr.records_id[pos];
                return page_record;
            }
        }

    }

    /**
     * @brief 
     * 
     * @param first 
     * @param end 
     * @return std::vector<long> 
     */
    std::vector<long> range_search (const T &first, const T &end){
        node root = readNode(header.disk_id);
        std::vector <long> res;
        range_search (root, first, end, res);
        return res;
    }


    /**
     * @brief 
     * 
     * @param ptr 
     * @param first 
     * @param second 
     * @param res 
     */
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

}
