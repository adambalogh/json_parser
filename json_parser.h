#pragma once

#include <string>
#include <unordered_map>

#include "json_value.h"

// Json parser using specification from http://json.org/
//
// Current implementation doesn't tolerate any whitespace.
//
class JsonParser {
 public:
  JsonParser(const char* p, const char* end) : p_(p), end_(end) {}

  JsonParser(const std::string& json)
      : JsonParser(&json[0], &json[0] + json.size()) {}

  JsonValue Parse();

 private:
  JsonValue ParseValue();
  JsonValue::ObjectType ParseObject();
  JsonValue::ArrayType ParseArray();
  JsonValue::StringType ParseString();
  JsonValue::NumberType ParseNumber();
  JsonValue::BoolType ParseBool();

  inline void SkipWhitespace();
  inline void Find(const char val);
  inline bool Match(const std::string& val);

  const char* p_;
  const char* const end_;
};
