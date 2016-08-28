#pragma once

#include <stdexcept>
#include <string>
#include <iostream>

namespace jp {

class TokenError : public std::exception {
 public:
  TokenError(std::string surroundings, std::string expected, std::string got)
      : surroundings_(std::move(surroundings)),
        expected_(std::move(expected)),
        got_(std::move(got)) {
    what_ += surroundings_;
    what_ += "expected ";
    what_ += expected_;
    what_ += ", got ";
    what_ += got_;
  }

  const char* what() const noexcept { return what_.c_str(); }

  const std::string& expected() const noexcept { return expected_; }
  const std::string& got() const noexcept { return got_; }

 private:
  const std::string surroundings_;
  const std::string expected_;
  const std::string got_;
  std::string what_;
};
}
