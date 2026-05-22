#ifndef INCLUDE_DATABASE_TABLES_HPP_
#define INCLUDE_DATABASE_TABLES_HPP_

#include <expected>
#include <vector>

#include "../parser/parser.hpp"
#include "../parser/tokenizer.hpp"
#include "database.hpp"
#include "table.hpp"

class Tables : public std::vector<Table> {
  public:
  enum class Error {
    NoSchemaPageError,
  };

  static std::expected<Tables, Error> create(const Database& db) {
    if (!db.page_size()) {
      return std::unexpected(Error::NoSchemaPageError);
    }

    Tables tables{};

    constexpr size_t NAME_IDX{1};
    constexpr size_t ROOT_PAGE_IDX{3};
    std::string_view SQLITE_SEQ_TBL_NAME{"sqlite_sequence"};
    auto schema_page{db.schema_page()};
    for (const auto& schema_page_cell : schema_page.cells()) {
      const auto& schema_page_record{schema_page_cell.record()};
      const auto& vals{schema_page_record.vals()};
      const auto& table_name{vals[NAME_IDX]};

      if (table_name == SQLITE_SEQ_TBL_NAME) {
        continue;
      }

      const auto& create_stmt_str{vals.back()};
      auto tokenizer{Tokenizer{create_stmt_str}};
      auto tokens{tokenizer.tokenize()};
      auto parser{Parser{tokens}};
      const auto stmt_var{parser.parse()};

      if (!std::holds_alternative<CreateStmt>(stmt_var)) {
        throw std::runtime_error("create statement expected");
      }

      const auto create_stmt{std::get<CreateStmt>(stmt_var)};
      const auto cols{create_stmt.cols};

      std::vector<std::string> col_names{};
      col_names.reserve(cols.size());
      for (const auto& [name, _] : cols) {
        col_names.emplace_back(name);
      }

      const auto root_page_str{vals[ROOT_PAGE_IDX]};
      const auto root_page{std::stoull(root_page_str)};
      const auto& tbl_page{db.page(root_page)};
      const auto& tbl_cells{tbl_page.cells()};

      std::vector<std::vector<std::string>> row_vals{};
      row_vals.reserve(tbl_cells.size());

      for (const auto& tbl_cell : tbl_cells) {
        const auto& tbl_record{tbl_cell.record()};
        const auto& tbl_vals{tbl_record.vals()};

        std::vector<std::string> vals{};
        vals.reserve(cols.size());

        for (size_t i{}; i < cols.size(); ++i) {
          const auto& [_, col]{cols[i]};
          const auto& tbl_val{tbl_vals[i]};
          if (col.col_name == "id" && col.primary_key) {
            vals.emplace_back(std::to_string(tbl_cell.row_id()));
            continue;
          }
          vals.emplace_back(tbl_val);
        }
        row_vals.emplace_back(vals);
      }

      tables.emplace_back(Table::create(table_name, col_names, row_vals));
    }

    return tables;
  }

  size_t row_count(std::string_view tbl_name) {
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
