#ifndef INCLUDE_DATABASE_TABLES_HPP_
#define INCLUDE_DATABASE_TABLES_HPP_

#include <vector>

#include "table.hpp"

class Tables : public std::vector<Table> {
  public:
  enum class Error {
    NoSchemaPageError,
  };

  size_t row_count(std::string_view tbl_name) const {
    for (const auto& t : *this) {
      if (t.tbl_name() == tbl_name) {
        return t.row_count();
      }
    }
    return 0;
  }

  void print_table_names() const {
    for (const auto& t : *this) {
      std::cout << t.tbl_name() << ' ';
    }
    std::cout << '\n';
  }

  void print(std::string_view tbl_name) const {
    for (const auto& t : *this) {
      if (t.tbl_name() == tbl_name) {
        t.print();
        break;
      }
    }
  }

  void print(std::string_view tbl_name, std::string_view col_name) const {
    for (const auto& t : *this) {
      if (t.tbl_name() == tbl_name) {
        t.print(col_name);
        break;
      }
    }
  }
};

#endif // INCLUDE_DATABASE_TABLES_HPP_
