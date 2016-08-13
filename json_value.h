#pragma once

#include <string>
#include <unordered_map>
#include <vector>

// TODO do this with templates
class JsonValue {
 public:
  enum Type {
    OBJECT,
    ARRAY,
    STRING,
    NUMBER,
  };

  using ObjectType = std::unordered_map<std::string, JsonValue>;
  using ArrayType = std::vector<JsonValue>;
  using StringType = std::string;
  using NumberType = double;

  JsonValue(const ObjectType obj) : type_(OBJECT), obj_(std::move(obj)) {}
  JsonValue(const ArrayType arr) : type_(ARRAY), arr_(std::move(arr)) {}
  JsonValue(const StringType str) : type_(STRING), str_(std::move(str)) {}
  JsonValue(const NumberType num) : type_(NUMBER), num_(std::move(num)) {}

  template <int Type>
  bool is() {
    return type_ == Type;
  }

  ObjectType getObject() { return obj_; }
  ArrayType getArray() { return arr_; }
  StringType getString() { return str_; }
  NumberType getNumber() { return num_; }

  std::string to_string() const {
    std::string out;
    switch (type_) {
      case OBJECT:
        out += "{";
        for (const auto& e : obj_) {
          out += e.first;
          out += ": ";
          out += e.second.to_string();
          out += ",";
        }
        out += "}";
        break;
      case ARRAY:
        out += "[";
        for (const auto& e : arr_) {
          out += e.to_string();
          out += ",";
        }
        out += "]";
        break;
      case STRING:
        out += str_;
        break;
      case NUMBER:
        out += std::to_string(num_);
        break;
    }
    return out;
  }

 private:
  const Type type_;

  ObjectType obj_;
  ArrayType arr_;
  StringType str_;
  NumberType num_;
};
