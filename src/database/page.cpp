#include <iostream>
#include <variant>

#include "cell.hpp"
#include "page.hpp"

TableInteriorPage::TableInteriorPage(std::ifstream &db)
    : offset_{db.tellg() != 100 ? static_cast<size_t>(db.tellg()) : 0},
      header_{PageHeader(db)} {
        pages_ = read_pages(header_, db);
      }

void TableInteriorPage::print() const {
  std::cout << "=== Page ===\n";
  std::cout << "Offset: " << offset_ << '\n';
  std::cout << '\n';
  header_.print();
  for (const auto &c : pages_) {
    if (std::holds_alternative<TableInteriorPage>(c)) {
      std::get<TableInteriorPage>(c).print();
      continue;
    }
    std::get<TableLeafPage>(c).print();
  }
}

TableLeafPage::TableLeafPage(std::ifstream &db)
    : offset_{db.tellg() != 100 ? static_cast<size_t>(db.tellg()) : 0},
      header_{PageHeader(db)},
      cells_{read_cell<TableLeafCell>(offset_, header_.num_of_cells(), db)} {}

void TableLeafPage::print() const {
  std::cout << "=== Page ===\n";
  std::cout << "Offset: " << offset_ << '\n';
  std::cout << '\n';
  header_.print();
  for (const auto &c : cells_) {
    c.print();
  }
}

std::vector<Page> read_pages(DatabaseHeader &header, std::ifstream &db) {
  std::vector<Page> pages{};
  for (size_t i{}; i < static_cast<size_t>(header.page_count()); ++i) {
    size_t offset{header.page_size() * i};
    if (i == 0) {
      offset += 100;
    }
    db.seekg(offset);

    // read page header first
    auto header_offset{db.tellg()};
    PageHeader header{db};
    db.seekg(header_offset);

    switch (header.type()) {
    case PageType::INTERIOR_TABLE:
      pages.emplace_back<TableInteriorPage>(db);
      break;
    case PageType::LEAF_TABLE:
      pages.emplace_back<TableLeafPage>(db);
      break;
    default:
      break;
    }
  }
  return pages;
}

std::vector<Page> read_pages(PageHeader &header, std::ifstream &db) {
  std::vector<Page> pages{};
  for (size_t i{}; i < static_cast<size_t>(header.num_of_cells()); ++i) {
    size_t offset{header.num_of_cells() * i};
    db.seekg(offset);

    switch (header.type()) {
    case PageType::INTERIOR_TABLE:
      pages.emplace_back<TableInteriorPage>(db);
      break;
    case PageType::LEAF_TABLE:
      pages.emplace_back<TableLeafPage>(db);
      break;
    default:
      break;
    }
  }
  return pages;
}
