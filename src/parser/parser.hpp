#ifndef INCLUDE_SRC_PARSER_HPP_
#define INCLUDE_SRC_PARSER_HPP_

#include <iostream>
#include <optional>
#include <stdexcept>
#include <utility>
#include <variant>

#include "token.hpp"
#include "token_type.hpp"

struct SelectColsStmt;
struct SelectColStmt;
struct SelectAllStmt;
struct CreateStmt;

using Stmt =
    std::variant<SelectColsStmt, SelectColStmt, SelectAllStmt, CreateStmt>;

struct SelectColStmt {
  std::string col_name{};
  std::string table_name{};
};

struct SelectColsStmt {
  std::vector<std::string> col_names{};
  std::string table_name{};
};

struct SelectAllStmt {
  std::string name{};
};

struct CreateCol {
  std::string col_name{};
  std::string type{};
  bool primary_key{};
  bool autoincrement{};
};

struct CreateStmt {
  std::string table_name{};
  std::vector<std::pair<std::string, CreateCol>> cols{};
};

class Parser {
  public:
  Parser(Tokens& tokens) : tokens_{std::move(tokens)} {}

  Stmt parse() {
    if (tokens_.empty())
      return {};

    if (tokens_[curr_].type == TokenType::SELECT) {
      return select_statement();
    }

    if (tokens_[curr_].type == TokenType::CREATE) {
      return create_statement();
    }

    throw std::runtime_error("error parsing");
  }

  private:
  Tokens tokens_{};
  size_t curr_{};

  Stmt select_statement() {
    expect(TokenType::SELECT);

    if (TokenType::COUNT_ALL == peek().type) {
      return select_all_statement();
    }

    if (TokenType::IDENTIFIER == peek().type && next().has_value() &&
        TokenType::COMMA == next().value().type) {
      return select_cols_statement();
    }

    if (TokenType::IDENTIFIER == peek().type) {
      return select_col_statement();
    }

    throw std::runtime_error("error parsing select statement");
  }

  SelectAllStmt select_all_statement() {
    expect(TokenType::COUNT_ALL);
    expect(TokenType::FROM);
    auto name{identifier()};
    expect(TokenType::END);
    return SelectAllStmt{
        .name = name,
    };
  }

  SelectColsStmt select_cols_statement() {
    std::vector<std::string> col_names{};
    while (TokenType::IDENTIFIER == peek().type) {
      auto col_name{identifier()};
      col_names.emplace_back(col_name);
      if (peek().type == TokenType::FROM) {
        break;
      }
      expect(TokenType::COMMA);
    }
    expect(TokenType::FROM);
    auto table_name{identifier()};
    expect(TokenType::END);
    return SelectColsStmt{
        .col_names = col_names,
        .table_name = table_name,
    };
  }

  SelectColStmt select_col_statement() {
    auto col_name{identifier()};
    expect(TokenType::FROM);
    auto table_name{identifier()};
    expect(TokenType::END);
    return SelectColStmt{
        .col_name = col_name,
        .table_name = table_name,
    };
  }

  CreateStmt create_statement() {
    expect(TokenType::CREATE);
    expect(TokenType::TABLE);
    auto table_name{identifier()};
    expect(TokenType::LPAREN);
    auto cols{create_cols()};
    expect(TokenType::RPAREN);

    return CreateStmt{
        .table_name = table_name,
        .cols = cols,
    };
  }

  std::vector<std::pair<std::string, CreateCol>> create_cols() {
    std::vector<std::pair<std::string, CreateCol>> cols{};
    while (true) {
      auto name{identifier()};
      auto type{col_type()};
      auto pk{primary_key()};
      auto a{autoincrement()};
      cols.push_back({
          name,
          {
              .col_name = name,
              .type = type,
              .primary_key = pk,
              .autoincrement = a,

          },
      });

      if (peek().type != TokenType::COMMA) {
        break;
      }

      expect(TokenType::COMMA);
    }

    if (!cols.size()) {
      throw std::runtime_error("create queries require columns");
    }

    return cols;
  }

  bool autoincrement() {
    if (peek().type == TokenType::AUTOINCREMENT) {
      advance();
      return true;
    }

    return false;
  }

  bool primary_key() {
    if (peek().type == TokenType::PRIMARY_KEY) {
      advance();
      return true;
    }

    return false;
  }

  std::string col_type() {
    if (peek().type == TokenType::INTEGER) {
      advance();
      return "int";
    }

    if (peek().type == TokenType::TEXT) {
      advance();
      return "text";
    }

    throw std::runtime_error("failed to parse type");
  }

  std::string identifier() { return expect(TokenType::IDENTIFIER).name; }

  const std::optional<Token> next() const {
    if (curr_ + 1 >= tokens_.size()) {
      return {};
    }
    return tokens_[curr_ + 1];
  }

  const Token& peek() const { return tokens_.at(curr_); };
  const Token& advance() { return tokens_.at(curr_++); };

  const Token& expect(TokenType type) {
    if (peek().type != type) {
      throw std::runtime_error("unexpected token");
    }
    return advance();
  }
};

#endif // INCLUDE_SRC_PARSER_HPP_
