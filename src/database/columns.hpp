#ifndef INCLUDE_DATABASE_COLUMNS_HPP_
#define INCLUDE_DATABASE_COLUMNS_HPP_

#include <expected>
#include <iostream>
#include <variant>
#include <vector>

#include "column.hpp"

class Columns final : public std::vector<Column> {
  public:
  enum class Error {
    SizeMismatch,
    CreateError,
  };

  static std::expected<Columns, Error>
  create(const std::vector<std::string>& names,
         const std::vector<std::string>& vals) {
    if (names.size() != vals.size()) {
      return std::unexpected(Error::SizeMismatch);
    }

    size_t size{names.size()};

    Columns columns{};
    columns.reserve(size);

    for (size_t i{}; i < size; ++i) {
      auto key{names[i]};
      auto val{vals[i]};
      auto col{Column::create(key, val)};
      if (!col) {
        return std::unexpected(Error::CreateError);
      }
      columns.emplace_back(col.value());
    }

    return columns;
  }

  void print() const {
    size_t count{};
    for (const auto& c : *this) {
      if (std::holds_alternative<std::string>(c.value())) {
        std::cout << std::get<std::string>(c.value());
      } else if (std::holds_alternative<uint64_t>(c.value())) {
        std::cout << std::get<uint64_t>(c.value());
      }

      if (++count < size()) {
        std::cout << "|";
      }
    }
    std::cout << '\n';
  }

  void print(std::string_view col_name) const {
    for (const auto& c : *this) {
      if (col_name != c.key()) {
        continue;
      }

      if (std::holds_alternative<std::string>(c.value())) {
        std::cout << std::get<std::string>(c.value());
      } else if (std::holds_alternative<uint64_t>(c.value())) {
        std::cout << std::get<uint64_t>(c.value());
      }
    }
    std::cout << '\n';
  }
};

#endif // INCLUDE_DATABASE_COLUMNS_HPP_
