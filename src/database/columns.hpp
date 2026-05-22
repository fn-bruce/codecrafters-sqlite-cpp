#ifndef INCLUDE_DATABASE_COLUMNS_HPP_
#define INCLUDE_DATABASE_COLUMNS_HPP_

#include <algorithm>
#include <expected>
#include <iostream>
#include <variant>
#include <vector>

#include "../parser/parser.hpp"
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

  void print(const std::vector<std::string>& col_names,
             std::optional<WhereClause> clause_res) const {

    if (clause_res) {
      auto clause{clause_res.value()};
      auto it{std::find_if(begin(), end(), [&clause](const Column& c) {
        if (c.key() != clause.col_name) {
          return false;
        }

        if (std::holds_alternative<std::string>(c.value())) {
          if (clause.op == ComparisonOp::EQUALS) {
            return std::get<std::string>(c.value()) == clause.col_val;
          } else {
            return std::get<std::string>(c.value()) != clause.col_val;
          }
        }

        return false;
      })};
      if (it == end()) {
        return;
      }
    }

    size_t count{};
    for (const auto& cn : col_names) {
      auto it{std::find_if(begin(), end(),
                           [&cn](const Column& c) { return cn == c.key(); })};
      if (it == end()) {
        continue;
      }

      if (std::holds_alternative<std::string>(it->value())) {
        std::cout << std::get<std::string>(it->value());
      } else if (std::holds_alternative<uint64_t>(it->value())) {
        std::cout << std::get<uint64_t>(it->value());
      }

      if (++count < col_names.size()) {
        std::cout << "|";
      }
    };
    std::cout << '\n';
  }
};

#endif // INCLUDE_DATABASE_COLUMNS_HPP_
