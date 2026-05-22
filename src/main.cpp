#include <array>
#include <cstring>
#include <fstream>
#include <ios>
#include <iostream>
#include <string>
#include <variant>
#include <vector>

#include "database/database.hpp"
#include "database/tables.hpp"
#include "database/utils.hpp"
#include "parser/parser.hpp"
#include "parser/tokenizer.hpp"

constexpr int DATABASE_HEADER_SIZE{100};
constexpr int PAGE_HEADER_SIZE{8};
constexpr int CELL_PTR_SIZE{2};

constexpr int DATABASE_HEADER_OFFSET{0};
constexpr int PAGE_SIZE_OFFSET{16};
constexpr int PAGE_HEADER_OFFSET{100};
constexpr int TABLE_COUNT_OFFSET{103};
constexpr int CELL_POINTER_OFFSET{108};

void print_tokens(Tokens tokens) {
  for (const auto& t : tokens) {
    std::cout << t.name << '\n';
  }
}

unsigned short get_page_size(std::ifstream& db) {
  std::array<uint8_t, 2> buf{};
  db.seekg(PAGE_SIZE_OFFSET);
  db.read(reinterpret_cast<char*>(buf.data()), 2);
  return read_big_endian<uint16_t>(&buf[0]);
}

unsigned short get_table_count(std::ifstream& db) {
  std::array<uint8_t, 2> buf{};
  db.seekg(TABLE_COUNT_OFFSET);
  db.read(reinterpret_cast<char*>(buf.data()), 2);
  return read_big_endian<uint16_t>(&buf[0]);
}

std::vector<std::string> get_table_names(std::ifstream& db,
                                         const int table_count) {
  std::vector<std::string> names{};
  names.reserve(table_count);

  for (int i{}; i < table_count; ++i) {
    // get offset
    std::array<uint8_t, 2> offset_buf{};
    db.seekg(CELL_POINTER_OFFSET + (CELL_PTR_SIZE * i));
    db.read(reinterpret_cast<char*>(offset_buf.data()), 2);
    uint16_t offset{};
    for (const auto& b : offset_buf) {
      offset = (offset << 8) | b;
    }

    // go to offset
    db.seekg(offset);

    // size of record
    uint8_t record_size{};
    db.read(reinterpret_cast<char*>(&record_size), 1);

    // get rowid
    uint8_t row_id{};
    db.read(reinterpret_cast<char*>(&row_id), 1);

    // get record header size
    uint8_t record_header_size{};
    db.read(reinterpret_cast<char*>(&record_header_size), 1);

    // get serial types
    std::vector<int> serial_type_codes{};
    serial_type_codes.reserve(static_cast<size_t>(record_header_size - 1));

    size_t record_count{1};
    while (record_count < static_cast<size_t>(record_header_size)) {
      auto [result, bytes_read] = read_varint(db);
      record_count += bytes_read;
      serial_type_codes.push_back(result);
    }

    // skip type field
    size_t type_size = (serial_type_codes[0] - 13) / 2;
    db.seekg(type_size, std::ios::cur);

    // read name cell
    size_t name_size = (serial_type_codes[1] - 13) / 2;
    std::string name{};
    name.reserve(name_size);
    for (size_t i{}; i < name_size; ++i) {
      char c{};
      db.read(&c, 1);
      name += c;
    }

    if (name == "sqlite_sequence") {
      continue;
    }

    names.push_back(name);
  }

  return names;
}



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

  if (command == ".dbinfo") {
    std::cout << "number of tables: " << database.table_count() << '\n';
    std::cout << "database page size: " << database.page_size() << '\n';
  } else if (command == ".tables") {
    std::ifstream db(database_file_path, std::ios::binary);
    auto table_count{get_table_count(db)};
    auto table_names{get_table_names(db, table_count)};
    for (size_t i{}; i < table_names.size(); ++i) {
      std::cout << table_names[i];
      if (i + 1 < table_names.size())
        std::cout << ' ';
    }
    std::cout << '\n';
  } else {
    auto tables_result{Tables::create(database)};
    if (!tables_result) {
      throw std::runtime_error("error creating tables");
    }

    auto tables{tables_result.value()};

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
