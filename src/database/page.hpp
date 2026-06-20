#ifndef INCLUDE_SRC_PAGE_HPP_
#define INCLUDE_SRC_PAGE_HPP_

#include <fstream>
#include <iostream>

#include "cells.hpp"
#include "page_header.hpp"

class Page {
  public:
  static Page create(std::ifstream& db) { return {db}; }

  void print() const {
    std::cout << "=== Page ===\n";
    std::cout << "Offset: " << offset_ << '\n';
    std::cout << '\n';
    header_.print();
    cells_.print();
  }

  uint16_t cell_count() const { return header_.num_of_cells(); }

  const Cells& cells() const { return cells_; }

  private:
  Page(std::ifstream& db)
      : offset_{db.tellg() != 100 ? static_cast<size_t>(db.tellg()) : 0},
        header_{PageHeader::create(db)},
        cells_{Cells(offset_, header_.num_of_cells(), db)} {}

  size_t offset_;
  PageHeader header_;
  Cells cells_;
};

#endif // INCLUDE_SRC_PAGE_HPP_
