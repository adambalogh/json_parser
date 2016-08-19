#pragma once

#include <string>
#include <unordered_map>

#include "json_value.h"

// Json parser using specification from http://json.org/
//
class JsonParser {
 public:
  JsonParser(const char* p, const char* end) : p_(p), end_(end) {}

  JsonParser(const std::string& json)
      : JsonParser(&json[0], &json[0] + json.size()) {}

  JsonValue Parse();

 private:
  // ControlTokens control the behaviour of the parser.
  //
  // E.g. on receiving an OBJECT_OPEN, it will start parsing
  // a json object.
  enum ControlToken {
    OBJECT_OPEN,   // start parsing object
    OBJECT_CLOSE,  // end parsing object
    ARRAY_OPEN,    // start parsing array
    ARRAY_CLOSE,   // end parsing array
    COMMA,         // continue parsing array or object
    STRING,        // start or end parsing a string
    COLON,         // parse value in object (e.g. key : value)
    BOOL,          // parse a bool
    NUMBER,        // parse number
  };

  JsonValue ParseValue(ControlToken tk);
  JsonValue ParseValue() { return ParseValue(GetNextControlToken()); }

  JsonValue::ObjectType ParseObject();
  JsonValue::ArrayType ParseArray();
  JsonValue::StringType ParseString();
  JsonValue::NumberType ParseNumber();
  JsonValue::BoolType ParseBool();

  ControlToken GetNextControlToken();

  inline void SkipWhitespace();
  inline void Find(const char val);

  // Tries to match the given string, starting from p_.
  // If successful, returns true, and sets p_ past the matched string.
  inline bool Match(const std::string& val);

  inline char GetChar() const { return *p_; }
  inline void NextChar() { ++p_; }

  const char* p_;
  const char* const end_;
};
