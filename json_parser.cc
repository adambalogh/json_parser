#include "json_parser.h"

#include <assert.h>
#include <cctype>
#include <iostream>

const char kObjectOpen = '{';
const char kObjectClose = '}';
const char kArrayOpen = '[';
const char kArrayClose = ']';
const char kStringOpen = '"';
const char kStringClose = '"';
const char kDoubleQuote = '"';
const char kWhitespace = ' ';
const char kColon = ':';
const char kComma = ',';

void JsonParser::Parse(const char* p, const char* end) { ParseValue(p, end); }

const char* JsonParser::ParseValue(const char* p, const char* end) {
  if (*p == kObjectOpen) {
    p = ParseObject(p, end);
  } else if (*p == kArrayOpen) {
    p = ParseArray(p, end);
  } else if (*p == kStringOpen) {
    p = ParseString(p, end);
  } else {
    p = ParseNumber(p, end);
  }
  return p;
}

const char* JsonParser::ParseObject(const char* p, const char* end) {
  assert(*p == kObjectOpen);
  ++p;
  if (*p == kObjectClose) {
    return ++p;
  }

  while (p != end) {
    p = ParseString(p, end);
    assert(*p == kColon);
    ++p;
    p = ParseValue(p, end);
    if (*p == kObjectClose) {
      break;
    }
    assert(*p == kComma);
    ++p;
  }

  assert(*p == kObjectClose);
  return ++p;
}

const char* JsonParser::ParseString(const char* p, const char* end) {
  assert(*p == kStringOpen);
  ++p;
  auto stringEnd = Find(p, end, kStringClose);
  std::cout << "string: " << std::string{p, stringEnd} << std::endl;
  return ++stringEnd;
}

const char* JsonParser::ParseArray(const char* p, const char* end) {
  assert(*p == kArrayOpen);
  ++p;
  if (*p == kArrayClose) {
    return ++p;
  }
  while (p != end) {
    p = ParseString(p, end);
    if (*p == kArrayClose) {
      break;
    }
    assert(*p == kComma);
    ++p;
  }
  assert(*p == kArrayClose);
  return ++p;
}

const char* JsonParser::ParseNumber(const char* p, const char* end) {
  int num = 0;
  assert(std::isdigit(*p));
  while (p != end && std::isdigit(*p)) {
    num *= 10;
    num += *p - '0';
    ++p;
  }
  std::cout << "num: " << num << std::endl;
  return p;
}

const char* JsonParser::Find(const char* p, const char* end, const char val) {
  while (p != end && *p != val) {
    ++p;
  }
  return p;
}

const char* JsonParser::SkipWhitespace(const char* p, const char* end) {
  while (p != end && *p == kWhitespace) {
    ++p;
  }
  return p;
}

int main() {
  std::string test =
      "{\"name\":\"Carl\",\"age\":10,\"food\":[\"spaghetti\",\"ice-cream\"],"
      "\"empty\":{},\"sub\":{\"fake\":10}}";
  JsonParser p;
  p.Parse(&test[0], &test[test.size()]);
}
