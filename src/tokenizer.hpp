#ifndef INCLUDE_SRC_TOKENIZER_HPP_
#define INCLUDE_SRC_TOKENIZER_HPP_

#include <cassert>
#include <iostream>
#include <optional>
#include <stdexcept>

#include "token.hpp"

class Tokenizer {
 public:
  Tokenizer(std::string_view query) : query_{query} {}

  Tokens tokenize() {
    while (!is_end()) {
      assert(curr().has_value());
      char c{curr().value()};
      if (is_next("autoincrement")) {
        autoincrement();
      } else if (is_next("COUNT(*)")) {
        count_all();
      } else if (is_next("CREATE")) {
        create();
      } else if (is_next("FROM")) {
        from();
      } else if (is_next("integer")) {
        integer();
      } else if (is_next("primary key")) {
        primary_key();
      } else if (is_next("SELECT")) {
        select();
      } else if (is_next("text")) {
        text();
      } else if (is_next("TABLE")) {
        table();
      } else {
        switch (c) {
          case ',':
            comma();
            break;
          case '(':
            lparen();
            break;
          case ')':
            rparen();
            break;
          case '\n':
          case '\r':
          case '\t':
          case ' ':
            ++pos_;
            break;
          default: {
            if (std::isalpha(c)) {
              identifier();
              break;
            }
            std::cout << c << '\n';
            throw std::runtime_error("issue tokenizing");
          }
        }
      }
    }

    tokens_.push_back({.name = "END", .type = TokenType::END});

    return tokens_;
  }

 private:
  Tokens tokens_{};
  const std::string query_{};
  size_t pos_{};

  bool is_next(std::string_view next) {
    if (pos_ + next.size() >= query_.size()) {
      return false;
    }

    auto sub_str{query_.substr(pos_, next.size())};
    return next == sub_str;
  }

  bool is_end() const { return pos_ >= query_.size(); }

  bool has_next() const { return pos_ + 1 >= query_.size(); }

  std::optional<char> curr() {
    if (!is_end()) {
      return query_[pos_];
    }
    return {};
  }

  std::optional<char> advance() {
    if (has_next()) {
      return query_[++pos_];
    }
    return {};
  }

  void keyword(std::string_view keyword, TokenType type) {
    if (pos_ + keyword.size() >= query_.size()) {
      throw std::runtime_error("error tokenizing select");
    }

    std::string sub_str{query_.substr(pos_, keyword.size())};
    if (sub_str != keyword) {
      throw std::runtime_error("error tokenizing select");
    }

    pos_ += keyword.size();
    tokens_.push_back({
        .name = sub_str,
        .type = type,
    });
  }

  void text() { keyword("text", TokenType::TEXT); }

  void autoincrement() { keyword("autoincrement", TokenType::AUTOINCREMENT); }

  void primary_key() { keyword("primary key", TokenType::PRIMARY_KEY); }

  void integer() { keyword("integer", TokenType::INTEGER); }

  void table() { keyword("TABLE", TokenType::TABLE); }

  void select() { keyword("SELECT", TokenType::SELECT); }

  void from() { keyword("FROM", TokenType::FROM); }

  void create() { keyword("CREATE", TokenType::CREATE); }

  void count_all() { keyword("COUNT(*)", TokenType::COUNT_ALL); }

  void lparen() {
    ++pos_;
    tokens_.push_back({
        .name = "(",
        .type = TokenType::LPAREN,
    });
  }

  void rparen() {
    ++pos_;
    tokens_.push_back({
        .name = ")",
        .type = TokenType::RPAREN,
    });
  }

  void comma() {
    ++pos_;
    tokens_.push_back({
        .name = ",",
        .type = TokenType::COMMA,
    });
  }

  void identifier() {
    size_t beg{pos_};
    while (curr().has_value() && std::isalpha(curr().value())) {
      ++pos_;
    }

    std::string identifier{query_.substr(beg, pos_ - beg)};
    tokens_.push_back({
        .name = identifier,
        .type = TokenType::IDENTIFIER,
    });
  }
};

#endif  // INCLUDE_SRC_TOKENIZER_HPP_
