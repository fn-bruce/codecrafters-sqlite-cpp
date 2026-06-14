#ifndef INCLUDE_SRC_DATABASE_HPP_
#define INCLUDE_SRC_DATABASE_HPP_

#include <cassert>
#include <stdexcept>
#include <string_view>
#include <variant>

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

  const std::vector<std::string_view> table_names() const {
    std::vector<std::string_view> table_names{};
    table_names.reserve(tables_.size());
    for (const auto &t : tables_) {
      table_names.push_back(t.tbl_name());
    }
    return table_names;
  }

  const Tables &tables() const { return tables_; }

  size_t row_count(std::string_view tbl_name) const {
    for (const auto &t : tables_) {
      if (t.tbl_name() == tbl_name) {
        return t.row_count();
      }
    }
    return 0;
  }

  uint16_t page_size() const { return header_.page_size(); }

  uint16_t table_count() const {
    if (!pages_.size()) {
      throw std::runtime_error("page size is 0");
    }

    return pages_.front().cell_count();
  }

  void execute(std::string_view command) const {
    if (command == ".dbinfo") {
      std::cout << "number of tables: " << table_count() << '\n';
      std::cout << "database page size: " << page_size() << '\n';
    } else if (command == ".tables") {
      for (const auto &n : table_names()) {
        std::cout << n << ' ';
      }
      std::cout << '\n';
    } else {
      auto tokenizer{Tokenizer{command}};
      auto tokens{tokenizer.tokenize()};
      auto parser{Parser{tokens}};
      auto stmt{parser.parse()};

      if (std::holds_alternative<SelectAllStmt>(stmt)) {
        auto select_all_stmt{std::get<SelectAllStmt>(stmt)};
        auto table_name{select_all_stmt.name};
        auto count{row_count(table_name)};
        std::cout << count << '\n';
      } else if (std::holds_alternative<SelectColsStmt>(stmt)) {
        auto select_cols_stmt{std::get<SelectColsStmt>(stmt)};
        auto col_names{select_cols_stmt.col_names};
        auto table_name{select_cols_stmt.table_name};
        auto where_clause{select_cols_stmt.where_clause};
        print(table_name, col_names, where_clause);
      }
    }
  }

  void print() const {
    header_.print();
    pages_.print();
  }

  void print(std::string_view tbl_name,
             const std::vector<std::string> &col_names,
             std::optional<WhereClause> clause) const {
    tables_.print(tbl_name, col_names, clause);
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
    for (const auto &schema_page_cell : schema.cells()) {
      const auto &schema_page_record{schema_page_cell.record()};
      const auto &vals{schema_page_record.values};

      const auto &table_name_res{vals[NAME_IDX]};
      if (!std::holds_alternative<String>(table_name_res)) {
        throw std::runtime_error("issue getting table name");
      }

      const auto &table_name{std::get<String>(table_name_res)};
      if (table_name == SQLITE_SEQ_TBL_NAME) {
        continue;
      }

      const auto &create_stmt_str_res{vals.back()};
      if (!std::holds_alternative<String>(create_stmt_str_res)) {
        throw std::runtime_error("issue getting create statment");
      }

      const auto &create_stmt_str{std::get<String>(create_stmt_str_res)};
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
      for (const auto &[name, _] : cols) {
        col_names.emplace_back(name);
      }

      const auto root_page_res{vals[ROOT_PAGE_IDX]};
      if (!std::holds_alternative<Int8>(root_page_res)) {
        throw std::runtime_error("issue getting root page");
      }

      const auto root_page{std::get<Int8>(root_page_res)};
      const auto &tbl_page{page(root_page)};
      const auto &tbl_cells{tbl_page.cells()};

      std::vector<std::vector<std::string>> row_vals{};
      row_vals.reserve(tbl_cells.size());

      for (const auto &tbl_cell : tbl_cells) {
        const auto &tbl_record{tbl_cell.record()};
        const auto &tbl_vals{tbl_record.values};

        std::vector<std::string> vals{};
        vals.reserve(cols.size());

        for (size_t i{}; i < cols.size(); ++i) {
          const auto &[_, col]{cols[i]};

          const auto &tbl_val{tbl_vals[i]};

          if (std::holds_alternative<Null>(tbl_val) && col.col_name == "id" &&
              col.primary_key) {
            vals.emplace_back(std::to_string(tbl_cell.row_id()));
            continue;
          }

          std::string tbl_val_str{};
          if (std::holds_alternative<Int8>(tbl_val)) {
            tbl_val_str = std::to_string(std::get<Int8>(tbl_val));
          } else if (std::holds_alternative<Int16>(tbl_val)) {
            tbl_val_str = std::to_string(std::get<Int16>(tbl_val));
          } else if (std::holds_alternative<Int24>(tbl_val)) {
            Int32 tmp{};
            for (const auto &b : std::get<Int24>(tbl_val)) {
              tmp |= b;
              tmp >>= 8;
            }
            tbl_val_str = std::to_string(tmp);
          } else if (std::holds_alternative<Int32>(tbl_val)) {
            tbl_val_str = std::to_string(std::get<Int32>(tbl_val));
          } else if (std::holds_alternative<Int48>(tbl_val)) {
            Int32 tmp{};
            for (const auto &b : std::get<Int48>(tbl_val)) {
              tmp |= b;
              tmp <<= 8;
            }
            tbl_val_str = std::to_string(tmp);
          } else if (std::holds_alternative<Int64>(tbl_val)) {
            tbl_val_str = std::to_string(std::get<Int64>(tbl_val));
          } else if (std::holds_alternative<String>(tbl_val)) {
            tbl_val_str = std::get<std::string>(tbl_val);
          } else {
            throw std::runtime_error("issue evaluating val type");
          }

          vals.emplace_back(tbl_val_str);
        }
        row_vals.emplace_back(vals);
      }

      tables.emplace_back(Table::create(table_name, col_names, row_vals));
    }

    return tables;
  }

  const Page &schema_page() const {
    assert(!pages_.empty() && "schema page doesn't exist in empty pages");
    return pages_.front();
  }

  const Page &page(size_t root_page) const {
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
