#include <iostream>

#include "cell.hpp"
#include "utils.hpp"

TableInteriorCell::TableInteriorCell(std::ifstream &db)
  : left_child_{read_varint(db).first},
    row_id_{read_varint(db).first} {}

void TableInteriorCell::print() const {
  std::cout << "=== Cell ===\n";
  std::cout << "Row ID: " << row_id_ << '\n';
  std::cout << "Left Child: " << left_child_ << '\n';
  std::cout << '\n';
}

TableLeafCell::TableLeafCell(std::ifstream &db)
    : offset_{static_cast<uint16_t>(db.tellg())},
      payload_size_{read_varint(db).first}, row_id_{read_varint(db).first},
      record_{Record(db)} {}

void TableLeafCell::print() const {
  std::cout << "=== Cell ===\n";
  std::cout << "Offset: " << offset_ << '\n';
  std::cout << "Payload Size: " << payload_size_ << '\n';
  std::cout << "Row ID: " << row_id_ << '\n';
  std::cout << '\n';
}
