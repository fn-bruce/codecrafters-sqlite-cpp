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
struct SelectAllStmt;
struct CreateStmt;

using Stmt = std::variant<SelectColsStmt, SelectAllStmt, CreateStmt>;

enum class ComparisonOp {
  EQUALS,
  NOT_EQUALS,
};

struct WhereClause {
  std::string col_name{};
  std::string col_val{};
  ComparisonOp op{};
};

struct SelectColsStmt {
  std::vector<std::string> col_names{};
  std::string table_name{};
  std::optional<WhereClause> where_clause{};
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

    if (TokenType::IDENTIFIER == peek().type) {
      return select_cols_statement();
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

    std::optional<WhereClause> where_clause{};
    if (peek().type == TokenType::WHERE) {
      expect(TokenType::WHERE);
      auto col_name{identifier()};
      auto op{comparision()};
      auto col_val{value()};
      where_clause = WhereClause{
          .col_name = col_name,
          .col_val = col_val,
          .op = op,
      };
    }

    expect(TokenType::END);
    return SelectColsStmt{
        .col_names = col_names,
        .table_name = table_name,
        .where_clause = where_clause,
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

  ComparisonOp comparision() {
    if (peek().type == TokenType::EQUALS) {
      advance();
      return ComparisonOp::EQUALS;
    } else if (peek().type == TokenType::NOT_EQUALS) {
      advance();
      return ComparisonOp::NOT_EQUALS;
    }
    throw std::runtime_error("parse comparison error");
  }

  std::string value() {
    if (peek().type == TokenType::STRING || peek().type == TokenType::NUMBER) {
      return advance().name;
    }
    throw std::runtime_error("parse value error of " + peek().name);
  }

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
      throw std::runtime_error("unexpected token of " + peek().name);
    }
    return advance();
  }
};

#endif // INCLUDE_SRC_PARSER_HPP_
