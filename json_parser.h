#pragma once

#include <string>
#include <unordered_map>

#include "json_value.h"

// Json parser using specification from http://json.org/
//
// Current implementation assumes that the given string's syntax is correct, and
// it only prints the values it finds. It doesn't tolerate any whitespace
// either.
//
class JsonParser {
 public:
  JsonParser(const char* p, const char* end) : p_(p), end_(end) {}

  JsonParser(const std::string& json)
      : JsonParser(&json[0], &json[json.size()]) {}

  JsonValue Parse();

 private:
  JsonValue ParseValue();
  JsonValue::ObjectType ParseObject();
  JsonValue::ArrayType ParseArray();
  JsonValue::StringType ParseString();
  JsonValue::NumberType ParseNumber();

  void SkipWhitespace();
  void Find(const char val);

  const char* p_;
  const char* const end_;
};
