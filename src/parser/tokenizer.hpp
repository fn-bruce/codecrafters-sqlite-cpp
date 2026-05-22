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
      } else if (is_next("COUNT(*)") || is_next("count(*)")) {
        count_all();
      } else if (is_next("CREATE")) {
        create();
      } else if (is_next("FROM") || is_next("from")) {
        from();
      } else if (is_next("WHERE") || is_next("where")) {
        where();
      } else if (is_next("integer")) {
        integer();
      } else if (is_next("primary key")) {
        primary_key();
      } else if (is_next("SELECT") || is_next("select")) {
        select();
      } else if (is_next("text")) {
        text();
      } else if (is_next("TABLE")) {
        table();
      } else {
        switch (c) {
        case '=':
          equals();
          break;
        case ',':
          comma();
          break;
        case '(':
          lparen();
          break;
        case ')':
          rparen();
          break;
        case '\'':
          string();
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

          if (std::isdigit(c)) {
            number();
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
    std::string upper{to_upper(sub_str)};
    if (upper != keyword) {
      throw std::runtime_error("error tokenizing select");
    }

    pos_ += keyword.size();
    tokens_.push_back({
        .name = upper,
        .type = type,
    });
  }

  void string() {
    if (query_[pos_++] != '\'') {
      throw std::runtime_error("error tokenizing string");
    }

    const size_t beg{pos_};
    while (std::isalpha(query_[pos_])) {
      ++pos_;
    }
    const std::string str{query_.substr(beg, pos_ - beg)};
    tokens_.push_back({
        .name = str,
        .type = TokenType::STRING,
    });

    if (query_[pos_++] != '\'') {
      throw std::runtime_error("error tokenizing string");
    }
  }

  void text() { keyword("TEXT", TokenType::TEXT); }

  void autoincrement() { keyword("AUTOINCREMENT", TokenType::AUTOINCREMENT); }

  void primary_key() { keyword("PRIMARY KEY", TokenType::PRIMARY_KEY); }

  void integer() { keyword("INTEGER", TokenType::INTEGER); }

  void table() { keyword("TABLE", TokenType::TABLE); }

  void select() { keyword("SELECT", TokenType::SELECT); }

  void from() { keyword("FROM", TokenType::FROM); }

  void where() { keyword("WHERE", TokenType::WHERE); }

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

  void equals() {
    ++pos_;
    tokens_.push_back({
        .name = "=",
        .type = TokenType::EQUALS,
    });
  }

  void number() {
    size_t beg{pos_};
    while (curr().has_value() && std::isdigit(curr().value())) {
      ++pos_;
    }

    std::string number{query_.substr(beg, pos_ - beg)};
    tokens_.push_back({
        .name = number,
        .type = TokenType::NUMBER,
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

  std::string to_upper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return s;
  }
};

#endif // INCLUDE_SRC_TOKENIZER_HPP_
