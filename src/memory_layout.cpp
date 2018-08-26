#include "memory_layout.h"
#include <cstring>

namespace aux {
  
  template<typename U>
  void le_to_host_aux(U& num);

  template<typename U>
  void be_to_host_aux(U& num);
  
  template<>
  void le_to_host_aux<uint8_t>(uint8_t& u) {
    return;
  }

  template<>
  void le_to_host_aux<uint16_t>(uint16_t& u) {
    uint8_t *pu=(uint8_t*)&u;
    u=pu[0] | pu[1]<<8;
  }

  template<>
  void le_to_host_aux<uint32_t>(uint32_t& u) {
    uint8_t *pu=(uint8_t*)&u;
    u=pu[0] | pu[1]<<8 | pu[2]<<16 | pu[3]<<24;
  }

  template<>
  void le_to_host_aux<uint64_t>(uint64_t& u) {
    uint8_t *pu=(uint8_t*)&u;
    u=pu[0] | pu[1]<<8 | pu[2]<<16 | pu[3]<<24 | (uint64_t)pu[4]<<32 | (uint64_t)pu[5]<<40 | (uint64_t)pu[6]<<48 | (uint64_t)pu[7]<<56;
  }

  template<>
  void be_to_host_aux<uint8_t>(uint8_t& u) {
    return;
  }

  template<>
  void be_to_host_aux<uint16_t>(uint16_t& u) {
    uint8_t *pu=(uint8_t*)&u;
    u=pu[1] | pu[0]<<8;
  }

  template<>
  void be_to_host_aux<uint32_t>(uint32_t& u) {
    uint8_t *pu=(uint8_t*)&u;
    u=pu[3] | pu[2]<<8 | pu[1]<<16 | pu[0]<<24;
  }

  template<>
  void be_to_host_aux<uint64_t>(uint64_t& u) {
    uint8_t *pu=(uint8_t*)&u;
    u=pu[7] | pu[6]<<8 | pu[5]<<16 | pu[4]<<24 | (uint64_t)pu[3]<<32 | (uint64_t)pu[2]<<40 | (uint64_t)pu[1]<<48 | (uint64_t)pu[0]<<56;
  }
}

namespace memory_layout {
  
  void pad_le(uint8_t* dst, uint8_t* src, int block_sz_in, int block_sz_out, int num_blocks) {
    memset(dst,0,num_blocks*block_sz_out);
    for(int i=0;i<num_blocks;++i)
      memcpy(dst+i*block_sz_out+(block_sz_out-block_sz_in),src+i*block_sz_in, block_sz_in);
  }
  
  void pad_be(uint8_t* dst, uint8_t* src, int block_sz_in, int block_sz_out, int num_blocks) {
    memset(dst,0,num_blocks*block_sz_out);
    for(int i=0;i<num_blocks;++i)
      memcpy(dst+i*block_sz_out,src+i*block_sz_in, block_sz_in);
  }
}
