#ifndef TORRENT_PARSER_HPP
#define TORRENT_PARSER_HPP
#include <cstdint>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

// Class representing a Bencode value
class BencodeValue {
 public:
  using KeyType = std::variant<int64_t, std::string>;
  using ValueType =
      std::variant<int64_t, std::string, std::vector<BencodeValue>,
                   std::map<KeyType, BencodeValue>>;

  // Constructors
  BencodeValue() : value(0) {}  // Default initializes to int64_t with value 0
  BencodeValue(int64_t val) : value(val) {}
  BencodeValue(const std::string& str) : value(str) {}
  BencodeValue(const std::vector<BencodeValue>& list) : value(list) {}
  BencodeValue(const std::map<KeyType, BencodeValue>& dict) : value(dict) {}

  // Convert the BencodeValue to a Bencode-encoded string
  std::string toString() const {
    if (std::holds_alternative<int64_t>(value)) {
      return "i" + std::to_string(std::get<int64_t>(value)) + "e";
    } else if (std::holds_alternative<std::string>(value)) {
      const auto& str = std::get<std::string>(value);
      return std::to_string(str.size()) + ":" + str;
    } else if (std::holds_alternative<std::map<KeyType, BencodeValue>>(value)) {
      const auto& dict = std::get<std::map<KeyType, BencodeValue>>(value);
      std::string result = "d";
      for (const auto& [k, v] : dict) {
        if (std::holds_alternative<int64_t>(k)) {
          result += "i" + std::to_string(std::get<int64_t>(k)) + "e";
        } else {
          const auto& key_str = std::get<std::string>(k);
          result += std::to_string(key_str.size()) + ":" + key_str;
        }
        result += v.toString();
      }
      result += "e";
      return result;
    } else {
      const auto& list = std::get<std::vector<BencodeValue>>(value);
      std::string result = "l";
      for (const auto& item : list) {
        result += item.toString();
      }
      result += "e";
      return result;
    }
  }

  // Public member to access the variant value
  ValueType value;
};

// Class for Bencode encoding and decoding
class Bencoder {
 public:
  // Decode a Bencode string into a BencodeValue
  BencodeValue decode(const std::string& str) {
    this->str = str;
    ptr = 0;  // Reset the pointer
    return decodeHelper();
  }

  // Encode a BencodeValue into a Bencode string
  std::string encode(const BencodeValue& val) { return val.toString(); }

 private:
  BencodeValue decodeHelper() {
    if (str[ptr] == 'i') {  // Integer
      size_t end_pos = str.find('e', ptr);
      if (end_pos == std::string::npos)
        throw std::invalid_argument("Invalid Bencode integer format");
      int64_t result = std::stoll(str.substr(ptr + 1, end_pos - ptr - 1));
      ptr = end_pos + 1;
      return result;
    } else if (str[ptr] == 'd') {  // Dictionary
      ptr++;
      std::map<BencodeValue::KeyType, BencodeValue> dict;
      while (ptr < str.size() && str[ptr] != 'e') {
        BencodeValue key = decodeHelper();
        if (!std::holds_alternative<std::string>(key.value)) {
          throw std::invalid_argument("Invalid Bencode dictionary key type");
        }
        auto value = decodeHelper();
        dict[std::get<std::string>(key.value)] = value;
      }
      if (ptr == str.size())
        throw std::invalid_argument("Unterminated dictionary");
      ptr++;
      return dict;
    } else if (str[ptr] == 'l') {  // List
      ptr++;
      std::vector<BencodeValue> list;
      while (ptr < str.size() && str[ptr] != 'e') {
        list.push_back(decodeHelper());
      }
      if (ptr == str.size()) throw std::invalid_argument("Unterminated list");
      ptr++;
      return list;
    } else if (std::isdigit(str[ptr])) {  // String
      size_t colon_pos = str.find(':', ptr);
      if (colon_pos == std::string::npos)
        throw std::invalid_argument("Invalid Bencode string format");
      int64_t len = std::stoll(str.substr(ptr, colon_pos - ptr));
      ptr = colon_pos + 1;
      if (ptr + len > str.size())
        throw std::invalid_argument("String length exceeds data size");
      std::string result = str.substr(ptr, len);
      ptr += len;
      return result;
    }
    throw std::invalid_argument("Invalid Bencode format");
  }

  std::string str;
  size_t ptr = 0;  // Current parsing position
};

#endif
