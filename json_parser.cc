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

// TODO replace assertions with exceptions
// TODO implement parsing of true, false, null
// TODO implement parsing of number exponent

// Chars that can follow a backslash in a string
const std::unordered_set<char> following_escape{'"', '\\', '/', 'b',
                                                'f', 'n',  'r', 't'};

JsonValue JsonParser::Parse() {
  const auto obj = ParseObject();
  assert(p_ == end_);
  return JsonValue{obj};
}

JsonValue::ObjectType JsonParser::ParseObject() {
  assert(*p_ == kObjectOpen);
  ++p_;
  if (*p_ == kObjectClose) {
    ++p_;
    return JsonValue::ObjectType{};
  }

  JsonValue::ObjectType obj;
  JsonValue::StringType key;
  while (true) {
    key = ParseString();
    assert(*p_ == kColon);
    ++p_;
    obj.emplace(key, std::move(ParseValue()));
    if (*p_ == kObjectClose) {
      break;
    }
    assert(*p_ == kComma);
    ++p_;
  }

  assert(*p_ == kObjectClose);
  ++p_;

  return obj;
}

JsonValue JsonParser::ParseValue() {
  if (*p_ == kObjectOpen) {
    return JsonValue{ParseObject()};
  } else if (*p_ == kArrayOpen) {
    return JsonValue{ParseArray()};
  } else if (*p_ == kStringOpen) {
    return JsonValue{ParseString()};
  } else {
    return JsonValue{ParseNumber()};
  }
}

JsonValue::ArrayType JsonParser::ParseArray() {
  assert(*p_ == kArrayOpen);
  ++p_;
  if (*p_ == kArrayClose) {
    ++p_;
    return JsonValue::ArrayType{};
  }

  JsonValue::ArrayType arr;
  while (true) {
    arr.push_back(ParseValue());
    if (*p_ == kArrayClose) {
      break;
    }
    assert(*p_ == kComma);
    ++p_;
  }

  assert(*p_ == kArrayClose);
  ++p_;
  return arr;
}

JsonValue::StringType JsonParser::ParseString() {
  assert(*p_ == kStringOpen);
  ++p_;
  const char* const string_start = p_;
  while (*p_ != kStringClose) {
    if (*p_ == kEscapeChar) {
      ++p_;
      assert(following_escape.count(*p_) != 0);
    }
    ++p_;
  }
  JsonValue::StringType str{string_start, p_};
  ++p_;
  return str;
}

JsonValue::NumberType JsonParser::ParseNumber() {
  JsonValue::NumberType num = 0;
  bool negative = false;
  if (*p_ == kMinusSign) {
    negative = true;
    ++p_;
  }

  assert(std::isdigit(*p_));

  if (*p_ == '0') {
    assert(*(p_ + 1) == kDot);
    ++p_;
  } else {
    while (std::isdigit(*p_)) {
      num *= 10;
      num += *p_ - '0';
      ++p_;
    }
  }
  // Parse fraction, if present
  if (*p_ == kDot) {
    int place = 0;
    int fraction = 0;
    ++p_;
    while (std::isdigit(*p_)) {
      fraction *= 10;
      fraction += *p_ - '0';
      ++place;
      ++p_;
    }
    num += static_cast<double>(fraction) / pow(10, place);
  }

  if (negative) {
    num *= -1;
  }
  return num;
}

void JsonParser::Find(const char val) {
  while (*p_ != val) {
    ++p_;
  }
}

void JsonParser::SkipWhitespace() {
  while (*p_ == kWhitespace) {
    ++p_;
  }
}
