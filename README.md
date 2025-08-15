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
| Nlohmann (C++11)          | 113.821     | 395.674          | 188.914      |
| QJson View (C++17)        | 766.703     | 1257.397         | 1164.910     |
| QJson (C++17)             | 433.341     | 655.207          | 551.014      |
| RapidJSONAutoUTF (C++)    | 455.983     | 458.955          | 299.482      |
| RapidJSONFullPrec (C++)   | 258.056     | 1388.862         | 570.321      |
| RapidJSONInsitu (C++)     | 594.838     | 1658.802         | 787.266      |
| RapidJSONIterative (C++)  | 584.473     | 1343.548         | 543.065      |
| RapidJSON (C++)           | 623.698     | 1600.768         | 606.504      |
---
### 2. Stringification Performance (Stringify, in MB/s)
| JSON Library            | canada.json | citmcatalog.json | twitter.json |
|-------------------------|-------------|------------------|--------------|
| Nlohmann (C++11)          | 53.017      | 548.515           | 326.782       |
| QJson View (C++17)        | 1587.847     | 4256.305          | 3885.540      |
| QJson (C++17)             | 1247.397     | 4107.706          | 3787.790      |
| RapidJSONAutoUTF (C++)    | 199.941     | 1404.254          | 575.223      |
| RapidJSONFullPrec (C++)   | 320.605     | 2796.588          | 1375.020      |
| RapidJSONInsitu (C++)     | 320.940     | 2622.914          | 1077.386      |
| RapidJSONIterative (C++)  | 320.557     | 2480.708          | 1158.190      |
| RapidJSON (C++)           | 320.701     | 2722.628          | 1289.633      |
---
### 3. Statistics Performance (Statistics, in MB/s)
| JSON Library            | canada.json | citmcatalog.json | twitter.json |
|-------------------------|-------------|------------------|--------------|
| Nlohmann (C++11)          | 5087.132    | 4733.305         | 2055.490     |
| QJson View (C++17)        | 4073.566    | 8808.503         | 2801.203     |
| QJson (C++17)             | 3601.962    | 8903.730         | 2814.293     |
| RapidJSONAutoUTF (C++)    | 6772.144    | 15394.300        | 13687.697    |
| RapidJSONFullPrec (C++)   | 6836.846    | 15687.525        | 14339.492    |
| RapidJSONInsitu (C++)     | 6772.144    | 14974.455        | 12814.015    |
| RapidJSONIterative (C++)  | 6793.574    | 15838.366        | 14339.492    |
| RapidJSON (C++)           | 3882.043    | 8715.292         | 8364.704     |
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

* C++17
* Standard library only (no external dependencies)

Designed for C++ developers who demand ultimate performance
