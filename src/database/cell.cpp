#include <iostream>

#include "cell.hpp"
#include "utils.hpp"

Cell::Cell(std::ifstream &db)
    : offset_{static_cast<uint16_t>(db.tellg())},
      payload_size_{read_varint(db).first}, row_id_{read_varint(db).first},
      record_{Record(db)} {}

void Cell::print() const {
  std::cout << "=== Cell ===\n";
  std::cout << "Offset: " << offset_ << '\n';
  std::cout << "Payload Size: " << payload_size_ << '\n';
  std::cout << "Row ID: " << row_id_ << '\n';
  std::cout << '\n';
}
