#ifndef INCLUDE_SRC_PAGE_HEADER_HPP_
#define INCLUDE_SRC_PAGE_HEADER_HPP_

#include <fstream>

#include "page_type.hpp"

class PageHeader {
public:
  PageHeader(std::ifstream& db);

  PageType type() const { return type_; }
  uint16_t num_of_cells() const { return num_of_cells_; }
  uint16_t start_of_cell() const { return start_of_cell_; }
  uint8_t num_of_frag_free() const { return num_of_frag_free_; }
  uint32_t right_most_ptr() const { return right_most_ptr_; }

  void print() const;

private:
  std::ifstream& db_;

  PageType type_{};
  uint16_t first_freeblock_{};
  uint16_t num_of_cells_{};
  uint16_t start_of_cell_{};
  uint8_t num_of_frag_free_{};
  uint32_t right_most_ptr_{};

  void parse();
};

#endif // INCLUDE_SRC_PAGE_HEADER_HPP_
