#C++ Json Parser

Under development.

## Example Usage

```c++
#include <assert.h>
#include <string>

#include "json_parser.h"

int main() {
  std::string json = "{\"name\": \"John\", \"age\": 31}";

  JsonValue value = JsonParser{json}.Parse();
  JsonValue::ObjectType person = value.getObject();

  assert(person.at("name").is<JsonValue::STRING>());
  std::string name = person.at("name");  // == "John"

  assert(person.at("age").is<JsonValue::NUMBER>());
  double age = person.at("age");  // == 31
}
```
