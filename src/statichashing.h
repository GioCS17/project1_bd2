/**
 * @file statishashing.h
 * @author Juan Vargas Castillo (juan.vargas@utec.edu.pe)
 * @author Giordano Alvitez Falcón (giordano.alvitez@utec.edu.pe)
 * @author Roosevelt.Ubaldo Chavez (roosevelt.ubaldo@utec.edu.pe)
 * @version 0.1
 * @date 2020-05-12
 * @copyright Copyright (c) 2020
 * 
 */

 #pragma once
#include "disk_manager.h"
#include<memory>
#include<queue>
#include<vector>


namespace bd2{

  template<int fd>
  class Bucket_S{
    public:
      int size;
      long address[fd];
      long NextBucket;
      Bucket_S(){
        NextBucket=-1;
      }
  };

  template<typename T,int gd,int fd>
  class StaticHashing{

    using page = std::shared_ptr<DiskManager>; 
    using value_key = T;
    using Bucket = Bucket_S<fd>;

    page control_bucket;
    page control_data;

    public:
    StaticHashing(){
    }

    StaticHashing(page c_bucket, page c_data){

      control_bucket = c_bucket;
      control_data = c_data;

    }
    ~StaticHashing(){
    }


    long getHash(value_key key){
      long hash=(long)key%gd;
      //transform key to hash
      return hash;
    }

    void insert(long address_register,value_key key){

      long hash=getHash(key);
      long address_bucket=hash;

      Bucket bucket;

      do{
        control_bucket->retrieve_record(address_bucket,bucket);
        address_bucket=bucket.NextBucket;
      }
      while(bucket.NextBucket!=-1);

      if(bucket.size==fd){
        Bucket new_bucket;
        new_bucket.address[0]=address_register;
        new_bucket.size=1;
        long pos=control_bucket->write_record_toending(new_bucket);
        bucket.NextBucket=pos;
        control_bucket->write_record(address_bucket,bucket);
      }
      else{
        bucket.address[bucket.size]=address_register;
        bucket.size++;
        control_bucket->write_record(address_bucket,bucket);
      }
    }
    value_key next_value(value_key value){
      value_key t;
      //next_
      return t;
    }

    std::vector<long> search(value_key begin, value_key end){
      std::vector<long> result;
      for(value_key i=begin;;i=next_value(i)){
        long hash=getHash(i);
        long address_bucket=hash;
        Bucket bucket;
        do{
          control_bucket->retrieve_record(address_bucket,bucket);
          address_bucket=bucket.NextBucket;
          for(int j=0;j<bucket.size;j++)
            result.push_back(bucket.address[j]);
        }
        while(bucket.NextBucket!=-1);
        if(i==end)
          break;
      }
      return result;
    }

  };
}


