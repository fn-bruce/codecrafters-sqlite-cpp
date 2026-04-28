#include <array>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

constexpr int DATABASE_HEADER_SIZE{100};
constexpr int PAGE_HEADER_SIZE{8};
constexpr int CELL_PTR_SIZE{2};

constexpr int DATABASE_HEADER_OFFSET{0};
constexpr int PAGE_SIZE_OFFSET{16};
constexpr int PAGE_HEADER_OFFSET{100};
constexpr int TABLE_COUNT_OFFSET{103};
constexpr int CELL_POINTER_OFFSET{108};

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
    // each cell pointer is a 2-byte offset from the page start
    int offset{CELL_POINTER_OFFSET + (CELL_PTR_SIZE * i)};
    db.seekg(offset);
    std::array<uint8_t, 2> offset_buf{};
    db.read(reinterpret_cast<char*>(offset_buf.data()), 2);
    auto cell_ptr_offset{read_big_endian<uint16_t>(&offset_buf[0])};

    // cell layout: [payload_size varint] [rowid varint] [record]
    db.seekg(cell_ptr_offset);
    auto [payload_size, ps_len]{read_varint(db)};
    auto [rowid, rowid_len]{read_varint(db)};

    // record header: [header_size varint] [serial_types...]
    auto [header_size, hs_len]{read_varint(db)};
    std::vector<uint64_t> serial_types{};
    int header_bytes_read{hs_len};
    while (header_bytes_read < header_size) {
      auto [serial_type, st_len] = read_varint(db);
      serial_types.push_back(serial_type);
      header_bytes_read += st_len;
    }

    // sqlite_schema columns: type, name, tbl_name, rootpage, sql
    int type_len = (serial_types[0] - 13) / 2;
    db.seekg(type_len, std::ios::cur);  // skip type

    int name_len = (serial_types[1] - 13) / 2;
    std::string name(name_len, '\0');
    db.read(name.data(), name_len);

    if (name != "sqlite_sequence") {
      names.push_back(name);
    }
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

  if (command == ".dbinfo") {
    std::ifstream db(database_file_path, std::ios::binary);
    if (!db) {
      std::cerr << "Failed to open the database file" << std::endl;
      return 1;
    }

    auto page_size{get_page_size(db)};
    auto table_count{get_table_count(db)};
    auto table_names{get_table_names(db, table_count)};

    std::cout << "database page size: " << page_size << '\n';
    std::cout << "number of tables: " << table_count << '\n';
    for (size_t i{}; i < table_names.size(); ++i) {
      std::cout << table_names[i];
      if (i + 1 < table_names.size()) std::cout << ' ';
    }
    std::cout << '\n';
  }

  return 0;
}
