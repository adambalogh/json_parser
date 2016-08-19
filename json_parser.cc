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
      throw std::runtime_error("invalid control token");
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
  AdvanceChar();

  ControlToken ct = GetNextControlToken();

  JsonValue::ObjectType obj;
  JsonValue::StringType key;

  while (ct != OBJECT_CLOSE) {
    assert(ct == STRING);
    key = ParseString();
    ct = GetNextControlToken();
    assert(ct == COLON);
    AdvanceChar();
    obj.emplace(key, std::move(ParseValue()));
    ct = GetNextControlToken();
    if (ct != COMMA) {
      assert(ct == OBJECT_CLOSE);
      break;
    }
    AdvanceChar();
    ct = GetNextControlToken();
  }

  assert(*p_ == kObjectClose);
  AdvanceChar();
  return obj;
}

JsonValue::ArrayType JsonParser::ParseArray() {
  assert(*p_ == kArrayOpen);
  AdvanceChar();

  ControlToken ct = GetNextControlToken();

  JsonValue::ArrayType arr;

  while (ct != ARRAY_CLOSE) {
    arr.push_back(ParseValue(ct));
    ct = GetNextControlToken();
    if (ct != COMMA) {
      assert(ct == ARRAY_CLOSE);
      break;
    }
    AdvanceChar();
    ct = GetNextControlToken();
  }

  assert(*p_ == kArrayClose);
  AdvanceChar();
  return arr;
}

JsonValue::StringType JsonParser::ParseString() {
  assert(*p_ == kStringOpen);
  AdvanceChar();

  const char* const string_start = p_;
  while (*p_ != kStringClose) {
    if (*p_ == kEscapeChar) {
      AdvanceChar();
      assert(following_escape.count(*p_) != 0);
    }
    AdvanceChar();
  }

  JsonValue::StringType str{string_start, p_};
  AdvanceChar();
  return str;
}

JsonValue::NumberType JsonParser::ParseNumber() {
  bool negative = false;
  if (*p_ == kMinusSign) {
    negative = true;
    AdvanceChar();
  }

  assert(std::isdigit(*p_));
  JsonValue::NumberType num = 0;

  if (*p_ == '0') {
    assert(*(p_ + 1) == kDot);
    AdvanceChar();
  } else {
    while (std::isdigit(*p_)) {
      num *= 10;
      num += *p_ - '0';
      AdvanceChar();
    }
  }
  // Parse fraction, if present
  if (*p_ == kDot) {
    int power_of_ten = 0;
    int fraction = 0;
    AdvanceChar();
    while (std::isdigit(*p_)) {
      fraction *= 10;
      fraction += *p_ - '0';
      ++power_of_ten;
      AdvanceChar();
    }
    num += static_cast<double>(fraction) / pow(10, power_of_ten);
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

void JsonParser::SkipWhitespace() {
  while (*p_ == kWhitespace) {
    AdvanceChar();
  }
}
