#ifndef INCLUDE_SRC_PAGE_TYPE_HPP_
#define INCLUDE_SRC_PAGE_TYPE_HPP_

#include <cstdint>

enum class PageType : uint8_t {
  INTERIOR_INDEX = 0x02,
  INTERIOR_TABLE = 0x05,
  LEAF_INDEX = 0x0a,
  LEAF_TABLE = 0x0d,
};

#endif // INCLUDE_SRC_PAGE_TYPE_HPP_
