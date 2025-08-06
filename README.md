# qjson

A fast, efficient JSON parsing and serialization library designed for modern C++ applications.

## Features

🚀Ultra-Fast Performance
* 6x faster parsing than nlohmann/json
* Nearly 3x faster serialization than nlohmann/json
* Zero-copy view mode provides additional 2x performance boost


⚡ Memory Efficient
* Two memory policies: copy and view modes
* View mode eliminates unnecessary string copying
* Compact internal storage reduces memory footprint

🛠 Modern C++ Design
* Fully utilizes C++17 features
* Template-based design with type safety
* constexpr optimizations for compile-time computation
* Exception-safe design

## Performance Benchmarks

Performance comparison with popular JSON libraries on the same hardware:

| Operation | qjson(copy) | qjson(view) | nlohmann::json |
| --------- | ----------- | ----------- | -------------- |
| Parsing   | 4326 ns     | 2017 ns     | 26241 ns       |
| Serializing | 6855 ns     | 4245 ns     | 20174 ns        |

+ Test environment: Intel i7-12700H @ 2.68GHz, 100,000次迭代

## Quick Start

### Building from Source

```bash
git clone git@github.com:qinyonghang/json.git
cd json
mkdir build && cd build
cmake ..
make
make install
```

### Using CMake

```cmake
find_package(qjson REQUIRED)
target_link_libraries(${TARGET_NAME} PRIVATE qlib::json)
```

### Parsing JSON

```cpp
#include "qlib/json.h"

using namespace qlib;
json_view_t json;
result = json::parse(&json, begin, end);
if (0 != result) {
    std::cout << "json::parse return " << result << std::endl;
    break;
}

// get string
auto name = json["name"].get<string_t>();
std::cout << "name: " << name << std::endl;

// get array
auto& array = json["array"].array();
for (auto& item : array) {
    std::cout << item.get<string_t>() << std::endl;
}

// get object
auto& object = json["object"].object();
for (auto& [key, value] : object) {
    std::cout << key << ": " << value.get<string_t>() << std::endl;
}
```

## Memory Policies

### Copy Mode(json_t)

```cpp
using namespace qlib;
json_t json;
result = json::parse(&json, begin, end);
if (0 != result) {
    std::cout << "json::parse return " << result << std::endl;
    break;
}
```

* Creates data copies
* Suitable for scenarios requiring JSON data modification
* Data independent with safe lifetime

### View Mode(json_view_t)

```cpp
using namespace qlib;
json_view_t json;
result = json::parse(&json, begin, end);
if (0 != result) {
    std::cout << "json::parse return " << result << std::endl;
    break;
}
```

* Zero-copy, references original data
* Up to 2x performance improvement
* Suitable for read-only scenarios, requires original data lifetime management

## High-Performance Usage Tips

1. `Prefer view mode`: Use json_view_t for read-only operations for optimal performance
2. `Avoid deep copying`: Use move semantics std::move() when possible

## Dependencies

* C++17
* Standard library only (no external dependencies)

Designed for C++ developers who demand ultimate performance
