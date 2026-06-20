#ifndef INCLUDE_SRC_PAGE_HPP_
#define INCLUDE_SRC_PAGE_HPP_

#include <fstream>

#include "cell.hpp"
#include "page_header.hpp"
#include "database_header.hpp"

class TableLeafPage {
public:
  TableLeafPage(std::ifstream& db);

  void print() const;

  uint16_t cell_count() const { return header_.num_of_cells(); }

  const std::vector<TableLeafCell>& cells() const { return cells_; }

private:
  size_t offset_;
  PageHeader header_;
  std::vector<TableLeafCell> cells_;
};

std::vector<TableLeafPage> read_pages(DatabaseHeader& header, std::ifstream& db);

#endif // INCLUDE_SRC_PAGE_HPP_
