#ifndef INCLUDE_SRC_PARSER_HPP_
#define INCLUDE_SRC_PARSER_HPP_

#include <any>
#include <optional>
#include <stdexcept>

#include "token.hpp"
#include "token_type.hpp"

struct Expr;

using ExprList = std::vector<Expr>;

struct SelectStmt {
  std::string name{};
};

class Parser {
 public:
  Parser(Tokens& tokens) : tokens_{std::move(tokens)} {}

  SelectStmt parse() {
    if (tokens_.empty()) return {};
    if (tokens_[curr_].type == TokenType::SELECT) {
      return select_statement();
    }
		throw std::runtime_error("issue parsing");
  }

 private:
  Tokens tokens_{};
  size_t curr_{};

  SelectStmt select_statement() {
    expect(TokenType::SELECT);
    expect(TokenType::COUNT_ALL);
    expect(TokenType::FROM);
    auto name{table_name()};
    expect(TokenType::END);
    return SelectStmt{
        .name = name,
    };
  }

  std::string table_name() { return expect(TokenType::IDENTIFIER).name; }

  const Token& peek() const { return tokens_.at(curr_); };
  const Token& advance() { return tokens_.at(curr_++); };

  const Token& expect(TokenType type) {
    if (peek().type != type) {
      throw std::runtime_error("unexpected token");
    }
    return advance();
  }
};

#endif  // INCLUDE_SRC_PARSER_HPP_
