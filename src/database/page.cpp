#include <iostream>

#include "page.hpp"

TableLeafPage::TableLeafPage(std::ifstream& db)
    : offset_{db.tellg() != 100 ? static_cast<size_t>(db.tellg()) : 0}, header_{PageHeader(db)},
      cells_{Cells(offset_, header_.num_of_cells(), db)} {}

void TableLeafPage::print() const {
  std::cout << "=== Page ===\n";
  std::cout << "Offset: " << offset_ << '\n';
  std::cout << '\n';
  header_.print();
  cells_.print();
}
