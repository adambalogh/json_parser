#pragma once

#include <string>

class JsonParser {
 public:
  void Parse(const char* p, const char* end);

 private:
  const char* ParseValue(const char* p, const char* const end);
  const char* ParseObject(const char* p, const char* const end);
  const char* ParseArray(const char* p, const char* const end);
  const char* ParseString(const char* p, const char* const end);
  const char* ParseNumber(const char* p, const char* end);

  const char* SkipWhitespace(const char* p, const char* const end);
  const char* Find(const char* p, const char* const end, const char val);
};
