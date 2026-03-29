#include <array>
#include <cstring>
#include <fstream>
#include <iostream>

template <typename T>
T read_big_endian(const uint8_t* data) {
  T result = 0;
  for (size_t i{}; i < sizeof(T); ++i) {
    result = (result << 8) | data[i];
  }
  return result;
}

int main(int argc, char* argv[]) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // You can use print statements as follows for debugging, they'll be visible
  // when running tests.
  std::cerr << "Logs from your program will appear here" << std::endl;

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

    std::array<uint8_t, 2> buf{};
    db.seekg(16);
    db.read(reinterpret_cast<char*>(buf.data()), 2);

    auto page_size = read_big_endian<uint16_t>(&buf[0]);
    auto page_count = read_big_endian<uint32_t>(&buf[28]);

    std::cout << "database page size: " << page_size << std::endl;
  }

  return 0;
}
