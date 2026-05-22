#include <cstring>
#include <ios>
#include <iostream>
#include <string>
#include <variant>

#include "database/database.hpp"
#include "database/tables.hpp"
#include "parser/parser.hpp"
#include "parser/tokenizer.hpp"

void print_tokens(const Tokens& tokens) {
  for (const auto& t : tokens) {
    std::cout << t.name << '\n';
  }
}

int main(int argc, char* argv[]) {
  constexpr int ARG_COUNT{3};

  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  if (argc != ARG_COUNT) {
    std::cerr << "Expected two arguments" << std::endl;
    return 1;
  }

  std::string database_file_path{argv[1]};
  std::string command{argv[2]};

  const Database database{database_file_path};
  const Tables& tables{database.tables()};

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
    } else if (std::holds_alternative<SelectColsStmt>(stmt)) {
      auto select_cols_stmt{std::get<SelectColsStmt>(stmt)};
      auto col_names{select_cols_stmt.col_names};
      auto table_name{select_cols_stmt.table_name};
      auto where_clause{select_cols_stmt.where_clause};
      tables.print(table_name, col_names, where_clause);
    }
  }

  return 0;
}
