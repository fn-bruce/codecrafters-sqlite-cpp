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

struct Record {
  Values values{};
};

inline Record read_record(std::ifstream &db) {
  // read header

  // read header size
  uint64_t header_size{read_varint(db).first};

  // read serial types
  std::vector<SerialType> serial_types{};
  serial_types.reserve(static_cast<size_t>(header_size - 1));
  int count{};
  while (count < header_size - 1) {
    const auto [result, bytes_read]{read_varint(db)};
    serial_types.emplace_back(SerialType{result});
    count += bytes_read;
  }

  // read values
  Values values{};
  for (const auto &t : serial_types) {
    Value value{};
    switch (t) {
    case SerialType::NIL:
      value = std::monostate{};
      break;
    case SerialType::BIT_8:
      value = read<int8_t>(db);
      break;
    case SerialType::BIT_16:
      value = read<int16_t>(db);
      break;
    case SerialType::BIT_24: {
      std::array<int8_t, 3> arr_24{};
      for (size_t i{}; i < arr_24.size(); ++i) {
        int8_t mask{static_cast<int8_t>(0b1111'1111)};
        int8_t curr{read<int8_t>(db)};
        int8_t byte{static_cast<int8_t>(curr | mask)};
        arr_24[i] = byte;
      }
      value = arr_24;
      break;
    }
    case SerialType::BIT_32:
      value = read<int32_t>(db);
      break;
    case SerialType::BIT_48: {
      std::array<int8_t, 5> arr_48{};
      for (size_t i{}; i < arr_48.size(); ++i) {
        int8_t mask{static_cast<int8_t>(0b1111'1111)};
        int8_t curr{read<int8_t>(db)};
        int8_t byte{static_cast<int8_t>(curr | mask)};
        arr_48[i] = byte;
      }
      value = arr_48;
      break;
    }
    case SerialType::BIT_64:
      value = read<int64_t>(db);
      break;
    case SerialType::FLOAT:
      // TODO: need to handle double in read function
      // value = read<double>(db);
      break;
    case SerialType::ZERO:
      break;
    case SerialType::ONE:
      break;
    case SerialType::RESERVED_1:
      break;
    case SerialType::RESERVED_2:
      break;
    default: {
      uint64_t other_value{static_cast<uint64_t>(t)};
      if (other_value >= 12 && other_value % 2 == 0) {
        const size_t size{static_cast<size_t>((other_value - 12) / 2)};
        std::vector<int8_t> blob{};
        blob.reserve(size);
        for (size_t i{}; i < size; ++i) {
          char c{};
          db.read(&c, 1);
          blob.emplace_back(c);
        }
        value = blob;
      } else if (other_value >= 13 && other_value % 2 == 1) {
        const size_t size{static_cast<size_t>((other_value - 13) / 2)};
        std::string str{};
        str.reserve(size);
        for (size_t i{}; i < size; ++i) {
          char c{};
          db.read(&c, 1);
          str += c;
        }
        value = str;
      }
      break;
    }
    }

    values.emplace_back(value);
  }
  return {
      .values = values,
  };
}

#endif // INCLUDE_SRC_RECORD_HPP_
