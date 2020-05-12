#pragma once
#include <cstdlib>
#include<fstream>
#include<iostream>
#include<string>

namespace bd2{

class ControlDisk : protected std::fstream{

  std::string filePath;
  bool empty;

  public:

  ControlDisk(std::string fp, bool reset = false)
    : std::fstream(fp.data(),
            std::ios::in | std::ios::out | std::ios::binary){
      filePath = fp;
      empty=false;
    if(!good() || reset){ //good check if any flag bit without googbit is on
      empty=true;
      open(fp.data(),
              std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc );

    }
  }

  ~ControlDisk(){ close();} //close de open file


  template<typename Record>
  void write_record(const long &n, Record &reg){
    clear(); //reset flags bit (goodbit, eofbit, failbit, badbit)
    seekp(n*sizeof(Record),std::ios::beg);
    write(reinterpret_cast<const char*>(&reg),sizeof(reg));
  }

  template<typename Record>
  bool retrieve_record(const long &n, Record &reg){
    clear();
    seekg(n*sizeof(Record),std::ios::beg);
    read(reinterpret_cast<char *>(&reg),sizeof(reg));
    return gcount()>0; //Returns the number of characters extracted by the last unformatted input operation performed on the fstrem .
  }

  inline bool is_empty(){ return empty;}


};

}
