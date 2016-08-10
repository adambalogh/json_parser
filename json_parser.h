#pragma once

#include <string>

// Json parser using specification from http://json.org/
//
// Current implementation assumes that the given string's syntax is correct, and
// it only prints the values it finds. It doesn't tolerate any whitespace
// either.
//
class JsonParser {
 public:
  void Parse(const char* p, const char* end);
  void Parse(const std::string& json) { Parse(&json[0], &json[json.size()]); }

 private:
  const char* ParseValue(const char* p, const char* const end);
  const char* ParseObject(const char* p, const char* const end);
  const char* ParseArray(const char* p, const char* const end);
  const char* ParseString(const char* p, const char* const end);
  const char* ParseNumber(const char* p, const char* const end);

  const char* SkipWhitespace(const char* p, const char* const end);
  const char* Find(const char* p, const char* const end, const char val);
};
