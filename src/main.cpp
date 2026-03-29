#include <array>
#include <cstring>
#include <fstream>
#include <iostream>

int main(int argc, char *argv[]) {
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
    db.read(reinterpret_cast<char *>(buf.data()), 2);

    unsigned short page_size = (static_cast<unsigned char>(buf[1]) |
                                (static_cast<unsigned char>(buf[0]) << 8));

    std::cout << "database page size: " << page_size << std::endl;
  }

  return 0;
}
