#ifndef INCLUDE_SRC_PAGE_HPP_
#define INCLUDE_SRC_PAGE_HPP_

#include <fstream>

#include "cell.hpp"
#include "page_header.hpp"
#include "database_header.hpp"

// basically points to other interior/leaf pages
class TableInteriorPage;

class TableLeafPage;

using Page = std::variant<TableInteriorPage, TableLeafPage>;

class TableInteriorPage {
public:
  TableInteriorPage(std::ifstream& db);

  void print() const;

  uint16_t cell_count() const { return header_.num_of_cells(); }

  const std::vector<Page>& pages() const { return pages_; }

private:
  size_t offset_;
  PageHeader header_;
  std::vector<Page> pages_;
};

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

std::vector<Page> read_pages(DatabaseHeader &header, std::ifstream &db);
std::vector<Page> read_pages(PageHeader &header, std::ifstream &db);

#endif // INCLUDE_SRC_PAGE_HPP_
