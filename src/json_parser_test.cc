#include <fstream>
#include <string>
#include <iostream>
#include <boost/filesystem.hpp>

#include <gtest/gtest.h>

#include "json_parser.h"
#include "token_error.h"

using namespace ::testing;
using namespace jp;
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

TEST(JsonParser, HandlesWhiteSpace) {
  string e = " { \" na me \"   :  \" A da m \"    , \"age   \":  10  }";
  auto obj = JsonParser{e}.Parse().getObject();
  EXPECT_EQ(" A da m ", obj.at(" na me ").getString());
  EXPECT_EQ(10, obj.at("age   ").getNumber());
}

TEST(JsonParser, InvalidBool) {
  string e = "{\"val\": trio}";
  EXPECT_THROW(JsonParser{e}.Parse(), std::runtime_error);
}

TEST(JsonParser, JsonOrgTests) {
  boost::filesystem::path test_dir("test_data/json.org");
  assert(boost::filesystem::is_directory(test_dir));
  for (auto& file_entry : boost::filesystem::directory_iterator(test_dir)) {
    std::ifstream file{file_entry.path().generic_string()};
    string e((std::istreambuf_iterator<char>(file)),
             std::istreambuf_iterator<char>());
    JsonParser{e}.Parse();
  }
}

TEST(JsonParser, JsonFailTests) {
  boost::filesystem::path test_dir("test_data/json_tests");
  assert(boost::filesystem::is_directory(test_dir));
  for (auto& file_entry : boost::filesystem::directory_iterator(test_dir)) {
    std::ifstream file{file_entry.path().generic_string()};
    string e((std::istreambuf_iterator<char>(file)),
             std::istreambuf_iterator<char>());
    try {
      std::cout << ">" << e << std::endl;
      JsonParser{e}.Parse();
      FAIL();
    } catch (std::exception& e) {
      std::cout << e.what() << std::endl << std::endl;
    }
  }
}

TEST(JsonParser, InvalidJson) {
  std::vector<std::string> jsons{
      ",", "{\"a\":[ :}", "{\"num\": 10, }", "{\"name\" }", "{\"name....}",
      "{\"name\"   \"joe\"}", "{\"Illegal expression\": 1 + 2}"};
  for (const auto& a : jsons) {
    try {
      JsonParser{a}.Parse();
      FAIL();
    } catch (std::exception& e) {
    }
  }
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
