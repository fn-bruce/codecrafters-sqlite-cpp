#ifndef INCLUDE_SRC_TOKEN_HPP_
#define INCLUDE_SRC_TOKEN_HPP_

#include <string>
#include <vector>

#include "token_type.hpp"

struct Token;

using Tokens = std::vector<Token>;

struct Token {
  std::string name;
  TokenType type;
};

#endif  // INCLUDE_SRC_TOKEN_HPP_
