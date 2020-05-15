/**
 * @file disk_manager.h
 * @author Juan Vargas Castillo (juan.vargas@utec.edu.pe)
 * @author Giordano Alvitez Falcón (giordano.alvitez.com)
 * @author Roosevelt.Ubaldo Chavez (roosevelt.ubaldo@utec.edu.pe)
 * @brief Disk Manager Implementation, is used to read and write on file streams
 * @version 0.1
 * @date 2020-05-12
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#pragma once
#include <cstdlib>
#include<fstream>
#include<iostream>
#include<string>

namespace bd2{

class DiskManager : protected std::fstream{

  std::string filePath;
  bool empty;

  public:

    /**
     *@brief Default constructor
     */
    DiskManager(){ };

     /**
     * @brief Construct a new Disk Manager object to read and write nodes from disk
     *
     * @param fp filename of the index file
     * @param reset flag to truncate or not the current filename
     */
      DiskManager(std::string fp, bool reset = false):std::fstream(fp.data(),std::ios::in | std::ios::out | std::ios::binary){
          filePath = fp;
          empty=false;
        if(!good() || reset){ //good check if any flag bit without googbit is on
          empty=true;
          open(filePath.data(),
                  std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc );
        }
      }

      ~DiskManager(){ close();} //close the open file

    /**
     * @brief Write a record to a disk file
     *
     * @tparam Record class to be stored
     * @param n position in which the record is going to be stored
     * @param reg record value
     */
      template<typename Record>
      void write_record(const long &n, Record &reg){
        clear(); //reset flags bit (goodbit, eofbit, failbit, badbit)
        seekp(n*sizeof(Record),std::ios::beg);
        write(reinterpret_cast<const char*>(&reg),sizeof(reg));
      }


      /**
       * @brief Write a record to the file's end
       * 
       * @tparam Record 
       * @param reg 
       */
      template<typename Record>
      long write_record_toending(Record &reg){
        clear(); //reset flags bit (goodbit, eofbit, failbit, badbit)
        seekp(0,std::ios::end);
        long pos=tellp();
        write(reinterpret_cast<const char*>(&reg),sizeof(reg));
        return pos;
      }

    /**
     * @brief Read a record from a disk file and returns if was successfully
     *
     * @tparam Record class to be read
     * @param n position in which the record is going to be read
     * @param reg record to save the read value
     * @return true was successfully read
     * @return false an error occurs
     */
      template<typename Record>
      bool retrieve_record(const long &n, Record &reg){
        clear();
        seekg(n*sizeof(Record),std::ios::beg);
        read(reinterpret_cast<char *>(&reg),sizeof(reg));
        return gcount() > 0; //Returns the number of characters extracted by the last unformatted input operation performed on the fstrem .
      }

      /**
       * @brief Function to check is the file is empty or not
       *
       * @return true the file is empty
       * @return false the file has elements
       */
      inline bool is_empty(){ return empty;}
    };
}
