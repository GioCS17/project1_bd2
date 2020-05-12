#pragma once
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
        long disk_id = -1; //id on disk

        void initChildrensWithZeros(){
            for (int i = 0; i < ORDER + 2; i++)
                children[i] = 0;
        }
    public:

        /**
         * @brief Construct a new Node object
          *
         * @param d_id diskManager id of the node on disk
         */
        Node(long d_id){
            disk_id = d_id;
            initChildrensWithZeros();
        };

        /**
         * @brief Construct a new Node object for leaf
         *
         * @param d_id
         * @param is_leaf
         */
        Node(long d_id, bool is_leaf_flag){
            disk_id = d_id;
            is_leaf = is_leaf_flag;
            initChildrensWithZeros();
        };

        /**
         * @brief Function to insert a key_value value in a given position
         *
         * @param key_value
         * @param pos
         */
        void insertKeyInPosition(int pos, const T &key_value){
            //Move to the right until we find the pos of the key_value value
            for(int i = n_keys; i > pos; i--){
                keys[i] = keys[i - 1];
                children[i + 1] = children[i];
            }
            keys[pos] = key_value;
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
}