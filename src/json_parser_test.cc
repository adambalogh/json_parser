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

TEST(JsonParser, String) {
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

TEST(JsonParser, NumberParsing) {
  std::vector<std::pair<std::string, double>> tests{{"-1", -1},
                                                    {"1", 1},
                                                    {"0", 0},
                                                    {"0.1", 0.1},
                                                    {"1e0", 1},
                                                    {"1.4e2", 140},
                                                    {"10e3", 10000},
                                                    {"10e+3", 10000},
                                                    {"10E+3", 10000},
                                                    {"-0.12e3", -120},
                                                    {"1e-2", 0.01}};
  for (const auto& pair : tests) {
    std::string json = "{\"num\": " + pair.first + "}";
    auto obj = JsonParser{json}.Parse().getObject();
    EXPECT_EQ(obj.at("num").getNumber(), pair.second);
  }
}

TEST(JsonParser, EmptyArray) {
  string e = "{\"name\":[]}";
  auto obj = JsonParser{e}.Parse();
  EXPECT_TRUE(obj.getObject().at("name").is<JsonValue::ARRAY>());
  EXPECT_EQ(0, obj.getObject().at("name").getArray().size());
}

TEST(JsonParser, Array) {
  string e = "{\"name\":[\"a\",\"b\"]}";
  auto obj = JsonParser{e}.Parse();
  EXPECT_TRUE(obj.getObject().at("name").is<JsonValue::ARRAY>());
  auto arr = obj.getObject().at("name").getArray();
  EXPECT_EQ(2, arr.size());
  EXPECT_EQ("a", arr[0].getString());
  EXPECT_EQ("b", arr[1].getString());
}

TEST(JsonParser, InvalidBool) {
  string e = "{\"val\": trio}";
  EXPECT_THROW(JsonParser{e}.Parse(), std::runtime_error);
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

TEST(JsonParser, InvalidNumber) {
  std::vector<std::string> jsons = {"{\"a\": 1", "{\"a\": 1.", "{\"a\": 1.}"};

  for (const auto& json : jsons) {
    try {
      JsonParser{json}.Parse();
      FAIL();
    } catch (std::exception& e) {
    }
  }
}

TEST(JsonParser, ComplexJson) {
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
  std::vector<std::pair<std::string, std::string>> test_cases{
      {"\\\\", "\\"}, {"jo\\ni", "jo\ni"}};
  for (auto& t : test_cases) {
    std::string json = "{\"name\":\"" + t.first + "\"}";
    EXPECT_EQ(t.second,
              JsonParser{json}.Parse().getObject().at("name").getString());
  }
}

TEST(JsonParser, SkipWhiteSpace) {
  string e = " { \" na me \"   :  \" A da m \"    , \"age   \":  10  }";
  auto obj = JsonParser{e}.Parse().getObject();
  EXPECT_EQ(" A da m ", obj.at(" na me ").getString());
  EXPECT_EQ(10, obj.at("age   ").getNumber());
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
      JsonParser{e}.Parse();
      std::cout << file_entry.path() << std::endl;
      FAIL();
    } catch (std::exception& e) {
    }
  }
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
