#ifndef INCLUDE_SRC_RECORD_HPP_
#define INCLUDE_SRC_RECORD_HPP_

#include <fstream>
#include <iostream>
#include <variant>
#include <vector>

#include "utils.hpp"

using Null = std::monostate;
using Int8 = int8_t;
using Int16 = int16_t;
using Int24 = std::array<int8_t, 3>;
using Int32 = int32_t;
using Int48 = std::array<int8_t, 5>;
using Int64 = int64_t;
using Double = double;
using Blob = std::vector<int8_t>;
using String = std::string;
using Value = std::variant<Null, Int8, Int16, Int24, Int32, Int48, Int64,
                           Double, Blob, String>;
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
      value = Null{};
    } else if (t == 1) {
      value = Int8{read<Int8>(db)};
    } else if (t == 2) {
      value = Int16{read<Int16>(db)};
    } else if (t == 3) {
      Int24 arr{};
      for (size_t i{}; i < arr.size(); ++i) {
        Int8 mask{static_cast<Int8>(0b1111'1111)};
        Int8 curr{read<Int8>(db)};
        Int8 byte{static_cast<Int8>(curr | mask)};
        arr[i] = byte;
      }
      value = arr;
    } else if (t == 4) {
      value = Int32{read<Int32>(db)};
    } else if (t == 5) {
      Int48 arr{};
      for (size_t i{}; i < arr.size(); ++i) {
        Int8 mask{static_cast<Int8>(0b1111'1111)};
        Int8 curr{read<Int8>(db)};
        Int8 byte{static_cast<Int8>(curr | mask)};
        arr[i] = byte;
      }
      value = arr;
    } else if (t == 6) {
      value = Int64{read<Int64>(db)};
    } else if (t == 7) {
      // value = Double{read<Double>(db)};
    } else if (t == 8 || t == 9 || t == 10 || t == 11) {
      // TODO: handle missing types
    } else if (t >= 12 && t % 2 == 0) {
      const size_t size{static_cast<size_t>((t - 12) / 2)};
      Blob blob{};
      blob.reserve(size);
      for (size_t i{}; i < size; ++i) {
        char c{};
        db.read(&c, 1);
        blob.emplace_back(c);
      }
      value = blob;
    } else if (t >= 13 && t % 2 == 1) {
      const size_t size{static_cast<size_t>((t - 13) / 2)};
      String str{};
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
