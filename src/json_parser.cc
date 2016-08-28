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
const char kColon = ':';
const char kComma = ',';
const char kEscapeChar = '\\';
const char kMinusSign = '-';
const char kPlusSign = '+';
const char kDot = '.';
const char kExponent = 'e';
const char kCapitalExponent = 'E';
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
  const auto c = GetChar();
  switch (c) {
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
      if (c == kMinusSign || std::isdigit(c)) {
        return NUMBER;
      }
      return INVALID;
  }
}

JsonValue JsonParser::Parse() {
  const auto obj = ParseValue();
  SkipSpace();
  if (Capacity()) {
    throw std::runtime_error("unexpected string at the end of input");
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
  assert(GetChar() == kObjectOpen);
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

      obj.emplace(key, ParseValue());
      ct = GetNextControlToken();
      if (ct != COMMA) {
        Expect(OBJECT_CLOSE, ct);
        break;
      }
      AdvanceChar();
      ct = GetNextControlToken();
    }
  }

  assert(GetChar() == kObjectClose);
  AdvanceChar();
  return obj;
}

JsonValue::ArrayType JsonParser::ParseArray() {
  assert(GetChar() == kArrayOpen);
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

  assert(GetChar() == kArrayClose);
  AdvanceChar();
  return arr;
}

// TODO this doesn't handle UTF-8 and some other edge cases
JsonValue::StringType JsonParser::ParseString() {
  assert(GetChar() == kStringOpen);
  AdvanceChar();

  const char* const start = p_;
  char c;
  while ((c = GetChar()) != kStringClose) {
    // only literal whitespace char allowed inside a string is a space,
    // everything else must be escaped
    if (c != ' ' && std::isspace(c)) {
      throw std::runtime_error(
          GetSurroundings() +
          "literal whitespace chars are not allowed inside JSON string");
    }
    if (c == kEscapeChar) {
      AdvanceChar();
      if (following_escape.count(GetChar()) == 0) {
        throw std::runtime_error(GetSurroundings() + "invalid escape char");
      }
    }
    AdvanceChar();
  }

  JsonValue::StringType str{start, p_};
  assert(GetChar() == kStringClose);
  AdvanceChar();
  return str;
}

// Only call this method if it is expected that we can parse a number with at
// least 1 digit
double JsonParser::ParseSimpleNumber() {
  if (!std::isdigit(GetChar())) {
    throw std::runtime_error(GetSurroundings() + "expected a number");
  }
  int num = 0;
  char c;
  // no valid JSON ends with a digit, so it's fine if we just use GetChar, which
  // throws when we reached the end of input
  while (std::isdigit((c = GetChar()))) {
    num *= 10;
    num += c - '0';
    AdvanceChar();
  }
  return num;
}

JsonValue::NumberType JsonParser::ParseNumber() {
  bool negative = false;
  char c = GetChar();

  // parse minus sign, if there is one
  if (c == kMinusSign) {
    negative = true;
    c = GetNextChar();
  }

  if (!std::isdigit(c)) {
    throw std::runtime_error("expected number");
  }

  JsonValue::NumberType num = 0;
  if (GetChar() == '0') {
    if (std::isdigit(GetNextChar())) {
      throw std::runtime_error("0 cannot be followed by digits");
    }
  } else {
    num = ParseSimpleNumber();
  }
  c = GetChar();

  // Parse fraction, if present
  if (c == kDot) {
    c = GetNextChar();
    if (!std::isdigit(c)) {
      throw std::runtime_error(GetSurroundings() +
                               ". must be followed by number");
    }
    int power_of_ten = 0;
    int fraction = 0;
    while (std::isdigit(c)) {
      fraction *= 10;
      fraction += c - '0';
      ++power_of_ten;
      c = GetNextChar();
    }
    num += static_cast<double>(fraction) / pow(10, power_of_ten);
  }

  // Parse exponential notation
  if (c == kExponent || c == kCapitalExponent) {
    c = GetNextChar();
    bool negative_exponential = false;
    if (!std::isdigit(c)) {
      if (c == kMinusSign) {
        negative_exponential = true;
      } else {
        Expect(kPlusSign);
      }
      c = GetNextChar();
    }

    int power = ParseSimpleNumber();
    if (negative_exponential) {
      power *= -1;
    }
    num *= pow(10, power);
  }

  assert(!std::isdigit(GetChar()));
  return negative ? num * -1 : num;
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
  assert(Capacity());
  return true;
}

void JsonParser::SkipSpace() {
  while (p_ != end_ && std::isspace(GetChar())) {
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
  if (GetChar() != c) {
    throw jp::TokenError{GetSurroundings(), quote(c), quote(*p_)};
  }
}
}
