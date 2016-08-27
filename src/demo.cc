#include <assert.h>
#include <string>

#include "json_parser.h"

int main() {
  std::string json = "{\"name\": \"John\", \"age\": 31}";

  JsonValue::ObjectType person = JsonParser{json}.Parse();

  assert(person.at("name").is<JsonValue::STRING>());
  std::string name = person.at("name");  // == "John"

  assert(person.at("age").is<JsonValue::NUMBER>());
  double age = person.at("age");  // == 31
}
