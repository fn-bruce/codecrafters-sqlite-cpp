#ifndef INCLUDE_SRC_UTILS_HPP_
#define INCLUDE_SRC_UTILS_HPP_

#include <cstdint>
#include <fstream>

template <typename T>
inline T read_big_endian(const uint8_t* data) {
  T result{};
  for (size_t i{}; i < sizeof(T); ++i) {
    result = (result << 8) | data[i];
  }
  return result;
}

template <typename T>
inline T read(std::ifstream& db) {
  std::array<uint8_t, sizeof(T)> buf{};
  db.read(reinterpret_cast<char*>(&buf.front()), sizeof(T));
  return read_big_endian<T>(&buf.front());
}

inline std::pair<uint64_t, int> read_varint(std::ifstream& db) {
  constexpr uint8_t CONTINUATION_BIT{0b1000'0000};
  constexpr uint8_t MASK{0b0111'1111};
  uint64_t result{};
  int bytes_read{};
  uint8_t curr_byte{};
  do {
    db.read(reinterpret_cast<char*>(&curr_byte), 1);
    result = (result << 7) | (curr_byte & MASK);
    ++bytes_read;
  } while (curr_byte & CONTINUATION_BIT);
  return {result, bytes_read};
}

#endif // INCLUDE_SRC_UTILS_HPP_
