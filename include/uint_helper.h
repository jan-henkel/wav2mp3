#ifndef UINT_HELPER_H
#define UINT_HELPER_H

#include <cstdint>

namespace aux {
  template<int N>
  struct uint_t_struct;

  template<>
  struct uint_t_struct<1> {
    using type = uint8_t;
  };

  template<>
  struct uint_t_struct<2> {
    using type = uint16_t;
  };

  template<>
  struct uint_t_struct<4> {
    using type = uint32_t;
  };

  template<>
  struct uint_t_struct<8> {
    using type = uint64_t;
  };
}


template<int N>
using uint_t = typename aux::uint_t_struct<N>::type;

template<typename T>
using uint_eq = uint_t<sizeof(T)>;

#endif
