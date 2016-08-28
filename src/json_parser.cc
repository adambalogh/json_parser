#include "json_parser.h"

#include <assert.h>
#include <cctype>
#include <unordered_set>
#include <iostream>
#include <sstream>

#include "helpers.h"
#include "token_error.h"

namespace jp {

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
const std::string kNull = "null";

// TODO implement parsing of number exponent
// TODO handle too deep JSONs

// Chars that can follow a backslash in a string
const std::unordered_set<char> following_escape{'"', '\\', '/', 'b',
                                                'f', 'n',  'r', 't'};

// GetNextControlToken always leaves p_ pointing to the parsed ControlToken,
// which is always a single char.
JsonParser::ControlToken JsonParser::GetNextControlToken() {
  SkipSpace();
  if (p_ == end_) {
    throw std::runtime_error(GetSurroundings() + "unexpected end of input");
  }
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
    case 'n':
      return NULL_VALUE;
    default:
      if (*p_ == kMinusSign || std::isdigit(*p_)) {
        return NUMBER;
      }
      return INVALID;
  }
}

JsonValue JsonParser::Parse() {
  const auto obj = ParseValue();
  SkipSpace();
  if (p_ != end_) {
    throw std::runtime_error("unexpected string at end");
  }
  return obj;
}

JsonValue JsonParser::ParseValue(const ControlToken ct) {
  switch (ct) {
    case OBJECT_OPEN:
      return JsonValue{ParseObject()};
    case ARRAY_OPEN:
      return JsonValue{ParseArray()};
    case STRING:
      return JsonValue{ParseString()};
    case BOOL:
      return JsonValue{ParseBool()};
    case NUMBER:
      return JsonValue{ParseNumber()};
    case NULL_VALUE:
      return ParseNull();
    default:
      throw std::runtime_error(GetSurroundings() +
                               "expected a JSON value, but got token: " +
                               ErrorMessageName(ct));
  }
}

JsonValue::ObjectType JsonParser::ParseObject() {
  assert(*p_ == kObjectOpen);
  AdvanceChar();

  JsonValue::ObjectType obj;

  ControlToken ct = GetNextControlToken();
  if (ct != OBJECT_CLOSE) {
    JsonValue::StringType key;

    while (true) {
      Expect(STRING, ct);
      key = ParseString();
      ct = GetNextControlToken();
      Expect(COLON, ct);
      AdvanceChar();
      obj.emplace(key, std::move(ParseValue()));
      ct = GetNextControlToken();
      if (ct != COMMA) {
        Expect(OBJECT_CLOSE, ct);
        break;
      }
      AdvanceChar();
      ct = GetNextControlToken();
    }
  }

  assert(*p_ == kObjectClose);
  AdvanceChar();
  return obj;
}

JsonValue::ArrayType JsonParser::ParseArray() {
  assert(*p_ == kArrayOpen);
  AdvanceChar();

  JsonValue::ArrayType arr;

  ControlToken ct = GetNextControlToken();
  if (ct != ARRAY_CLOSE) {
    while (true) {
      arr.push_back(ParseValue(ct));
      ct = GetNextControlToken();
      if (ct != COMMA) {
        Expect(ARRAY_CLOSE, ct);
        break;
      }
      AdvanceChar();
      ct = GetNextControlToken();
    }
  }

  assert(*p_ == kArrayClose);
  AdvanceChar();
  return arr;
}

// TODO this doesn't handle UTF-8 and some other edge cases
JsonValue::StringType JsonParser::ParseString() {
  assert(*p_ == kStringOpen);
  AdvanceChar();

  const char* const string_start = p_;
  while (p_ != end_ && *p_ != kStringClose) {
    // only literal whitespace char allowed inside a string is a space (' ')
    if (*p_ != ' ' && std::isspace(*p_)) {
      throw std::runtime_error(
          GetSurroundings() +
          "literal whitespace chars are not allowed inside JSON string");
    }
    if (*p_ == kEscapeChar) {
      AdvanceChar();
      if (following_escape.count(*p_) == 0) {
        throw std::runtime_error(GetSurroundings() + "invalid escape char");
      }
    }
    AdvanceChar();
  }
  Expect(kStringClose);

  JsonValue::StringType str{string_start, p_};
  assert(*p_ == kStringClose);
  AdvanceChar();
  return str;
}

