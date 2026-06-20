#include "cells.hpp"
#include "utils.hpp"


Cells::Cells(size_t page_offset, uint16_t count, std::ifstream& db) {
  std::vector<uint16_t> cell_ptrs{};
  cell_ptrs.reserve(static_cast<size_t>(count));
  for (size_t i{}; i < static_cast<size_t>(count); ++i) {
    auto cell_ptr{read<uint16_t>(db)};
    cell_ptrs.push_back(cell_ptr);
  }

  for (const auto& ptr : cell_ptrs) {
    db.seekg(page_offset + static_cast<size_t>(ptr));
    emplace_back(db);
  }
}

void Cells::print() const {
  for (const auto& c : *this) {
    c.print();
  }
}
