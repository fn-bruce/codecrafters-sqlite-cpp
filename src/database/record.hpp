#ifndef INCLUDE_SRC_RECORD_HPP_
#define INCLUDE_SRC_RECORD_HPP_

#include <fstream>
#include <variant>
#include <vector>

#include "utils.hpp"

using Value = std::variant<std::monostate, int8_t, int16_t, std::array<int8_t, 3>, int32_t, std::array<int8_t, 5>, int64_t,
                           double, std::vector<int8_t>, std::string>;
using Values = std::vector<Value>;

struct Record {
  Values values{};
};

inline Record read_record(std::ifstream &db) {
  // read header

  // read header size
  uint64_t header_size{read_varint(db).first};

  // read serial types
  std::vector<uint64_t> serial_types{};
  serial_types.reserve(static_cast<size_t>(header_size - 1));
  int count{};
  while (count < header_size - 1) {
    const auto [result, bytes_read] {read_varint(db)};
    serial_types.emplace_back(result);
    count += bytes_read;
  }

  // read values
  Values values{};
  for (const auto &t : serial_types) {
    Value value{};
    if (t == 0) {
      value = std::monostate{};
    } else if (t == 1) {
      value = read<int8_t>(db);
    } else if (t == 2) {
      value = read<int16_t>(db);
    } else if (t == 3) {
      std::array<int8_t, 3> arr{};
      for (size_t i{}; i < arr.size(); ++i) {
        int8_t mask{static_cast<int8_t>(0b1111'1111)};
        int8_t curr{read<int8_t>(db)};
        int8_t byte{static_cast<int8_t>(curr | mask)};
        arr[i] = byte;
      }
      value = arr;
    } else if (t == 4) {
      value = read<int32_t>(db);
    } else if (t == 5) {
      std::array<int8_t, 5> arr{};
      for (size_t i{}; i < arr.size(); ++i) {
        int8_t mask{static_cast<int8_t>(0b1111'1111)};
        int8_t curr{read<int8_t>(db)};
        int8_t byte{static_cast<int8_t>(curr | mask)};
        arr[i] = byte;
      }
      value = arr;
    } else if (t == 6) {
      value = read<int64_t>(db);
    } else if (t == 7) {
      // value = Double{read<Double>(db)};
    } else if (t == 8 || t == 9 || t == 10 || t == 11) {
      // TODO: handle missing types
    } else if (t >= 12 && t % 2 == 0) {
      const size_t size{static_cast<size_t>((t - 12) / 2)};
      std::vector<int8_t> blob{};
      blob.reserve(size);
      for (size_t i{}; i < size; ++i) {
        char c{};
        db.read(&c, 1);
        blob.emplace_back(c);
      }
      value = blob;
    } else if (t >= 13 && t % 2 == 1) {
      const size_t size{static_cast<size_t>((t - 13) / 2)};
      std::string str{};
      str.reserve(size);
      for (size_t i{}; i < size; ++i) {
        char c{};
        db.read(&c, 1);
        str += c;
      }
      value = str;
    }
    values.emplace_back(value);
  }
  return {
      .values = values,
  };
}

#endif // INCLUDE_SRC_RECORD_HPP_
