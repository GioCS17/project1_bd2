/**
 * @file b_plus_tree_iterator.h
 * @author Juan Vargas Castillo (juan.vargas@utec.edu.pe)
 * @author Giordano Alvitez Falcón (giordano.alvitez@utec.edu.pe)
 * @author Roosevelt.Ubaldo Chavez (roosevelt.ubaldo@utec.edu.pe)
 * @brief B+Tree Iterators Implementation, operators ++, --, dereference * were implented
 * @version 0.1
 * @date 2020-05-12
 * @copyright Copyright (c) 2020
 * 
 */
#include "disk_manager.h"
#include <memory>
#include <iostream>
#include <vector>
#include <cmath>
#pragma once

namespace bd2{
    template <class T, int ORDER>
    class BPlusTree;

    /**
     * @brief B Plus Tree Iterator Object
     * 
     * @tparam T type of the index
     * @tparam ORDER order of the btree
     */
    template <class T, int ORDER>
    class BPlusTreeIterator{

        using node = bd2::Node<T, ORDER>;
        using diskManager = std::shared_ptr<DiskManager>;
        
        long node_disk_id; //node id on disk
        int keys_pos; //iterator for keys elements
        
        diskManager disk_manager;

        /**
         * @brief Read a node from disk by a given disk id position
         * 
         * @param disk_id position on disk to be read
         * @return node node with the read values 
         */
        node readNode(long disk_id){
            node new_node(-1);
            disk_manager->retrieve_record(disk_id, new_node);
            return new_node;
        }

    public:

        friend class BPlusTree<T, ORDER>;

        /**
         * @brief Construct a new BPlusTreeIterator object
         * 
         * @param manager disk manager of the btree
         * @param ndi disk id of the node
         */
        BPlusTreeIterator(diskManager &manager, long ndi){
            disk_manager = manager;
            node_disk_id = ndi;
            keys_pos = 0;
        }

        /**
         * @brief Construct a new BPlusTreeIterator object
         * 
         * @param manager disk manager of the btree
         * @param ndi disk id of the node
         * @param _keys_pos position on key to start the iterator
         */
        BPlusTreeIterator(diskManager &manager, long ndi, int _keys_pos){
            disk_manager = manager;
            node_disk_id = ndi;
            keys_pos = _keys_pos;
        }

        /**
         * @brief Construct a new BPlusTreeIterator object by and other iterator
         * 
         * @param bpti other B+Tree iterator
         */
        BPlusTreeIterator(const BPlusTreeIterator & bpti){
            node_disk_id = bpti.node_disk_id;
            disk_manager = bpti.disk_manager;
            keys_pos = bpti.keys_pos;
        }

        /**
         * @brief Prefix ++ operator
         * Increase in one the value of keys_pos, if we reach the end of a 
         * node, we go to the next node
         * 
         * @return BPlusTreeIterator& 
         */
        BPlusTreeIterator& operator++(){
            keys_pos++;
            node temp = readNode(node_disk_id);
            if (keys_pos >= temp.n_keys){    //if we reach the end of the keys, go to the next node
                node_disk_id = temp.next_node;
                keys_pos = 0;
            }
            return *this;
        }

        /**
         * @brief Postfix ++ operator
         * Increase in one the value of keys_pos, if we reach the end of a 
         * node, we go to the next node
         * 
         * @return BPlusTreeIterator& 
         */
        BPlusTreeIterator operator++(int){
            BPlusTreeIterator tmp (*this);
            operator++();
            return tmp;
        }

        /**
         * @brief Prefix -- operator
         * Decrease in one the value of keys_pos, if we reach the start -1 position 
         * of a node, we go to the previous node
         * 
         * @return BPlusTreeIterator& 
         */
        BPlusTreeIterator& operator--(){
            keys_pos--;
            node temp = readNode(node_disk_id);
            if (keys_pos < 0){    //if we reach the end of the keys, go to the next node
                node_disk_id = temp.prev_node;
                if (node_disk_id != -1){
                    node prev = readNode(node_disk_id);
                    keys_pos = prev.n_keys -1;
                }else{
                    keys_pos = 0;
                }

            }
            return *this;
        }

        /**
         * @brief Postfix -- operator
         * Decrease in one the value of keys_pos, if we reach the start -1 position 
         * of a node, we go to the previous node
         * 
         * @return BPlusTreeIterator& 
         */
        BPlusTreeIterator operator--(int){
            BPlusTreeIterator tmp (*this);
            operator--();
            return tmp;
        }

        /**
         * @brief Assing the value of one iterator to another
         * 
         * @param bpti 
         * @return BPlusTreeIterator& 
         */
        BPlusTreeIterator& operator=(const BPlusTreeIterator& bpti){
            keys_pos = bpti.keys_pos;
            node_disk_id = bpti.node_disk_id;
            disk_manager = bpti.disk_manager;
        }

        /**
         * @brief Check if the two iterators are equal
         * 
         * @param bpti another iterator
         * @return true are equal
         * @return false are different
         */
        bool operator==(const BPlusTreeIterator& bpti){
            bool nodes_equal = (node_disk_id == bpti.node_disk_id);
            bool keys_pos_equal = (keys_pos == bpti.keys_pos);
            return nodes_equal && keys_pos_equal;
        }

        /**
         * @brief Check is the two iterators are different
         * 
         * @param bpti another iterator
         * @return true are different
         * @return false are equal
         */
        bool operator!=(const BPlusTreeIterator& bpti){
            return !((*this)==bpti);
        }

        /**
         * @brief Dereference operator, return the value of the key in the 
         * current keys_pos position
         * 
         * @return T key value
         */
        T operator*(){
            node temp = readNode(node_disk_id);
            return temp.keys[keys_pos];
        }

    };
}