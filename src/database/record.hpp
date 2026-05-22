#ifndef INCLUDE_SRC_RECORD_HPP_
#define INCLUDE_SRC_RECORD_HPP_

#include <fstream>
#include <iostream>
#include <vector>

#include "utils.hpp"

class Record {
  public:
  static Record create(std::ifstream& db) { return {db}; }

  const std::vector<std::string>& vals() const noexcept { return vals_; }

  void print() const {
    std::cout << "=== Record ===\n";

    std::cout << "Header Size: " << header_size_ << '\n';

    std::cout << "Serial Types: " << '\n';
    for (const auto& t : serial_types_) {
      std::cout << "- " << t << '\n';
    }

    std::cout << "Column Values: " << '\n';
    for (const auto& v : vals_) {
      std::cout << "- " << v << '\n';
    }

    std::cout << '\n';
  }

  private:
  Record(std::ifstream& db)
      : header_size_{read_varint(db).first},
        serial_types_{read_serial_types(db)}, vals_{read_vals(db)} {}

  uint64_t header_size_{};
  std::vector<uint64_t> serial_types_{};
  std::vector<std::string> vals_{};

  std::vector<uint64_t> read_serial_types(std::ifstream& db) {
    // get serial types
    std::vector<uint64_t> serial_type_codes{};
    serial_type_codes.reserve(static_cast<size_t>(header_size_ - 1));

    size_t record_count{1};
    while (record_count < static_cast<size_t>(header_size_)) {
      auto [result, bytes_read] = read_varint(db);
      record_count += bytes_read;
      serial_type_codes.push_back(result);
    }
    return serial_type_codes;
  }

  std::vector<std::string> read_vals(std::ifstream& db) {
    std::vector<std::string> vals{};
    vals.reserve(serial_types_.size());
    for (const auto& t : serial_types_) {
      std::string val{};
      if (t == 0) {
        val = "null";
      } else if (t == 1) {
        uint8_t num{read<uint8_t>(db)};
        val = std::to_string(num);
      } else if (t % 2 != 0) {
        auto text_size{(t - 13) / 2};
        val.reserve(text_size);
        for (size_t i{}; i < static_cast<size_t>(text_size); ++i) {
          char c{};
          db.read(&c, 1);
          val += c;
        }
      }
      vals.push_back(val);
    }
    return vals;
  }
};

#endif // INCLUDE_SRC_RECORD_HPP_
