#ifndef INCLUDE_DATABASE_ROWS_HPP_
#define INCLUDE_DATABASE_ROWS_HPP_

#include <expected>
#include <vector>

#include "../parser/parser.hpp"
#include "row.hpp"

class Rows final : public std::vector<Row> {
  public:
  enum class Error {
    NamesEmptyError,
    ValsEmptyError,
    SizeMismatchError,
    CreateError,
  };

  static std::expected<Rows, Error>
  create(const std::vector<std::string>& names,
         const std::vector<std::vector<std::string>>& all_vals) {
    size_t size{names.size()};
    for (const auto& vals : all_vals) {
      if (size != vals.size()) {
        return std::unexpected(Error::SizeMismatchError);
      }
    }

    Rows rows{};
    for (const auto& vals : all_vals) {
      auto row{Row::create(names, vals)};
      if (!row) {
        return std::unexpected(Error::CreateError);
      }
      rows.emplace_back(row.value());
    }

    return rows;
  }

  void print() const {
    for (const auto& r : *this) {
      r.print();
    }
  }

  void print(std::string_view col_name) const {
    for (const auto& r : *this) {
      r.print(col_name);
    }
  }

  void print(const std::vector<std::string>& col_names,
             std::optional<WhereClause> clause) const {
    for (const auto& r : *this) {
      r.print(col_names, clause);
    }
  }
};

#endif // INCLUDE_DATABASE_ROWS_HPP_
