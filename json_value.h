#pragma once

#include <assert.h>
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

  explicit JsonValue(ObjectType obj) : type_(OBJECT), obj_(std::move(obj)) {}
  explicit JsonValue(ArrayType arr) : type_(ARRAY), arr_(std::move(arr)) {}
  explicit JsonValue(StringType str) : type_(STRING), str_(std::move(str)) {}
  explicit JsonValue(NumberType num) : type_(NUMBER), num_(std::move(num)) {}

  template <int Type>
  bool is() {
    return type_ == Type;
  }

  ObjectType getObject() {
    if (type_ != OBJECT) {
      throw std::runtime_error("not an object");
    }
    return obj_;
  }

  ArrayType getArray() {
    if (type_ != ARRAY) {
      throw std::runtime_error("not an array");
    }
    return arr_;
  }

  StringType getString() {
    if (type_ != STRING) {
      throw std::runtime_error("not a string");
    }
    return str_;
  }

  NumberType getNumber() {
    if (type_ != NUMBER) {
      throw std::runtime_error("not a number");
    }
    return num_;
  }

  operator ObjectType() { return getObject(); }
  operator ArrayType() { return getArray(); }
  operator StringType() { return getString(); }
  operator NumberType() { return getNumber(); }

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
