#include <string>
#include <iostream>

#include <gtest/gtest.h>

#include "json_parser.h"

using namespace ::testing;
using std::string;

TEST(JsonParser, EmptyJson) {
  string e = "{}";
  std::cout << JsonParser{e}.Parse().to_string() << std::endl;
}

TEST(JsonParser, GetString) {
  string e = "{\"name\":\"Adam\"}";
  auto obj = JsonParser{e}.Parse();
  EXPECT_TRUE(obj.is<JsonValue::OBJECT>());
  EXPECT_TRUE(obj.getObject().at("name").is<JsonValue::STRING>());
  EXPECT_EQ("Adam", obj.getObject().at("name").getString());
}

TEST(JsonParser, Complex) {
  string e =
      "{\"name\":\"Carl\",\"age\":-0.010,\"food\":[\"spaghetti\",\"ice-"
      "cream\"],\"empty\":{},\"sub\":{\"fake\":-10.94}}";
  std::cout << JsonParser{e}.Parse().to_string() << std::endl;
}

TEST(JsonParser, StringEscapeChar) {
  std::vector<string> test_cases{"{\"name\":\"\\\\\"}", "{\"name\":\"\\\"\"}"};
  for (auto& t : test_cases) {
    std::cout << JsonParser{t}.Parse().to_string() << std::endl;
  }
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
