#ifndef MEMORY_LAYOUT_H
#define MEMORY_LAYOUT_H

#include <utility>
#include <cstring>
#include "uint_helper.h"

namespace aux {
  template<typename U>
  void le_to_host_aux(U& num);
  
  template<typename U>
  void be_to_host_aux(U& num);
  
  template<>
  void le_to_host_aux<uint8_t>(uint8_t&);

  template<>
  void le_to_host_aux<uint16_t>(uint16_t& u);

  template<>
  void le_to_host_aux<uint32_t>(uint32_t& u);

  template<>
  void le_to_host_aux<uint64_t>(uint64_t& u);

  template<>
  void be_to_host_aux<uint8_t>(uint8_t& u);

  template<>
  void be_to_host_aux<uint16_t>(uint16_t& u);

  template<>
  void be_to_host_aux<uint32_t>(uint32_t& u);

  template<>
  void be_to_host_aux<uint64_t>(uint64_t& u);
}

namespace memory_layout {
  template<typename T>
  void le_to_host(T& num) {
    aux::le_to_host_aux(*(uint_eq<T>*)(&num));
  }

  template<typename T>
  void be_to_host(T& num) {
    aux::be_to_host_aux(*(uint_eq<T>*)(&num));
  }

  template<typename T>
  void host_to_le(T& num) {
    uint_eq<T> in=*((uint_eq<T>*)&num);
    uint8_t* out=(uint8_t*)&num;
    for(int i=0;i<sizeof(T);++i) {
      out[i]=in&(0xFF);
      in>>=8;
    }
  }

  template<typename T>
  void host_to_be(T& num) {
    uint_eq<T> in=*((uint_eq<T>*)&num);
    uint8_t* out=(uint8_t*)&num;
    for(int i=sizeof(T)-1;i>=0;--i) {
      out[i]=in&(0xFF);
      in>>=8;
    }
  }

#pragma clang diagnostic ignored "-Wunused-value"
  template<typename... Args>
  void les_to_host(Args&... args) {
    using expand_type = int[];
    expand_type { (le_to_host(args), 0)... };
  }

  template<typename... Args>
  void bes_to_host(Args&... args) {
    using expand_type = int[];
    expand_type { (be_to_host(args), 0)... };
  }

  template<typename... Args>
  void host_to_les(Args&... args) {
    using expand_type = int[];
    expand_type { (host_to_le(args), 0)... };
  }

  template<typename... Args>
  void host_to_bes(Args&... args) {
    using expand_type = int[];
    expand_type { (host_to_be(args), 0)... };
  }

  void pad_le(uint8_t* dst, uint8_t* src, int block_sz_in, int block_sz_out, int num_blocks);
  
  void pad_be(uint8_t* dst, uint8_t* src, int block_sz_in, int block_sz_out, int num_blocks);

  template<typename T>
  void le_to_host_arr(uint8_t* arr, int num_blocks) {
    for(int i=0;i<num_blocks;++i)
      le_to_host(*((T*)(arr+i*sizeof(T))));
  }

  template<typename T>
  void be_to_host_arr(uint8_t* arr, int num_blocks) {
    for(int i=0;i<num_blocks;++i)
      be_to_host(*((T*)(arr+i*sizeof(T))));
  }

  template<typename T>
  void host_to_le_arr(uint8_t* arr, int num_blocks) {
    for(int i=0;i<num_blocks;++i)
      host_to_le(*((T*)(arr+i*sizeof(T))));
  }

  template<typename T>
  void host_to_be_arr(uint8_t* arr, int num_blocks) {
    for(int i=0;i<num_blocks;++i)
      host_to_be(*((T*)(arr+i*sizeof(T))));
  }
}
#endif
