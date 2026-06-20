#ifndef INCLUDE_SRC_CELL_HPP_
#define INCLUDE_SRC_CELL_HPP_

#include <cstdint>
#include <fstream>

#include "record.hpp"
#include "utils.hpp"

class TableInteriorCell {
public:
  TableInteriorCell(std::ifstream &db);

  uint64_t left_child() const { return left_child_; }
  uint64_t row_id() const { return row_id_; }

  void print() const;

private:
  uint64_t left_child_{};
  uint64_t row_id_{};
};

class TableLeafCell {
public:
  TableLeafCell(std::ifstream &db);

  uint64_t row_id() const { return row_id_; }
  const Record &record() const { return record_; }

  void print() const;

private:
  uint16_t offset_{};
  uint64_t payload_size_{};
  uint64_t row_id_{};
  Record record_;
};

template <typename T>
std::vector<T> read_cell(size_t page_offset, uint16_t count, std::ifstream &db) {
  std::vector<T> cells{};
  std::vector<uint16_t> cell_ptrs{};
  cell_ptrs.reserve(static_cast<size_t>(count));
  for (size_t i{}; i < static_cast<size_t>(count); ++i) {
    auto cell_ptr{read<uint16_t>(db)};
    cell_ptrs.push_back(cell_ptr);
  }

  for (const auto& ptr : cell_ptrs) {
    db.seekg(page_offset + static_cast<size_t>(ptr));
    cells.emplace_back(db);
  }

  return cells;
}

#endif // INCLUDE_SRC_CELL_HPP_
