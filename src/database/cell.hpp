#ifndef INCLUDE_SRC_CELL_HPP_
#define INCLUDE_SRC_CELL_HPP_

#include <cstdint>
#include <fstream>

#include "record.hpp"

struct TableLeafCell {
  uint64_t payload_size{};
  uint64_t row_id{};
};

class Cell {
public:
  Cell(std::ifstream &db);

  uint64_t row_id() const { return row_id_; }
  const Record &record() const { return record_; }

  void print() const;

private:
  uint16_t offset_{};
  uint64_t payload_size_{};
  uint64_t row_id_{};
  Record record_;
};

#endif // INCLUDE_SRC_CELL_HPP_
