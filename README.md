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

### JSON File Structure Statistics

| Filename | Size(bytes) | elementCount | objectCount | arrayCount | numberCount | stringCount | trueCount | falseCount | nullCount |
| -------- | ----------- | ------------ | ----------- | ---------- | ----------  | ----------  | --------- | ---------- | --------- |
| canana.json | 2251051 | 167170 | 4 | 56045 | 111126 | 12 | 0 | 0 | 0 |
| citm_catalog.json | 1727204 | 11908 | 10937 | 10451 | 14392 | 26604 | 0 | 0 | 1263 |
| twitter.json | 631514 | 568 | 1264 | 1050 | 2109 | 18099 | 345 | 2446 | 1946 |

### Parsing Performance (MB/s)
| Library | canana.json | citm_catalog.json | twitter.json |
| - | - | - | - |
| qjson | 296.474 | 745.672 | 615.806 |
| Nlohmann Json | 115.010 | 363.779 | 175.228 |
| Rapid Json | 625.880 | 1591.488 | 579.652 |

### Serialization Performance (MB/s)
| Library | canana.json | citm_catalog.json | twitter.json |
| - | - | - | - |
| qjson | 1873.272 | 3894.066 | 3740.737 |
| Nlohmann Json | 51.979 | 523.084 | 309.168 |
| Rapid Json | 307.825 | 2491.967 | 1221.620 |

### Statistics Performance (MB/s)
| Library | canana.json | citm_catalog.json | twitter.json |
| - | - | - | - |
| qjson | 5963.249 | 5968.080 | 3273.145 |
| Nlohmann Json | 4557.897 | 4601.090 | 1930.316 |
| Rapid Json | 6545.029 | 14707.054 | 13383.526 |

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
