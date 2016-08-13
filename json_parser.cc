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
const char kMinusSign = '-';
const char kDot = '.';

// Chars that can follow a backslash in a string
const std::unordered_set<char> following_escape{'"', '\\', '/', 'b',
                                                'f', 'n',  'r', 't'};

// TODO replace assertions with exceptions
void JsonParser::Parse(const char* p, const char* end) {
  p = ParseObject(p, end);
  assert(p == end);
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
  double num = 0;
  bool negative = false;
  if (*p == kMinusSign) {
    negative = true;
    ++p;
  }

  assert(std::isdigit(*p));

  if (*p == '0') {
    assert(*(p + 1) == kDot);
    ++p;
  } else {
    while (std::isdigit(*p)) {
      num *= 10;
      num += *p - '0';
      ++p;
    }
  }
  // Parse fraction, if present
  if (*p == kDot) {
    int place = 0;
    int fraction = 0;
    ++p;
    while (std::isdigit(*p)) {
      fraction *= 10;
      fraction += *p - '0';
      ++place;
      ++p;
    }
    num += static_cast<double>(fraction) / pow(10, place);
  }

  if (negative) {
    num *= -1;
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