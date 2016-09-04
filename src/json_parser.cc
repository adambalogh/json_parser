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

const std::unordered_map<char, char> escaped_map{{'"', '"'},
                                                 {'\\', '\\'},
                                                 {'/', '/'},
                                                 {'b', '\b'},
                                                 {'f', '\f'},
                                                 {'n', '\n'},
                                                 {'r', '\r'},
                                                 {'t', '\t'}};

// GetNextControlToken always leaves p_ pointing to the parsed ControlToken,
// which is always a single char.
JsonParser::ControlToken JsonParser::GetNextControlToken() {
  SkipSpace();
  const auto c = GetChar();
  switch (c) {
    case kObjectOpen:
      return ControlToken::OBJECT_OPEN;
    case kObjectClose:
      return ControlToken::OBJECT_CLOSE;
    case kArrayOpen:
      return ControlToken::ARRAY_OPEN;
    case kArrayClose:
      return ControlToken::ARRAY_CLOSE;
    case kComma:
      return ControlToken::COMMA;
    case kStringOpen:
      return ControlToken::STRING;
    case kColon:
      return ControlToken::COLON;
    case 't':
    case 'f':
      return ControlToken::BOOL;
    case 'n':
      return ControlToken::NULL_VALUE;
    default:
      if (std::isdigit(c) || c == kMinusSign) {
        return ControlToken::NUMBER;
      }
      return ControlToken::INVALID;
  }
}

JsonValue JsonParser::Parse() {
  auto obj = ParseValue();
  SkipSpace();
  if (Capacity()) {
    throw std::runtime_error("unexpected string at the end of input");
  }
  return obj;
}

JsonValue JsonParser::ParseValue(const ControlToken ct) {
  switch (ct) {
    case ControlToken::OBJECT_OPEN:
      return JsonValue{ParseObject()};
    case ControlToken::ARRAY_OPEN:
      return JsonValue{ParseArray()};
    case ControlToken::STRING:
      return JsonValue{ParseString()};
    case ControlToken::BOOL:
      return JsonValue{ParseBool()};
    case ControlToken::NUMBER:
      return JsonValue{ParseNumber()};
    case ControlToken::NULL_VALUE:
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
  if (ct != ControlToken::OBJECT_CLOSE) {
    JsonValue::StringType key;

    while (true) {
      Expect(ControlToken::STRING, ct);
      key = ParseString();

      ct = GetNextControlToken();
      Expect(ControlToken::COLON, ct);
      AdvanceChar();

      obj.emplace(std::make_pair(key, ParseValue()));
      ct = GetNextControlToken();
      if (ct != ControlToken::COMMA) {
        Expect(ControlToken::OBJECT_CLOSE, ct);
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
  if (ct != ControlToken::ARRAY_CLOSE) {
    while (true) {
      arr.push_back(ParseValue(ct));
      ct = GetNextControlToken();
      if (ct != ControlToken::COMMA) {
        Expect(ControlToken::ARRAY_CLOSE, ct);
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

// TODO this doesn't handle UTF-8
JsonValue::StringType JsonParser::ParseString() {
  assert(GetChar() == kStringOpen);
  AdvanceChar();

  const char* const start = p_;
  char c;

  int num_escaped_chars = 0;
  while ((c = GetChar()) != kStringClose) {
    // only literal whitespace char allowed inside a string is a space,
    // everything else must be escaped
    if (c != ' ' && std::isspace(c)) {
      throw std::runtime_error(
          GetSurroundings() +
          "literal whitespace chars are not allowed inside JSON string");
    }
    if (c == kEscapeChar) {
      c = GetNextChar();
      if (following_escape.count(c) == 0) {
        throw std::runtime_error(GetSurroundings() + "invalid escape char");
      }
      ++num_escaped_chars;
    }
    AdvanceChar();
  }

  JsonValue::StringType str;
  str.resize(p_ - start - num_escaped_chars);

  int i = 0;
  for (const char* c = start; c != p_; ++c) {
    if (*c == kEscapeChar) {
      ++c;
      str[i] = escaped_map.at(*c);
    } else {
      str[i] = *c;
    }
    ++i;
  }

  assert(i == str.size());
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

// TODO refactor this
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

// TODO move this elsewhere
std::string JsonParser::ErrorMessageName(const ControlToken ct) const {
  switch (ct) {
    case ControlToken::OBJECT_OPEN:
      return quote('{');
    case ControlToken::OBJECT_CLOSE:
      return quote('}');
    case ControlToken::ARRAY_OPEN:
      return quote('[');
    case ControlToken::ARRAY_CLOSE:
      return quote(']');
    case ControlToken::STRING:
      return "a string";
    case ControlToken::NUMBER:
      return "a number";
    case ControlToken::BOOL:
      return "true or false";
    case ControlToken::NULL_VALUE:
      return "null";
    case ControlToken::COLON:
      return quote(':');
    case ControlToken::COMMA:
      return quote(',');
    case ControlToken::INVALID:
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
