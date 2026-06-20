#ifndef INCLUDE_SRC_CELL_HPP_
#define INCLUDE_SRC_CELL_HPP_

#include <cstdint>
#include <fstream>
#include <iostream>

#include "record.hpp"

struct TableLeafCell {
  uint64_t payload_size{};
  uint64_t row_id{};
};

class Cell {
public:
  Cell(std::ifstream &db)
      : offset_{static_cast<uint16_t>(db.tellg())},
        payload_size_{read_varint(db).first}, row_id_{read_varint(db).first},
        record_{Record(db)} {}

  uint64_t row_id() const { return row_id_; }
  const Record &record() const { return record_; }

  void print() const {
    std::cout << "=== Cell ===\n";
    std::cout << "Offset: " << offset_ << '\n';
    std::cout << "Payload Size: " << payload_size_ << '\n';
    std::cout << "Row ID: " << row_id_ << '\n';
    std::cout << '\n';
  }

private:

  uint16_t offset_{};
  uint64_t payload_size_{};
  uint64_t row_id_{};
  Record record_;
};

#endif // INCLUDE_SRC_CELL_HPP_
