#include <array>
#include <cstring>
#include <format>
#include <fstream>
#include <ios>
#include <iostream>
#include <string>
#include <variant>
#include <vector>

#include "parser.hpp"
#include "tokenizer.hpp"

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

template <typename T>
T read_big_endian(const uint8_t* data) {
  T result{};
  for (size_t i{}; i < sizeof(T); ++i) {
    result = (result << 8) | data[i];
  }
  return result;
}

std::pair<uint64_t, int> read_varint(std::ifstream& db) {
  constexpr uint8_t CONTINUATION_BIT{0b1000'0000};
  constexpr uint8_t MASK{0b0111'1111};
  uint64_t result{};
  int bytes_read{};
  uint8_t curr_byte{};
  do {
    db.read(reinterpret_cast<char*>(&curr_byte), 1);
    result = (result << 7) | (curr_byte & MASK);
    ++bytes_read;
  } while (curr_byte & CONTINUATION_BIT);
  return {result, bytes_read};
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

int get_all_row_count(std::ifstream& db, std::string table_name) {
  auto page_size{get_page_size(db)};
  auto table_count{get_table_count(db)};

  int count{};
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

    if (name != table_name) {
      continue;
    }

    // skip table name cell
    size_t table_name_size = (serial_type_codes[2] - 13) / 2;
    db.seekg(table_name_size, std::ios::cur);

    // get rootpage
    size_t rootpage_size{static_cast<size_t>(serial_type_codes[3])};
    uint8_t rootpage{};
    db.read(reinterpret_cast<char*>(&rootpage), 1);

    // navigate to rootpage
    size_t header_offset{static_cast<size_t>(rootpage - 1 == 0 ? 100 : 8)};
    size_t page_offset{(page_size * static_cast<size_t>(rootpage - 1)) +
                       header_offset};

    db.seekg(page_offset);
    while (true) {
      std::array<char, 2> curr_bytes{};
      db.read(reinterpret_cast<char*>(curr_bytes.data()), 2);
      // std::cout << "curr_byte: " << std::format("{:#x}", curr_byte) << '\n';
      if (!curr_bytes[0] && !curr_bytes[1]) {
        break;
      }
      ++count;
    }

    break;
  }

  return count;
}

int get_col_row(std::ifstream& db, std::string col_name,
                std::string table_name) {
  auto page_size{get_page_size(db)};
  auto table_count{get_table_count(db)};

  int count{};
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
    auto [record_header_size, rhs_bytes] = read_varint(db);

    // get serial types
    std::vector<int> serial_type_codes{};
    serial_type_codes.reserve(static_cast<size_t>(record_header_size - 1));

    size_t header_bytes_read{static_cast<size_t>(rhs_bytes)};
    while (header_bytes_read < record_header_size) {
      auto [result, bytes_read] = read_varint(db);
      header_bytes_read += bytes_read;
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

    if (name != table_name) {
      continue;
    }

    // skip table name cell
    size_t table_name_size = (serial_type_codes[2] - 13) / 2;
    db.seekg(table_name_size, std::ios::cur);

    // get rootpage
    size_t rootpage_size{static_cast<size_t>(serial_type_codes[3])};
    uint8_t rootpage{};
    db.read(reinterpret_cast<char*>(&rootpage), 1);

    // get create statement
    size_t statement_size{static_cast<size_t>((serial_type_codes[4] - 13) / 2)};
    std::string statement{};
    statement.reserve(statement_size);
    for (size_t i{}; i < statement_size; ++i) {
      char c{};
      db.read(&c, 1);
      statement += c;
    }

    // parse create statement
    auto tokenizer{Tokenizer{statement}};
    auto tokens{tokenizer.tokenize()};
    auto parser{Parser{tokens}};
    auto stmt_var{parser.parse()};

    if (!std::holds_alternative<CreateStmt>(stmt_var)) {
      throw std::runtime_error("create statement expected");
    }

    auto create_stmt{std::get<CreateStmt>(stmt_var)};

    // validate col exists
    bool col_exists{};
    for (const auto& [k, v] : create_stmt.cols) {
      if (v.col_name == col_name) {
        col_exists = true;
        break;
      }
    }

    if (!col_exists) {
      throw std::runtime_error("col " + col_name + " doesn't exist");
    }

    // navigate to rootpage
    size_t header_offset{static_cast<size_t>(rootpage - 1 == 0 ? 100 : 8)};
    size_t page_offset{page_size * static_cast<size_t>(rootpage - 1)};
    size_t num_cells_offset{page_offset + 3};

    // get num cells
    std::array<uint8_t, 2> bytes{};
    db.seekg(num_cells_offset);
    db.read(reinterpret_cast<char*>(&bytes[0]), 2);
    auto num_cells{read_big_endian<uint16_t>(&bytes[0])};

    // process cells
    for (size_t i{}; i < static_cast<size_t>(num_cells); ++i) {
      size_t cell_ptrs_offset{page_offset + header_offset};
      db.seekg(cell_ptrs_offset + (2 * i));

      std::array<uint8_t, 2> curr_bytes{};
      db.read(reinterpret_cast<char*>(curr_bytes.data()), 2);

      // exit on 0 bytes
      if (!curr_bytes[0] && !curr_bytes[1]) {
        break;
      }

      // get cell pointer
      uint16_t cell_ptr{read_big_endian<uint16_t>(&curr_bytes[0])};

      // get cell_offset
      size_t cell_offset{page_offset + cell_ptr};

      // go to cell
      db.seekg(cell_offset);

      // get payload size
      uint8_t payload_size{};
      db.read(reinterpret_cast<char*>(&payload_size), 1);

      // get row id
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

      size_t stc_i{};
      for (const auto& [k, v] : create_stmt.cols) {
        const auto& stc{serial_type_codes[stc_i++]};
        std::string col_val{};
        if (stc == 0) {
          if (v.type == "int" && v.primary_key) {
            col_val = std::to_string(row_id);
          } else {
            col_val = "null";
          }
        } else if (stc % 2 != 0) {
          auto text_size{(stc - 13) / 2};
          col_val.reserve(text_size);
          for (size_t i{}; i < static_cast<size_t>(text_size); ++i) {
            char c{};
            db.read(&c, 1);
            col_val += c;
          }
        }

        if (k == col_name) {
          std::cout << col_val << '\n';
        }
      }
    }

    break;
  }

  return count;
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

  if (command == ".dbinfo") {
    std::ifstream db(database_file_path, std::ios::binary);
    if (!db) {
      std::cerr << "Failed to open the database file" << std::endl;
      return 1;
    }

    auto page_size{get_page_size(db)};
    auto table_count{get_table_count(db)};

    std::cout << "number of tables: " << table_count << '\n';
    std::cout << "database page size: " << page_size << '\n';
  } else if (command == ".tables") {
    std::ifstream db(database_file_path, std::ios::binary);
    auto table_count{get_table_count(db)};
    auto table_names{get_table_names(db, table_count)};
    for (size_t i{}; i < table_names.size(); ++i) {
      std::cout << table_names[i];
      if (i + 1 < table_names.size()) std::cout << ' ';
    }
    std::cout << '\n';
  } else {
    std::ifstream db(database_file_path, std::ios::binary);
    auto tokenizer{Tokenizer{command}};
    auto tokens{tokenizer.tokenize()};
    auto parser{Parser{tokens}};
    auto stmt{parser.parse()};

    if (std::holds_alternative<SelectAllStmt>(stmt)) {
      auto select_all_stmt{std::get<SelectAllStmt>(stmt)};
      auto table_name{select_all_stmt.name};
      auto row_count{get_all_row_count(db, table_name)};
      std::cout << row_count << '\n';
    } else if (std::holds_alternative<SelectColStmt>(stmt)) {
      auto select_col_stmt{std::get<SelectColStmt>(stmt)};
      auto col_name{select_col_stmt.col_name};
      auto table_name{select_col_stmt.table_name};
      auto row_count{get_col_row(db, col_name, table_name)};
    }
  }

  return 0;
}
