#include "json_parser.h"

#include <assert.h>
#include <cctype>
#include <unordered_set>
#include <iostream>

// Removes whitespace before and after a json element is parsed
#define WS(line)    \
  SkipWhitespace(); \
  line;             \
  SkipWhitespace();

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
  switch (GetChar()) {
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
      if (*p_ == kMinusSign || std::isdigit(GetChar())) {
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

JsonValue JsonParser::ParseValue(ControlToken ct) {
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
  assert(GetChar() == kObjectOpen);
  NextChar();

  ControlToken ct = GetNextControlToken();

  JsonValue::ObjectType obj;
  JsonValue::StringType key;

  while (ct != OBJECT_CLOSE) {
    assert(ct == STRING);
    key = ParseString();
    ct = GetNextControlToken();
    assert(ct == COLON);
    NextChar();
    obj.emplace(key, std::move(ParseValue()));
    ct = GetNextControlToken();
    if (ct != COMMA) {
      break;
    }
    NextChar();
    ct = GetNextControlToken();
  }

  assert(GetChar() == kObjectClose);
  NextChar();
  return obj;
}

JsonValue::ArrayType JsonParser::ParseArray() {
  assert(GetChar() == kArrayOpen);
  NextChar();

  ControlToken ct = GetNextControlToken();

  JsonValue::ArrayType arr;
  while (ct != ARRAY_CLOSE) {
    arr.push_back(ParseValue(ct));
    ct = GetNextControlToken();
    if (ct != COMMA) {
      break;
    }
    NextChar();
    ct = GetNextControlToken();
  }

  assert(GetChar() == kArrayClose);
  NextChar();
  return arr;
}

JsonValue::StringType JsonParser::ParseString() {
  assert(GetChar() == kStringOpen);
  NextChar();

  const char* const string_start = p_;
  while (GetChar() != kStringClose) {
    if (GetChar() == kEscapeChar) {
      NextChar();
      assert(following_escape.count(GetChar()) != 0);
    }
    NextChar();
  }
  JsonValue::StringType str{string_start, p_};
  NextChar();
  return str;
}

JsonValue::NumberType JsonParser::ParseNumber() {
  JsonValue::NumberType num = 0;
  bool negative = false;
  if (GetChar() == kMinusSign) {
    negative = true;
    NextChar();
  }

  assert(std::isdigit(GetChar()));

  if (GetChar() == '0') {
    assert(*(p_ + 1) == kDot);
    NextChar();
  } else {
    while (std::isdigit(GetChar())) {
      num *= 10;
      num += GetChar() - '0';
      NextChar();
    }
  }
  // Parse fraction, if present
  if (GetChar() == kDot) {
    int place = 0;
    int fraction = 0;
    NextChar();
    while (std::isdigit(GetChar())) {
      fraction *= 10;
      fraction += GetChar() - '0';
      ++place;
      NextChar();
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
  while (GetChar() != val) {
    NextChar();
  }
}

void JsonParser::SkipWhitespace() {
  while (GetChar() == kWhitespace) {
    NextChar();
  }
}
