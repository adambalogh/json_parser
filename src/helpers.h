#pragma once

#include <string>

std::string quote(const char c) {
  std::string out;
  out += "'";
  out += c;
  out += "'";
  return out;
}
