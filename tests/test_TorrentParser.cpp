
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

class BencodeValue {
 public:
  // Define the variant type that can hold int64_t, string, vector (for lists),
  // or map (for dicts)
  using ValueType =
      std::variant<int64_t, std::string, std::vector<BencodeValue>,
                   std::map<std::string, BencodeValue>>;

  // Constructors for different types
  BencodeValue() {};
  BencodeValue(int64_t val) : value(val) {}
  BencodeValue(const std::string& str) : value(str) {}
  BencodeValue(const std::vector<BencodeValue>& list) : value(list) {}
  BencodeValue(const std::map<std::string, BencodeValue>& dict) : value(dict) {}

  // Encoding function
  std::string encode() const { return encodeHelper(value); }

  // Decoding function (this example is a simplified version, assumes valid
  // input)
  static BencodeValue decode(const std::string& str) {
    return decodeHelper(str, 0).first;
  }

 private:
  // The variant value
  ValueType value;

  // Helper function for encoding
  std::string encodeHelper(const ValueType& val) const {
    if (std::holds_alternative<int64_t>(val)) {
      return std::to_string(std::get<int64_t>(val));
    }
    if (std::holds_alternative<std::string>(val)) {
      const auto& str = std::get<std::string>(val);
      return std::to_string(str.size()) + ":" + str;
    }
    if (std::holds_alternative<std::vector<BencodeValue>>(val)) {
      const auto& list = std::get<std::vector<BencodeValue>>(val);
      std::string result = "l";
      for (const auto& item : list) {
        result += item.encode();
      }
      return result + "e";
    }
    if (std::holds_alternative<std::map<std::string, BencodeValue>>(val)) {
      const auto& dict = std::get<std::map<std::string, BencodeValue>>(val);
      std::string result = "d";
      for (const auto& pair : dict) {
        result += std::to_string(pair.first.size()) + ":" + pair.first +
                  pair.second.encode();
      }
      return result + "e";
    }
    throw std::runtime_error("Unsupported type in bencode encoding");
  }

  // Helper function for decoding
  static std::pair<BencodeValue, size_t> decodeHelper(const std::string& str,
                                                      size_t pos) {
    if (str[pos] == 'd') {
      std::map<std::string, BencodeValue> dict;
      pos++;
      while (str[pos] != 'e') {
        // Read key
        size_t key_len = std::stoi(str.substr(pos, str.find(':', pos) - pos));
        pos = str.find(':', pos) + 1;
        std::string key = str.substr(pos, key_len);
        pos += key_len;

        // Read value
        auto [value, new_pos] = decodeHelper(str, pos);
        dict[key] = value;
        pos = new_pos;
      }
      return {BencodeValue(dict), pos + 1};  // Skip 'e'
    }
    if (str[pos] == 'l') {
      std::vector<BencodeValue> list;
      pos++;
      while (str[pos] != 'e') {
        auto [value, new_pos] = decodeHelper(str, pos);
        list.push_back(value);
        pos = new_pos;
      }
      return {BencodeValue(list), pos + 1};  // Skip 'e'
    }
    if (std::isdigit(str[pos])) {
      size_t len = std::stoi(str.substr(pos, str.find(':', pos) - pos));
      pos = str.find(':', pos) + 1;
      std::string value = str.substr(pos, len);
      return {BencodeValue(value), pos + len};
    }
    if (str[pos] == '-') {
      size_t end_pos = str.find(':', pos);
      int64_t value = std::stoll(str.substr(pos, end_pos - pos));
      pos = end_pos + 1;
      return {BencodeValue(value), pos};
    }
    throw std::runtime_error("Invalid bencode string format");
  }
};

class Bencoder {
 public:
  BencodeValue decode(const std::string str){
    if(str[0]=='i'){
      if(str.size()<2 || str.back()!='e') throw std::invalid_argument("Invalid Bencode string format");
      return BencodeValue(std::stoll(str.substr(1,str.size()-2)));
    }
    if(str[0]=='d'){
      if(str.size()<2 || str.back()!='e') throw std::invalid_argument("Invalid Bencode string format");
      return decodeDict(str.substr(1,str.size()-2));
    }
    if(str[0]=='l'){
      if(str.size()<2 || str.back()!='e') throw std::invalid_argument("Invalid Bencode string format");
      return decodeList(str.substr(1,str.size()-2));
    }
    if(std::isdigit(str[0])){
      size_t colon_pos = str.find(':');
      if(colon_pos==std::string::npos) throw std::invalid_argument("Invalid Bencode string format");
      int64_t length = std::stoll(str.substr(0,colon_pos));
      if(colon_pos+1+length>str.size()) throw std::invalid_argument("String length exceeds available data");
      return BencodeValue(str.substr(colon_pos+1,length));
    }
    throw std::invalid_argument("Invalid Bencode string format");
  }
  BencodeValue decodeDict(const std::string & val);
};

// Example Usage
int main() { return 0; };
