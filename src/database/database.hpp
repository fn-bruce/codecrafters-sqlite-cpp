#ifndef INCLUDE_SRC_DATABASE_HPP_
#define INCLUDE_SRC_DATABASE_HPP_

#include <cassert>
#include <string_view>

#include "database_header.hpp"
#include "pages.hpp"

class Database {
  public:
  Database(std::string_view file_path)
      : file_path_{file_path}, db_{init_db(file_path)},
        header_{DatabaseHeader::create(db_)},
        pages_{Pages::create(header_, db_)} {}

  const Page& schema_page() const {
    assert(!pages_.empty() && "schema page doesn't exist in empty pages");
    return pages_.front();
  }

  const Page& page(size_t root_page) const {
    assert(root_page != 0 && "root_page is never 0");
    size_t idx{root_page - 1};
    assert(idx <= pages_.size() && "idx for page doesn't exist");
    return pages_[idx];
  }

  uint16_t page_size() const { return header_.page_size(); }

  uint16_t table_count() const {
    if (!pages_.size()) {
      throw std::runtime_error("page size is 0");
    }

    return pages_.front().cell_count();
  }

  void print() const {
    header_.print();
    pages_.print();
  }

  private:
  const std::string file_path_;
  std::ifstream db_;
  DatabaseHeader header_;
  Pages pages_{};

  std::ifstream init_db(std::string_view file_path) {
    std::ifstream db{file_path_, std::ios::binary};
    if (!db) {
      throw std::runtime_error("failed to open the database file");
    }
    return db;
  }

  std::vector<std::string> get_col_names(std::string tbl_name) const {
    if (!pages_.size()) {
      throw std::runtime_error("page size is 0");
    }

    auto first_page{pages_.front()};
    auto cells{first_page.cells()};

    return {};
  }
};

#endif // INCLUDE_SRC_DATABASE_HPP_
