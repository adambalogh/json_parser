#pragma once

#include <string>

namespace jp {

std::string quote(const char c) {
  std::string out;
  out += "'";
  out += c;
  out += "'";
  return out;
}
}
