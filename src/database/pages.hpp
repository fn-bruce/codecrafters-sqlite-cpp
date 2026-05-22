#ifndef INCLUDE_SRC_PAGES_HPP_
#define INCLUDE_SRC_PAGES_HPP_

#include <vector>

#include "database_header.hpp"
#include "page.hpp"

class Pages : public std::vector<Page> {
  public:
  static Pages create(const DatabaseHeader& header, std::ifstream& db) {
    Pages pages{};

    for (size_t i{}; i < static_cast<size_t>(header.page_count()); ++i) {
      size_t offset{header.page_size() * i};
      if (i == 0) {
        offset += 100;
      }
      db.seekg(offset);
      pages.push_back(Page::create(db));
    }

    return pages;
  }

  void print() const {
    for (const auto& p : *this) {
      p.print();
    }
  }
};

#endif // INCLUDE_SRC_PAGES_HPP_
