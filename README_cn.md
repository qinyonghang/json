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

### 序列化速度对比表 (MB/s)
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

### 统计速度对比表 (MB/s)
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

* C++14
* 标准库（无外部依赖项）

专为追求极致性能的C++开发者设计
