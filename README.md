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
+ Test environment: Intel i7-12700H @ 2.68GHz, 100,000 Iterations

### JSON File Structure Statistics
---
| Filename | Size(bytes) | elementCount | objectCount | arrayCount | numberCount | stringCount | trueCount | falseCount | nullCount |
| -------- | ----------- | ------------ | ----------- | ---------- | ----------  | ----------  | --------- | ---------- | --------- |
| canana.json | 2251051 | 167170 | 4 | 56045 | 111126 | 12 | 0 | 0 | 0 |
| citm_catalog.json | 1727204 | 11908 | 10937 | 10451 | 14392 | 26604 | 0 | 0 | 1263 |
| twitter.json | 631514 | 568 | 1264 | 1050 | 2109 | 18099 | 345 | 2446 | 1946 |
---
### 1. Parsing Performance (Parse, in MB/s)
| JSON Library            | canada.json | citmcatalog.json | twitter.json |
|-------------------------|-------------|------------------|--------------|
| Nlohmann (C++11) | 103.945 | 368.416 | 181.022 |
| QJson (View+Pool) | 807.360 | 1574.752 | 1387.693 |
| QJson (Copy+Pool) | 695.873 | 1232.005 | 1073.545 |
| QJson (View) | 731.189 | 1030.138 | 930.848 |
| QJson (Copy) | 377.089 | 612.111 | 431.727 |
| RapidJSON_AutoUTF | 387.924 | 394.442 | 262.079 |
| RapidJSON_FullPrec | 246.698 | 1344.645 | 525.990 |
| RapidJSON_Insitu | 564.049 | 1597.663 | 751.883 |
| RapidJSON_Iterative | 550.454 | 1138.348 | 520.085 |
| RapidJSON | 593.850 | 1574.752 | 564.971 |
---
### 2. Stringification Performance (Stringify, in MB/s)
| JSON Library            | canada.json | citmcatalog.json | twitter.json |
|-------------------------|-------------|------------------|--------------|
| Nlohmann (C++11) | 48.947 | 517.821 | 306.805 |
| QJson (View+Pool) | 2172.844 | 4666.261 | 3811.764 |
| QJson (Copy+Pool) | 1953.384 | 4653.079 | 3717.646 |
| QJson (View) | 2036.783 | 4887.804 | 3885.540 |
| QJson (Copy) | 1984.075 | 4692.849 | 3885.540 |
| RapidJSON_AutoUTF | 190.688 | 1359.068 | 562.333 |
| RapidJSON_FullPrec | 287.462 | 2549.830 | 1123.617 |
| RapidJSON_Insitu | 300.416 | 2278.271 | 965.158 |
| RapidJSON_Iterative | 299.870 | 2541.960 | 1129.941 |
| RapidJSON | 302.022 | 2569.719 | 1229.099 |
---
### 3. Statistics Performance (Statistics, in MB/s)
| JSON Library            | canada.json | citmcatalog.json | twitter.json |
|-------------------------|-------------|------------------|--------------|
| Nlohmann (C++11) | 4923.783 | 4537.714 | 1942.770 |
| QJson (View+Pool) | 5740.025 | 10359.686 | 3717.646 |
| QJson (Copy+Pool) | 5261.690 | 9863.414 | 3672.309 |
| QJson (View) | 6925.063 | 11850.288 | 2448.206 |
| QJson (Copy) | 5590.546 | 10981.267 | 2448.206 |
| RapidJSON_AutoUTF | 6446.755 | 14707.054 | 13383.526 |
| RapidJSON_FullPrec | 6168.878 | 13959.238 | 12547.056 |
| RapidJSON_Insitu | 6446.755 | 14839.550 | 13383.526 |
| RapidJSON_Iterative | 6389.195 | 14323.392 | 13092.580 |
| RapidJSON | 6408.267 | 14323.392 | 12814.015 |
---
### Summary & Observations
- **Parsing Performance**:  
  `QJson View` and `RapidJSONInsitu` deliver the best parsing speeds across most files. `Nlohmann` is consistently slower.
- **Stringification Performance**:  
  `QJson` and `RapidJSONFullPrec` perform best on complex files like `twitter.json`. `simdjson` and `SonicJSON` show impressive speeds on `citmcatalog.json`.
- **Statistics Performance**:  
  The `RapidJSON` family dominates, especially on `citmcatalog.json`, reaching speeds above **15,000 MB/s**.
---

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

* C++14
* Standard library only (no external dependencies)

Designed for C++ developers who demand ultimate performance
