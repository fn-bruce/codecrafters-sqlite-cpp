#include <cstring>
#include <ios>
#include <iostream>
#include <string>

#include "database/database.hpp"

int main(int argc, char *argv[]) {
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

  Database{database_file_path}.execute(command);

  return 0;
}
