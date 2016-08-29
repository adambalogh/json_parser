#pragma once

#include <assert.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace jp {

// TODO do this with templates
class JsonValue {
 public:
  enum Type { OBJECT, ARRAY, STRING, NUMBER, BOOL, NULL_VALUE };

  using ObjectType = std::unordered_map<std::string, JsonValue>;
  using ArrayType = std::vector<JsonValue>;
  using StringType = std::string;
  using NumberType = double;
  using BoolType = bool;

  explicit JsonValue(ObjectType obj) : type_(OBJECT), obj_(std::move(obj)) {}
  explicit JsonValue(ArrayType arr) : type_(ARRAY), arr_(std::move(arr)) {}
  explicit JsonValue(StringType str) : type_(STRING), str_(std::move(str)) {}
  explicit JsonValue(NumberType num) : type_(NUMBER), num_(std::move(num)) {}
  explicit JsonValue(BoolType val) : type_(BOOL), bool_(val) {}
  explicit JsonValue() : type_(NULL_VALUE) {}

  template <int Type>
  inline bool is() const {
    return type_ == Type;
  }

  const ObjectType& getObject() const {
    if (type_ != OBJECT) {
      throw std::runtime_error("not an object");
    }
    return obj_;
  }

  const ArrayType& getArray() const {
    if (type_ != ARRAY) {
      throw std::runtime_error("not an array");
    }
    return arr_;
  }

  const StringType& getString() const {
    if (type_ != STRING) {
      throw std::runtime_error("not a string");
    }
    return str_;
  }

  NumberType getNumber() const {
    if (type_ != NUMBER) {
      throw std::runtime_error("not a number");
    }
    return num_;
  }

  BoolType getBool() const {
    if (type_ != BOOL) {
      throw std::runtime_error("not a bool");
    }
    return bool_;
  }

  operator const ObjectType&() const { return getObject(); }
  operator const ArrayType&() const { return getArray(); }
  operator const StringType&() const { return getString(); }
  operator NumberType() const { return getNumber(); }
  operator BoolType() const { return getBool(); }

  // Should only be used for debugging
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
        out += "\"" + str_ + "\"";
        break;
      case NUMBER:
        out += std::to_string(num_);
        break;
      case BOOL:
        out += bool_ ? "true" : "false";
        break;
      case NULL_VALUE:
        out += "null";
    }
    return out;
  }

 private:
  const Type type_;

  // TODO use an union here
  ObjectType obj_;
  ArrayType arr_;
  StringType str_;
  NumberType num_;
  BoolType bool_;
};
}
