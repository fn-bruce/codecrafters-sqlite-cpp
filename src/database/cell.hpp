#ifndef INCLUDE_SRC_CELL_HPP_
#define INCLUDE_SRC_CELL_HPP_

#include <cstdint>
#include <fstream>

#include "record.hpp"

class Cell {
  public:
  static Cell create(std::ifstream& db) { return {db}; }

  uint64_t row_id() const { return row_id_; }
  const Record& record() const { return record_; }

  void print() const {
    std::cout << "=== Cell ===\n";
    std::cout << "Offset: " << offset_ << '\n';
    std::cout << "Payload Size: " << payload_size_ << '\n';
    std::cout << "Row ID: " << row_id_ << '\n';
    std::cout << '\n';
    record_.print();
  }

  private:
  Cell(std::ifstream& db)
      : offset_{static_cast<uint16_t>(db.tellg())},
        payload_size_{read_varint(db).first}, row_id_{read_varint(db).first},
        record_{Record::create(db)} {}

  uint16_t offset_{};
  uint64_t payload_size_{};
  uint64_t row_id_{};
  Record record_;
};

#endif // INCLUDE_SRC_CELL_HPP_