JsonValue::NumberType JsonParser::ParseNumber() {
  bool negative = false;
  if (*p_ == kMinusSign) {
    negative = true;
    AdvanceChar();
  }

  if (!std::isdigit(*p_)) {
    throw std::runtime_error("expected number");
  }
  JsonValue::NumberType num = 0;

  if (*p_ == '0') {
    if (std::isdigit(*(p_ + 1))) {
      throw std::runtime_error("0 cannot be followed by digits");
    }
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
    AdvanceChar();
    if (!std::isdigit(*p_)) {
      throw std::runtime_error(GetSurroundings() +
                               ". must be followed by number");
    }
    int power_of_ten = 0;
    int fraction = 0;
    while (std::isdigit(*p_)) {
      fraction *= 10;
      fraction += *p_ - '0';
      ++power_of_ten;
      AdvanceChar();
    }
    num += static_cast<double>(fraction) / pow(10, power_of_ten);
  }
  // Parse exponential notation
  if (*p_ == 'e' || *p_ == 'E') {
    AdvanceChar();
    bool negative = false;
    if (!std::isdigit(*p_)) {
      if (*p_ != '+') {
        negative = true;
        Expect('-');
      }
      AdvanceChar();
    }
    if (!std::isdigit(*p_)) {
      throw std::runtime_error(GetSurroundings() +
                               "exponent must be followed by number");
    }
    int power = 0;
    while (std::isdigit(*p_)) {
      power *= 10;
      power += *p_ - '0';
      AdvanceChar();
    }
    if (negative) {
      power *= -1;
    }
    num *= pow(10, power);
  }

  if (negative) {
    num *= -1;
  }
  return num;
}

JsonValue::BoolType JsonParser::ParseBool() {
  if (Match(kTrue)) {
    return true;
  }
  if (Match(kFalse)) {
    return false;
  }
  throw std::runtime_error("invalid bool value");
}

JsonValue JsonParser::ParseNull() {
  if (Match(kNull)) {
    return JsonValue();
  }
  throw std::runtime_error("invalid null value");
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

void JsonParser::SkipSpace() {
  while (p_ != end_ && std::isspace(*p_)) {
    AdvanceChar();
  }
}

std::string JsonParser::GetSurroundings() const {
  const long max_length = 10;
  std::string out;
  auto prefix_length = std::min(max_length, std::distance(start_, p_));
  const char* s = p_ - prefix_length;
  for (; s != p_; ++s) {
    out += *s;
  }
  if (p_ != end_) {
    out += *s;
    ++s;
    for (int i = 0; i < max_length && s != end_; ++i, ++s) {
      out += *s;
    }
  }
  out += '\n';
  for (int i = 0; i < prefix_length; ++i) {
    out += ' ';
  }
  out += '^';
  out += '\n';
  return out;
}

std::string JsonParser::ErrorMessageName(const ControlToken ct) const {
  switch (ct) {
    case OBJECT_OPEN:
      return quote('{');
    case OBJECT_CLOSE:
      return quote('}');
    case ARRAY_OPEN:
      return quote('[');
    case ARRAY_CLOSE:
      return quote(']');
    case STRING:
      return "a string";
    case NUMBER:
      return "a number";
    case BOOL:
      return "true or false";
    case NULL_VALUE:
      return "null";
    case COLON:
      return quote(':');
    case COMMA:
      return quote(',');
    case INVALID:
      return quote(*p_);
  }
}

void JsonParser::Expect(ControlToken expected, ControlToken actual) const {
  if (actual != expected) {
    throw jp::TokenError{GetSurroundings(), ErrorMessageName(expected),
                         ErrorMessageName(actual)};
  }
}

void JsonParser::Expect(const char c) const {
  if (p_ == end_) {
    std::stringstream error_msg;
    error_msg << GetSurroundings() << std::endl << "expected " << quote(c)
              << ", but reached end of input";
    throw std::runtime_error(error_msg.str());
  }
  if (*p_ != c) {
    throw jp::TokenError{GetSurroundings(), quote(c), quote(*p_)};
  }
}
}
