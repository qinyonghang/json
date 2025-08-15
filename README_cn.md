# qjson

一个专为现代C++设计的快速、高效的JSON解析和序列化库。

## 特性

🚀 极致性能
* 比nlohmann/json快6倍以上的解析速度
* 比nlohmann/json快近3倍的序列化速度
* 零拷贝视图模式，进一步提升性能达2倍以上

⚡ 内存优化
* 支持两种内存策略：拷贝(copy)和视图(view)
* 视图模式避免不必要的字符串拷贝，显著提升性能
* 紧凑的内部存储结构，减少内存占用

🛠 现代C++设计
* 全面使用C++17特性
* 模板化设计，类型安全
* constexpr优化，编译时计算
* 异常安全设计

## 性能基准测试

在相同硬件环境下与流行JSON库的性能对比
+ 测试环境：Intel i7-12700H @ 2.68GHz, 100,000次迭代

### JSON文件结构统计

| 文件名 | Size(bytes) | elementCount | objectCount | arrayCount | numberCount | stringCount | trueCount | falseCount | nullCount |
| -------- | ----------- | ------------ | ----------- | ---------- | ----------  | ----------  | --------- | ---------- | --------- |
| canana.json | 2251051 | 167170 | 4 | 56045 | 111126 | 12 | 0 | 0 | 0 |
| citm_catalog.json | 1727204 | 11908 | 10937 | 10451 | 14392 | 26604 | 0 | 0 | 1263 |
| twitter.json | 631514 | 568 | 1264 | 1050 | 2109 | 18099 | 345 | 2446 | 1946 |

### 解析速度对比表 (MB/s)
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

### 序列化速度对比表 (MB/s)
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

### 统计速度对比表 (MB/s)
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

### 📝 总结与观察
- 解析性能：QJson View 和 RapidJSON_Insitu 在多数文件上表现优异，Nlohmann 整体较慢。
- 序列化性能：QJson 系列和 RapidJSON_FullPrec 在较复杂文件（如 twitter.json）上表现最佳。
- 统计性能：RapidJSON 系列库表现极为出色，尤其在 citm_catalog.json 上统计速度可达 15,000+ MB/s。

## 快速开始

### 从源码构建

```bash
git clone git@github.com:qinyonghang/json.git
cd json
mkdir build && cd build
cmake ..
make
make install
```

### 使用 CMake

```cmake
find_package(qjson REQUIRED)
target_link_libraries(${TARGET_NAME} PRIVATE qlib::json)
```

### 解析 JSON

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

## 内存策略

### Copy模式(json_t)

```cpp
using namespace qlib;
json_t json;
result = json::parse(&json, begin, end);
if (0 != result) {
    std::cout << "json::parse return " << result << std::endl;
    break;
}
```

* 创建数据副本
* 适用于需要修改JSON数据的场景
* 数据与生命周期无关

### View模式(json_view_t)

```cpp
using namespace qlib;
json_view_t json;
result = json::parse(&json, begin, end);
if (0 != result) {
    std::cout << "json::parse return " << result << std::endl;
    break;
}
```

* 零拷贝，引用原始数据
* 相对Copy模式，性能提升高达2倍
* 适用于只读场景，需要管理原始数据生命周期

## 高性能使用建议
1. `优先使用View模式`: 对于只读操作，请使用json_view_t，以获得最佳性能
2. `避免深度拷贝`: 当可能时，请使用移动语义（std::move()）

## 依赖项

* C++17
* 标准库（无外部依赖项）

专为追求极致性能的C++开发者设计
