#ifndef INCLUDE_DATABASE_COLUMN_HPP_
#define INCLUDE_DATABASE_COLUMN_HPP_

#include <expected>
#include <string>
#include <variant>

class Column final {
  public:
  using Key = std::string;
  using Value = std::variant<std::string, uint64_t>;

  enum class Error {
    EmptyKey,
  };

  static std::expected<Column, Error> create(const Key& key, Value value) {
    if (key.empty()) {
      return std::unexpected(Error::EmptyKey);
    }
    return Column{key, value};
  }

  const Key& key() const noexcept { return key_; }
  const Value& value() const noexcept { return value_; }

  private:
  Column(const Key& key, Value& value)
      : key_{key}, value_{value} {}

  Key key_{};
  Value value_{};
};

#endif // INCLUDE_DATABASE_COLUMN_HPP_
