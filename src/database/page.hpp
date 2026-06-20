#ifndef INCLUDE_SRC_PAGE_HPP_
#define INCLUDE_SRC_PAGE_HPP_

#include <fstream>

#include "cells.hpp"
#include "page_header.hpp"

class Page {
public:
  Page(std::ifstream& db);

  void print() const;

  uint16_t cell_count() const { return header_.num_of_cells(); }

  const Cells& cells() const { return cells_; }

private:
  size_t offset_;
  PageHeader header_;
  Cells cells_;
};

#endif // INCLUDE_SRC_PAGE_HPP_
