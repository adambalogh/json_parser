#pragma once

#include <string>
#include <unordered_map>

#include "json_value.h"

namespace jp {

// Json parser using specification from http://json.org/
//
class JsonParser {
 public:
  JsonParser(const char* p, const char* end) : p_(p), start_(p), end_(end) {}

  JsonParser(const std::string& json)
      : JsonParser(&json[0], &json[0] + json.size()) {}

  // Currently, the outermost value doesn't have to be an object, not as per the
  // specification
  JsonValue Parse();

 private:
  // A ControlToken controls the behaviour of the parser.
  //
  // E.g. on receiving an OBJECT_OPEN, it will start parsing
  // a json object.
  enum ControlToken {
    OBJECT_OPEN,   // start parsing object
    OBJECT_CLOSE,  // stop parsing object
    ARRAY_OPEN,    // start parsing array
    ARRAY_CLOSE,   // stop parsing array
    COMMA,         // continue parsing array or object
    STRING,        // parse a string
    COLON,         // parse value in object key-value pair
    BOOL,          // parse a bool
    NUMBER,        // parse number
    NULL_VALUE,    // parse a null
    INVALID
  };

  JsonValue ParseValue(const ControlToken tk);
  JsonValue ParseValue() { return ParseValue(GetNextControlToken()); }

  JsonValue::ObjectType ParseObject();
  JsonValue::ArrayType ParseArray();
  JsonValue::StringType ParseString();
  JsonValue::NumberType ParseNumber();
  JsonValue::BoolType ParseBool();
  JsonValue ParseNull();

  inline double ParseSimpleNumber();

  ControlToken GetNextControlToken();

  inline void SkipSpace();

  // Tries to match the given string, starting from p_.
  // If successful, returns true, and sets p_ past the matched string.
  inline bool Match(const std::string& val);

  inline void AdvanceChar() { ++p_; }

  // Returns how many characters are left from the input string, including the
  // character pointed to by p_
  inline size_t Capacity() const { return end_ - p_; }

  inline const char GetChar() const {
    assert(p_ < end_);  // TODO make this an exception
    return *p_;
  }

  std::string GetSurroundings() const;
  void Expect(const char c) const;
  void Expect(const ControlToken expected, const ControlToken actual) const;
  std::string ErrorMessageName(const ControlToken ct) const;

  const char* p_;
  const char* const start_;
  const char* const end_;
};
}
