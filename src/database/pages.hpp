#ifndef INCLUDE_SRC_PAGES_HPP_
#define INCLUDE_SRC_PAGES_HPP_

#include <vector>

#include "database_header.hpp"
#include "page.hpp"

class Pages : public std::vector<TableLeafPage> {
public:
  Pages() = default;
  Pages(DatabaseHeader& header, std::ifstream& db);

  void print() const;
};

#endif // INCLUDE_SRC_PAGES_HPP_
