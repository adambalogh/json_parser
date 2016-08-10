#include "json_parser.h"

#include <assert.h>
#include <cctype>
#include <unordered_set>
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
const char kEscapeChar = '\\';

// Chars that can follow a backslash in a string
const std::unordered_set<char> following_escape{'"', '\\', '/', 'b',
                                                'f', 'n',  'r', 't'};

void JsonParser::Parse(const char* p, const char* end) { ParseObject(p, end); }

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

  while (true) {
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

const char* JsonParser::ParseArray(const char* p, const char* end) {
  assert(*p == kArrayOpen);
  ++p;
  if (*p == kArrayClose) {
    return ++p;
  }

  while (true) {
    p = ParseValue(p, end);
    if (*p == kArrayClose) {
      break;
    }
    assert(*p == kComma);
    ++p;
  }

  assert(*p == kArrayClose);
  return ++p;
}

const char* JsonParser::ParseString(const char* p, const char* end) {
  assert(*p == kStringOpen);
  ++p;
  const char* const string_start = p;
  while (true) {
    if (*p == kStringClose) {
      break;
    }
    if (*p == kEscapeChar) {
      ++p;
      assert(following_escape.count(*p) != 0);
    }
    ++p;
  }
  std::cout << "string: " << std::string{string_start, p} << std::endl;
  return ++p;
}

const char* JsonParser::ParseNumber(const char* p, const char* const end) {
  int num = 0;
  assert(std::isdigit(*p));
  while (std::isdigit(*p)) {
    num *= 10;
    num += *p - '0';
    ++p;
  }
  std::cout << "num: " << num << std::endl;
  return p;
}

const char* JsonParser::Find(const char* p, const char* end, const char val) {
  while (*p != val) {
    ++p;
  }
  return p;
}

const char* JsonParser::SkipWhitespace(const char* p, const char* end) {
  while (*p == kWhitespace) {
    ++p;
  }
  return p;
}
