#C++ Json Parser

## Example Usage

```c++
#include <string>
#include "json_parser.h"

int main() {
  std::string json = "{\"name\": \"John\", \"age\": 31}";

  JsonValue value = JsonParser{json}.Parse();
  JsonValue::ObjectType person = value.getObject();
  person.at("name").getString();  // == "John"
  person.at("age").getNumber();   // == 31
}
```
