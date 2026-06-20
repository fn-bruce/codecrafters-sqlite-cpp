#include "database_header.hpp"

DatabaseHeader::DatabaseHeader(std::ifstream& db) : db_{db} {
  db.seekg(0);
  parse();
}

void DatabaseHeader::print() const {
  std::cout << "=== Database Header ====" << '\n';
  std::cout << '\n';
  std::cout << "Header String: " << header_string_ << '\n';
  std::cout << "Page Size: " << page_size_ << '\n';
  std::cout << "Write Version: " << static_cast<uint16_t>(write_version_)
            << '\n';
  std::cout << "Read Version: " << static_cast<uint16_t>(read_version_)
            << '\n';
  std::cout << "Unused Reserved Space: "
            << static_cast<uint16_t>(unused_reserved_space_) << '\n';
  std::cout << "Max Embedded Payload Fraction: "
            << static_cast<uint16_t>(max_embedded_payload_fraction_) << '\n';
  std::cout << "Min Embedded Payload Fraction: "
            << static_cast<uint16_t>(min_embedded_payload_fraction_) << '\n';
  std::cout << "Leaf Payload Fraction: "
            << static_cast<uint16_t>(leaf_payload_fraction_) << '\n';
  std::cout << "File Change Counter: " << file_change_counter_ << '\n';
  std::cout << "Database File Size In_pages: " << database_file_size_in_pages_
            << '\n';
  std::cout << "Freelist Trunk Page: " << freelist_trunk_page_ << '\n';
  std::cout << "Num Of Freelist Pages: " << num_of_freelist_pages_ << '\n';
  std::cout << "Schema Cookie: " << schema_cookie_ << '\n';
  std::cout << "Schema Format Num: " << schema_format_num_ << '\n';
  std::cout << "Default Page Cache Size: " << default_page_cache_size_
            << '\n';
  std::cout << "Largest Root Page Num: " << largest_root_page_num_ << '\n';
  std::cout << "Database Text Encoding: " << database_text_encoding_ << '\n';
  std::cout << "User Version: " << user_version_ << '\n';
  std::cout << "Inc Vacuum Mode: " << inc_vacuum_mode_ << '\n';
  std::cout << "Application Id: " << application_id_ << '\n';

  std::cout << '\n';
}

std::string DatabaseHeader::read_header_string() const {
  constexpr uint8_t HEADER_STR_OFFSET{0};
  constexpr uint8_t HEADER_STR_SIZE{16};

  db_.seekg(HEADER_STR_OFFSET);
  std::array<uint8_t, HEADER_STR_SIZE> header_str_buf{};
  db_.read(reinterpret_cast<char*>(&header_str_buf.front()), HEADER_STR_SIZE);
  return std::string(std::begin(header_str_buf), std::end(header_str_buf));
}
