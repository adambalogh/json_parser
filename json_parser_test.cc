#include <string>
#include <iostream>

#include <gtest/gtest.h>

#include "json_parser.h"

using namespace ::testing;
using std::string;

TEST(JsonParser, EmptyJson) {
  string e = "{}";
  auto obj = JsonParser{e}.Parse().getObject();
  EXPECT_EQ(0, obj.size());
}

TEST(JsonParser, GetString) {
  string e = "{\"name\":\"Adam\"}";
  auto obj = JsonParser{e}.Parse();
  EXPECT_TRUE(obj.getObject().at("name").is<JsonValue::STRING>());
  EXPECT_EQ("Adam", obj.getObject().at("name").getString());
}

TEST(JsonParser, Bool) {
  string e = "{\"val\":true}";
  auto obj = JsonParser{e}.Parse();
  EXPECT_TRUE(obj.getObject().at("val").is<JsonValue::BOOL>());
  EXPECT_EQ(true, obj.getObject().at("val").getBool());
}

TEST(JsonParser, EmptyArray) {
  string e = "{\"name\":[]}";
  auto obj = JsonParser{e}.Parse();
  EXPECT_TRUE(obj.getObject().at("name").is<JsonValue::ARRAY>());
  EXPECT_EQ(0, obj.getObject().at("name").getArray().size());
}

TEST(JsonParser, Complex) {
  string e =
      "{\"name\":\"Carl\",\"age\":-0.010,\"food\":[\"spaghetti\",\"ice-"
      "cream\"],\"sub\":{\"fake\":-10.94}}";
  auto obj = JsonParser{e}.Parse().getObject();
  EXPECT_EQ("Carl", obj.at("name").getString());
  EXPECT_EQ(-0.01, obj.at("age").getNumber());
  EXPECT_TRUE(obj.at("sub").is<JsonValue::OBJECT>());
  EXPECT_EQ(-10.94, obj.at("sub").getObject().at("fake").getNumber());
}

TEST(JsonParser, StringEscape) {
  std::vector<string> test_cases{"\\\\", "\\\""};
  for (auto& t : test_cases) {
    std::string json = "{\"name\":\"" + t + "\"}";
    EXPECT_EQ(t, JsonParser{json}.Parse().getObject().at("name").getString());
  }
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
