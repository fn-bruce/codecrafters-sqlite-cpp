#ifndef INCLUDE_SRC_DATABASE_HPP_
#define INCLUDE_SRC_DATABASE_HPP_

#include <cassert>
#include <string_view>

#include "database_header.hpp"
#include "page.hpp"
#include "tables.hpp"

#include "../parser/parser.hpp"

class Database {
public:
  Database(std::string_view file_path);

  const std::vector<std::string_view> table_names() const;

  const Tables &tables() const { return tables_; }

  size_t row_count(std::string_view tbl_name) const;

  uint16_t page_size() const { return header_.page_size(); }

  uint16_t table_count() const;

  void execute(std::string_view command) const;

  void print() const;

  void print(std::string_view tbl_name,
             const std::vector<std::string> &col_names,
             std::optional<WhereClause> clause) const;

private:
  const std::string file_path_;
  std::ifstream db_;
  DatabaseHeader header_;
  std::vector<Page> pages_{};
  Tables tables_{};

  std::ifstream init_db(std::string_view file_path);

  Tables init_tables();

  const TableLeafPage &schema_page() const;

  const TableLeafPage &page(size_t root_page) const;

  std::vector<std::string> get_col_names(std::string tbl_name) const;
};

#endif // INCLUDE_SRC_DATABASE_HPP_
