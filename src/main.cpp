#include <cstring>
#include <ios>
#include <iostream>
#include <string>
#include <variant>

#include "database/database.hpp"
#include "database/tables.hpp"
#include "parser/parser.hpp"
#include "parser/tokenizer.hpp"

int main(int argc, char* argv[]) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  if (argc != 3) {
    std::cerr << "Expected two arguments" << std::endl;
    return 1;
  }

  std::string database_file_path = argv[1];
  std::string command = argv[2];

  Database database{database_file_path};

  auto tables_result{Tables::create(database)};
  if (!tables_result) {
    throw std::runtime_error("error creating tables");
  }

  auto tables{tables_result.value()};

  if (command == ".dbinfo") {
    std::cout << "number of tables: " << database.table_count() << '\n';
    std::cout << "database page size: " << database.page_size() << '\n';
  } else if (command == ".tables") {
    tables.print_table_names();
  } else {
    auto tokenizer{Tokenizer{command}};
    auto tokens{tokenizer.tokenize()};
    auto parser{Parser{tokens}};
    auto stmt{parser.parse()};

    if (std::holds_alternative<SelectAllStmt>(stmt)) {
      auto select_all_stmt{std::get<SelectAllStmt>(stmt)};
      auto table_name{select_all_stmt.name};
      auto row_count{tables.row_count(table_name)};
      std::cout << row_count << '\n';
    } else if (std::holds_alternative<SelectColStmt>(stmt)) {
      auto select_col_stmt{std::get<SelectColStmt>(stmt)};
      auto col_name{select_col_stmt.col_name};
      auto table_name{select_col_stmt.table_name};
      tables.print(table_name, col_name);
    }
  }

  return 0;
}
