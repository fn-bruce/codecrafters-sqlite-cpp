#ifndef INCLUDE_SRC_DATABASE_HPP_
#define INCLUDE_SRC_DATABASE_HPP_

#include <cassert>
#include <string_view>

#include "database_header.hpp"
#include "pages.hpp"
#include "tables.hpp"

#include "../parser/parser.hpp"
#include "../parser/tokenizer.hpp"

class Database {
  public:
  Database(std::string_view file_path)
      : file_path_{file_path}, db_{init_db(file_path)},
        header_{DatabaseHeader::create(db_)},
        pages_{Pages::create(header_, db_)}, tables_{init_tables()} {}

  const Tables& tables() const { return tables_; }

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
  Tables tables_{};

  std::ifstream init_db(std::string_view file_path) {
    std::ifstream db{file_path_, std::ios::binary};
    if (!db) {
      throw std::runtime_error("failed to open the database file");
    }
    return db;
  }

  Tables init_tables() {
    if (!page_size()) {
      // TODO: throw an error
    }

    Tables tables{};

    constexpr size_t NAME_IDX{1};
    constexpr size_t ROOT_PAGE_IDX{3};
    std::string_view SQLITE_SEQ_TBL_NAME{"sqlite_sequence"};
    auto schema{schema_page()};
    for (const auto& schema_page_cell : schema.cells()) {
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
      const auto& tbl_page{page(root_page)};
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
