#ifndef INCLUDE_DATABASE_TABLE_HPP_
#define INCLUDE_DATABASE_TABLE_HPP_

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "rows.hpp"

class Table {
  public:
  static Table create(std::string_view tbl_name,
                      const std::vector<std::string>& col_names,
                      const std::vector<std::vector<std::string>>& row_vals) {
    return {
        tbl_name,
        col_names,
        row_vals,
    };
  }

  std::string_view tbl_name() const { return tbl_name_; }

  size_t row_count() const { return rows_.size(); }

  void print() const { rows_.print(); }

  void print(std::string_view col_name) const { rows_.print(col_name); }

  private:
  Table(std::string_view tbl_name, const std::vector<std::string>& col_names,
        const std::vector<std::vector<std::string>>& row_vals)
      : tbl_name_{tbl_name}, rows_{init_rows(col_names, row_vals)} {}

  const std::string tbl_name_{};
  const Rows rows_;

  Rows init_rows(const std::vector<std::string>& col_names,
                 const std::vector<std::vector<std::string>> row_vals) {
    auto result{Rows::create(col_names, row_vals)};
    if (!result) {
      switch (result.error()) {
      case Rows::Error::NamesEmptyError:
        throw std::runtime_error("names empty error when creating rows");
      case Rows::Error::ValsEmptyError:
        throw std::runtime_error("vals empty error when creating rows");
      case Rows::Error::SizeMismatchError:
        throw std::runtime_error("size mismatch when creating rows");
      case Rows::Error::CreateError:
        throw std::runtime_error("create when creating rows");
      }
    }
    return result.value();
  }
};

#endif // INCLUDE_DATABASE_TABLE_HPP_
