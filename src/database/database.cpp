#include <stdexcept>
#include <variant>

#include "database.hpp"
#include "../parser/tokenizer.hpp"

Database::Database(std::string_view file_path)
    : file_path_{file_path}, db_{init_db(file_path)},
      header_{DatabaseHeader(db_)},
      pages_{Pages(header_, db_)}, tables_{init_tables()} {}

const std::vector<std::string_view> Database::table_names() const {
  std::vector<std::string_view> table_names{};
  table_names.reserve(tables_.size());
  for (const auto &t : tables_) {
    table_names.push_back(t.tbl_name());
  }
  return table_names;
}

size_t Database::row_count(std::string_view tbl_name) const {
  for (const auto &t : tables_) {
    if (t.tbl_name() == tbl_name) {
      return t.row_count();
    }
  }
  return 0;
}

uint16_t Database::table_count() const {
  if (!pages_.size()) {
    throw std::runtime_error("page size is 0");
  }

  return pages_.front().cell_count();
}

void Database::execute(std::string_view command) const {
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

void Database::print() const {
  header_.print();
  pages_.print();
}

void Database::print(
    std::string_view tbl_name,
    const std::vector<std::string> &col_names,
    std::optional<WhereClause> clause) const {
  tables_.print(tbl_name, col_names, clause);
}

std::ifstream Database::init_db(std::string_view file_path) {
  std::ifstream db{file_path_, std::ios::binary};
  if (!db) {
    throw std::runtime_error("failed to open the database file");
  }
  return db;
}

Tables Database::init_tables() {
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
    const auto &vals{schema_page_record.values()};

    const auto &table_name_res{vals[NAME_IDX]};
    if (!std::holds_alternative<std::string>(table_name_res)) {
      throw std::runtime_error("issue getting table name");
    }

    const auto &table_name{std::get<std::string>(table_name_res)};
    if (table_name == SQLITE_SEQ_TBL_NAME) {
      continue;
    }

    const auto &create_stmt_str_res{vals.back()};
    if (!std::holds_alternative<std::string>(create_stmt_str_res)) {
      throw std::runtime_error("issue getting create statment");
    }

    const auto &create_stmt_str{std::get<std::string>(create_stmt_str_res)};
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
    if (!std::holds_alternative<int8_t>(root_page_res)) {
      throw std::runtime_error("issue getting root page");
    }

    const auto root_page{std::get<int8_t>(root_page_res)};
    const auto &tbl_page{page(root_page)};
    const auto &tbl_cells{tbl_page.cells()};

    std::vector<std::vector<std::string>> row_vals{};
    row_vals.reserve(tbl_cells.size());

    for (const auto &tbl_cell : tbl_cells) {
      const auto &tbl_record{tbl_cell.record()};
      const auto &tbl_vals{tbl_record.values()};

      std::vector<std::string> vals{};
      vals.reserve(cols.size());

      for (size_t i{}; i < cols.size(); ++i) {
        const auto &[_, col]{cols[i]};

        const auto &tbl_val{tbl_vals[i]};

        if (std::holds_alternative<std::monostate>(tbl_val) && col.col_name == "id" &&
            col.primary_key) {
          vals.emplace_back(std::to_string(tbl_cell.row_id()));
          continue;
        }

        std::string tbl_val_str{};
        if (std::holds_alternative<int8_t>(tbl_val)) {
          tbl_val_str = std::to_string(std::get<int8_t>(tbl_val));
        } else if (std::holds_alternative<int16_t>(tbl_val)) {
          tbl_val_str = std::to_string(std::get<int16_t>(tbl_val));
        } else if (std::holds_alternative<std::array<int8_t, 3>>(tbl_val)) {
          int32_t tmp{};
          for (const auto &b : std::get<std::array<int8_t, 3>>(tbl_val)) {
            tmp |= b;
            tmp >>= 8;
          }
          tbl_val_str = std::to_string(tmp);
        } else if (std::holds_alternative<int32_t>(tbl_val)) {
          tbl_val_str = std::to_string(std::get<int32_t>(tbl_val));
        } else if (std::holds_alternative<std::array<int8_t, 5>>(tbl_val)) {
          int32_t tmp{};
          for (const auto &b : std::get<std::array<int8_t, 5>>(tbl_val)) {
            tmp |= b;
            tmp <<= 8;
          }
          tbl_val_str = std::to_string(tmp);
        } else if (std::holds_alternative<int64_t>(tbl_val)) {
          tbl_val_str = std::to_string(std::get<int64_t>(tbl_val));
        } else if (std::holds_alternative<std::string>(tbl_val)) {
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

const TableLeafPage &Database::schema_page() const {
  assert(!pages_.empty() && "schema page doesn't exist in empty pages");
  return pages_.front();
}

const TableLeafPage &Database::page(size_t root_page) const {
  assert(root_page != 0 && "root_page is never 0");
  size_t idx{root_page - 1};
  assert(idx <= pages_.size() && "idx for page doesn't exist");
  return pages_[idx];
}

std::vector<std::string> Database::get_col_names(std::string tbl_name) const {
  if (!pages_.size()) {
    throw std::runtime_error("page size is 0");
  }

  auto first_page{pages_.front()};
  auto cells{first_page.cells()};

  return {};
}
