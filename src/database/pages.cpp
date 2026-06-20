#include <iostream>

#include "pages.hpp"

Pages::Pages(DatabaseHeader& header, std::ifstream& db) {
  for (size_t i{}; i < static_cast<size_t>(header.page_count()); ++i) {
    size_t offset{header.page_size() * i};
    if (i == 0) {
      offset += 100;
    }
    db.seekg(offset);
    emplace_back(db);
  }
}

void Pages::print() const {
  for (const auto& p : *this) {
    p.print();
  }
}
