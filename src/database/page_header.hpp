#ifndef INCLUDE_SRC_PAGE_HEADER_HPP_
#define INCLUDE_SRC_PAGE_HEADER_HPP_

#include <fstream>
#include <iostream>

#include "page_type.hpp"
#include "utils.hpp"

class PageHeader {
  public:
  static PageHeader create(std::ifstream& db) { return {db}; }

  PageType type() const { return type_; }
  uint16_t num_of_cells() const { return num_of_cells_; }
  uint16_t start_of_cell() const { return start_of_cell_; }
  uint8_t num_of_frag_free() const { return num_of_frag_free_; }
  uint32_t right_most_ptr() const { return right_most_ptr_; }

  void print() const {
    std::cout << "=== Page Header ===\n";

    std::cout << "Type: ";
    switch (type_) {
    case PageType::INTERIOR_INDEX:
      std::cout << "Interior Index";
      break;
    case PageType::INTERIOR_TABLE:
      std::cout << "Interior Table";
      break;
    case PageType::LEAF_INDEX:
      std::cout << "Leaf Index";
      break;
    case PageType::LEAF_TABLE:
      std::cout << "Leaf Table";
      break;
    default:
      std::cout << static_cast<int>(type_);
    }
    std::cout << '\n';

    std::cout << "First Free Block: " << first_freeblock_ << '\n';
    std::cout << "Cell Count: " << num_of_cells_ << '\n';
    std::cout << "Cell Content Area: " << start_of_cell_ << '\n';
    std::cout << "Fragment Free Bytes: " << num_of_frag_free_ << '\n';

    switch (type_) {
    case PageType::INTERIOR_INDEX:
    case PageType::INTERIOR_TABLE:
      std::cout << "Right-most Pointer: " << right_most_ptr_ << '\n';
      break;
    case PageType::LEAF_INDEX:
    case PageType::LEAF_TABLE:
      break;
    }

    std::cout << '\n';
  }

  private:
  PageHeader(std::ifstream& db) : db_{db} { parse(); }

  std::ifstream& db_;

  PageType type_{};
  uint16_t first_freeblock_{};
  uint16_t num_of_cells_{};
  uint16_t start_of_cell_{};
  uint8_t num_of_frag_free_{};
  uint32_t right_most_ptr_{};

  void parse() {
    type_ = static_cast<PageType>(read<uint8_t>(db_));
    first_freeblock_ = read<uint16_t>(db_);
    num_of_cells_ = read<uint16_t>(db_);
    start_of_cell_ = read<uint16_t>(db_);
    num_of_frag_free_ = read<uint8_t>(db_);

    switch (type_) {
    case PageType::INTERIOR_INDEX:
    case PageType::INTERIOR_TABLE:
      right_most_ptr_ = read<uint32_t>(db_);
      break;
    case PageType::LEAF_INDEX:
    case PageType::LEAF_TABLE:
      break;
    }
  }
};

#endif // INCLUDE_SRC_PAGE_HEADER_HPP_
