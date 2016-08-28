#C++ Json Parser

Under development.

## Example Usage

```c++
#include <assert.h>
#include <string>

#include "json_parser.h"

using jp::JsonValue;
using jp::JsonParser;

int main() {
  std::string json = "{\"name\": \"John\", \"age\": 31}";

  const JsonValue::ObjectType& person = JsonParser{json}.Parse();

  assert(person.at("name").is<JsonValue::STRING>());
  const std::string& name = person.at("name");  // == "John"

  assert(person.at("age").is<JsonValue::NUMBER>());
  double age = person.at("age");  // == 31
}

```
