#ifndef INCLUDE_SRC_CELLS_HPP_
#define INCLUDE_SRC_CELLS_HPP_

#include <vector>

#include "cell.hpp"

class Cells : public std::vector<Cell> {
public:
  Cells(size_t page_offset, uint16_t count, std::ifstream& db);

  void print() const;
};

#endif // INCLUDE_SRC_CELLS_HPP_
