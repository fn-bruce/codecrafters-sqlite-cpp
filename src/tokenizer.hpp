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
      switch (c) {
        case 'S':
          select();
          break;
        case 'C':
          count_all();
          break;
        case 'F':
          from();
          break;
        case ';':
          tokens_.push_back({
              .name = std::string{c},
              .type = TokenType::SEMICOLON,
          });
          advance();
          break;
        case ' ':
          ++pos_;
          break;
        default: {
          if (std::isalpha(c)) {
            identifier();
            break;
          }
          throw std::runtime_error("issue tokenizing");
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

  void select() {
    if (pos_ + 6 >= query_.size()) {
      throw std::runtime_error("error tokenizing select");
    }

    std::string sub_str{query_.substr(pos_, 6)};
    if (sub_str != "SELECT") {
      throw std::runtime_error("error tokenizing select");
    }

    pos_ += 6;
    tokens_.push_back({
        .name = sub_str,
        .type = TokenType::SELECT,
    });
  }

  void from() {
    if (pos_ + 4 >= query_.size()) {
      throw std::runtime_error("error tokenizing select");
    }

    std::string sub_str{query_.substr(pos_, 4)};
    if (sub_str != "FROM") {
      throw std::runtime_error("error tokenizing select");
    }

    pos_ += 4;
    tokens_.push_back({
        .name = sub_str,
        .type = TokenType::FROM,
    });
  }

  void count_all() {
    if (pos_ + 8 >= query_.size()) {
      throw std::runtime_error("error tokenizing select");
    }

    std::string sub_str{query_.substr(pos_, 8)};
    if (sub_str != "COUNT(*)") {
      throw std::runtime_error("error tokenizing select");
    }

    pos_ += 8;
    tokens_.push_back({
        .name = sub_str,
        .type = TokenType::COUNT_ALL,
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
