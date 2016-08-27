#C++ Json Parser

## Example Usage

```c++
#include <string>

#include "json_parser.h"

int main() {
  std::string json = "{\"name\": \"John\", \"age\": 31}";

  JsonValue value = JsonParser{json}.Parse();
  JsonValue::ObjectType person = value.getObject();

  assert(person.at("name").is<JsonValue::STRING>());
  person.at("name").getString();  // == "John"

  assert(person.at("age").is<JsonValue::NUMBER>());
  person.at("age").getNumber();  // == 31
}
```
