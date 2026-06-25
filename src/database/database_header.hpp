#ifndef INCLUDE_SRC_DATABASE_HEADER_HPP_
#define INCLUDE_SRC_DATABASE_HEADER_HPP_

#include <cstdint>
#include <array>

#include "utils.hpp"

class DatabaseHeader {
public:
  DatabaseHeader(std::ifstream& db);

  uint16_t page_size() const { return page_size_; }
  uint32_t page_count() const { return database_file_size_in_pages_; }

  void print() const;

private:
  std::ifstream& db_;

  std::string header_string_{};
  uint16_t page_size_{};
  uint8_t write_version_{};
  uint8_t read_version_{};
  uint8_t unused_reserved_space_{};
  uint8_t max_embedded_payload_fraction_{};
  uint8_t min_embedded_payload_fraction_{};
  uint8_t leaf_payload_fraction_{};
  uint32_t file_change_counter_{};
  uint32_t database_file_size_in_pages_{};
  uint32_t freelist_trunk_page_{};
  uint32_t num_of_freelist_pages_{};
  uint32_t schema_cookie_{};
  uint32_t schema_format_num_{};
  uint32_t default_page_cache_size_{};
  uint32_t largest_root_page_num_{};
  uint32_t database_text_encoding_{};
  uint32_t user_version_{};
  uint32_t inc_vacuum_mode_{};
  uint32_t application_id_{};
  std::array<uint8_t, 20> reserved_for_expansion_{};
  uint32_t version_valid_for_num_{};
  uint32_t sqlite_version_num_{};

  void parse() {
    header_string_ = read_header_string();
    page_size_ = read<uint16_t>(db_);
    write_version_ = read<uint8_t>(db_);
    read_version_ = read<uint8_t>(db_);
    unused_reserved_space_ = read<uint8_t>(db_);
    max_embedded_payload_fraction_ = read<uint8_t>(db_);
    min_embedded_payload_fraction_ = read<uint8_t>(db_);
    leaf_payload_fraction_ = read<uint8_t>(db_);
    file_change_counter_ = read<uint32_t>(db_);
    database_file_size_in_pages_ = read<uint32_t>(db_);
    freelist_trunk_page_ = read<uint32_t>(db_);
    num_of_freelist_pages_ = read<uint32_t>(db_);
    schema_cookie_ = read<uint32_t>(db_);
    schema_format_num_ = read<uint32_t>(db_);
    default_page_cache_size_ = read<uint32_t>(db_);
    largest_root_page_num_ = read<uint32_t>(db_);
    database_text_encoding_ = read<uint32_t>(db_);
    user_version_ = read<uint32_t>(db_);
    inc_vacuum_mode_ = read<uint32_t>(db_);
    application_id_ = read<uint32_t>(db_);
  }

  std::string read_header_string() const;
};

#endif // INCLUDE_SRC_DATABASE_HEADER_HPP_
