#ifndef INCLUDE_SRC_RECORD_HPP_
#define INCLUDE_SRC_RECORD_HPP_

#include <fstream>
#include <variant>
#include <vector>

#include "utils.hpp"

using Value = std::variant<
  std::monostate,
  int8_t,
  int16_t,
  std::array<int8_t, 3>,
  int32_t,
  std::array<int8_t, 5>,
  int64_t,
  double,
  std::vector<int8_t>,
  std::string
>;

using Values = std::vector<Value>;

enum class SerialType : uint64_t {
  NIL = 0,
  BIT_8 = 1,
  BIT_16 = 2,
  BIT_24 = 3,
  BIT_32 = 4,
  BIT_48 = 5,
  BIT_64 = 6,
  FLOAT = 7,
  ZERO = 8,
  ONE = 9,
  RESERVED_1 = 10,
  RESERVED_2 = 11,
};

class Record {
public:
  Record(std::ifstream &db);

  const Values values() const { return values_; }
private:
  Values values_{};

  void read_record(std::ifstream &db);
};

#endif // INCLUDE_SRC_RECORD_HPP_
