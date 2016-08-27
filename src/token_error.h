#pragma once

#include <stdexcept>
#include <string>

namespace jp {

class TokenError : public std::exception {
 public:
  TokenError(std::string surroundings, std::string expected, std::string got)
      : surroundings_(std::move(surroundings)),
        expected_(std::move(expected)),
        got_(std::move(got)) {
    std::string what;
    what += surroundings_;
    what += '\n';
    what += "expected ";
    what += expected_;
    what += " got ";
    what += got_;
    what_ = what.c_str();
  }

  const char* what() const noexcept { return what_; }

  const std::string& expected() const noexcept { return expected_; }
  const std::string& got() const noexcept { return got_; }

 private:
  const std::string surroundings_;
  const std::string expected_;
  const std::string got_;
  const char* what_;
};
}
