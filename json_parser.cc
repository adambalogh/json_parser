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
const std::string kTrue = "true";
const std::string kFalse = "false";

// TODO replace assertions with exceptions
// TODO implement parsing of null
// TODO implement parsing of number exponent

// Chars that can follow a backslash in a string
const std::unordered_set<char> following_escape{'"', '\\', '/', 'b',
                                                'f', 'n',  'r', 't'};

JsonParser::ControlToken JsonParser::GetNextControlToken() {
  SkipWhitespace();
  ControlToken token;
  switch (*p_) {
    case kObjectOpen:
      return OBJECT_OPEN;
    case kObjectClose:
      return OBJECT_CLOSE;
    case kArrayOpen:
      return ARRAY_OPEN;
    case kArrayClose:
      return ARRAY_CLOSE;
    case kComma:
      return COMMA;
    case kStringOpen:
      return STRING;
    case kColon:
      return COLON;
    case 't':
    case 'f':
      return BOOL;
    default:
      if (*p_ == kMinusSign || std::isdigit(*p_)) {
        return NUMBER;
      }
      std::string error = "invalid control token: ";
      error += *p_;
      throw std::runtime_error(error);
  }
}

JsonValue JsonParser::Parse() {
  const auto obj = ParseValue();
  assert(p_ == end_);
  return obj;
}

JsonValue JsonParser::ParseValue(const ControlToken ct) {
  if (ct == OBJECT_OPEN) {
    return JsonValue{ParseObject()};
  }
  if (ct == ARRAY_OPEN) {
    return JsonValue{ParseArray()};
  }
  if (ct == STRING) {
    return JsonValue{ParseString()};
  }
  if (ct == BOOL) {
    return JsonValue(ParseBool());
  }
  if (ct == NUMBER) {
    return JsonValue{ParseNumber()};
  }
  throw std::runtime_error("invalid control token");
}

JsonValue::ObjectType JsonParser::ParseObject() {
  assert(*p_ == kObjectOpen);
  Advance();

  ControlToken ct = GetNextControlToken();

  JsonValue::ObjectType obj;
  JsonValue::StringType key;

  while (ct != OBJECT_CLOSE) {
    assert(ct == STRING);
    key = ParseString();
    ct = GetNextControlToken();
    assert(ct == COLON);
    Advance();
    obj.emplace(key, std::move(ParseValue()));
    ct = GetNextControlToken();
    if (ct != COMMA) {
      break;
    }
    Advance();
    ct = GetNextControlToken();
  }

  assert(*p_ == kObjectClose);
  Advance();
  return obj;
}

JsonValue::ArrayType JsonParser::ParseArray() {
  assert(*p_ == kArrayOpen);
  Advance();

  ControlToken ct = GetNextControlToken();

  JsonValue::ArrayType arr;
  while (ct != ARRAY_CLOSE) {
    arr.push_back(ParseValue(ct));
    ct = GetNextControlToken();
    if (ct != COMMA) {
      break;
    }
    Advance();
    ct = GetNextControlToken();
  }

  assert(*p_ == kArrayClose);
  Advance();
  return arr;
}

JsonValue::StringType JsonParser::ParseString() {
  assert(*p_ == kStringOpen);
  Advance();

  const char* const string_start = p_;
  while (*p_ != kStringClose) {
    if (*p_ == kEscapeChar) {
      Advance();
      assert(following_escape.count(*p_) != 0);
    }
    Advance();
  }
  JsonValue::StringType str{string_start, p_};
  Advance();
  return str;
}

JsonValue::NumberType JsonParser::ParseNumber() {
  JsonValue::NumberType num = 0;
  bool negative = false;
  if (*p_ == kMinusSign) {
    negative = true;
    Advance();
  }

  assert(std::isdigit(*p_));

  if (*p_ == '0') {
    assert(*(p_ + 1) == kDot);
    Advance();
  } else {
    while (std::isdigit(*p_)) {
      num *= 10;
      num += *p_ - '0';
      Advance();
    }
  }
  // Parse fraction, if present
  if (*p_ == kDot) {
    int place = 0;
    int fraction = 0;
    Advance();
    while (std::isdigit(*p_)) {
      fraction *= 10;
      fraction += *p_ - '0';
      ++place;
      Advance();
    }
    num += static_cast<double>(fraction) / pow(10, place);
  }

  if (negative) {
    num *= -1;
  }
  return num;
}

JsonValue::BoolType JsonParser::ParseBool() {
  if (Match(kTrue)) {
    return JsonValue{true};
  }
  if (Match(kFalse)) {
    return JsonValue{false};
  }
  throw std::runtime_error("invalid bool value");
}

bool JsonParser::Match(const std::string& val) {
  for (int i = 0; i < val.size(); ++i) {
    if (p_ + i == end_) {
      return false;
    }
    if (*(p_ + i) != val[i]) {
      return false;
    }
  }
  p_ += val.size();
  return true;
}

void JsonParser::Find(const char val) {
  while (*p_ != val) {
    Advance();
  }
}

void JsonParser::SkipWhitespace() {
  while (*p_ == kWhitespace) {
    Advance();
  }
}
