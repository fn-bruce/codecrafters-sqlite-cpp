#include <iostream>

#include "page.hpp"
#include "cell.hpp"

TableLeafPage::TableLeafPage(std::ifstream& db)
    : offset_{db.tellg() != 100 ? static_cast<size_t>(db.tellg()) : 0}, header_{PageHeader(db)},
      cells_{read_cell<TableLeafCell>(offset_, header_.num_of_cells(), db)} {}

void TableLeafPage::print() const {
  std::cout << "=== Page ===\n";
  std::cout << "Offset: " << offset_ << '\n';
  std::cout << '\n';
  header_.print();
  for (const auto& c : cells_) {
    c.print();
  }
}

std::vector<TableLeafPage> read_pages(DatabaseHeader& header, std::ifstream& db) {
  std::vector<TableLeafPage> pages{};
  for (size_t i{}; i < static_cast<size_t>(header.page_count()); ++i) {
    size_t offset{header.page_size() * i};
    if (i == 0) {
      offset += 100;
    }
    db.seekg(offset);
    pages.emplace_back(db);
  }
  return pages;
}
