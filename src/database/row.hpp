#ifndef INCLUDE_DATABASE_ROW_HPP_
#define INCLUDE_DATABASE_ROW_HPP_

#include <expected>

#include "columns.hpp"

class Row {
  public:
  enum class Error {
    CreateError,
  };

  static std::expected<Row, Error>
  create(const std::vector<std::string>& names,
         const std::vector<std::string>& vals) {

    auto columns{Columns::create(names, vals)};
    if (!columns) {
      return std::unexpected(Error::CreateError);
    }

    return Row{columns.value()};
  }

  void print() const { columns_.print(); }

  void print(std::string_view col_name) const { columns_.print(col_name); }

  private:
  Row(const Columns& columns) : columns_{columns} {}

  Columns columns_{};
};

#endif // INCLUDE_DATABASE_ROW_HPP_
