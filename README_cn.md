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

### JSON文件结构统计

| 文件名 | Size(bytes) | elementCount | objectCount | arrayCount | numberCount | stringCount | trueCount | falseCount | nullCount |
| -------- | ----------- | ------------ | ----------- | ---------- | ----------  | ----------  | --------- | ---------- | --------- |
| canana.json | 2251051 | 167170 | 4 | 56045 | 111126 | 12 | 0 | 0 | 0 |
| citm_catalog.json | 1727204 | 11908 | 10937 | 10451 | 14392 | 26604 | 0 | 0 | 1263 |
| twitter.json | 631514 | 568 | 1264 | 1050 | 2109 | 18099 | 345 | 2446 | 1946 |

### 解析速度对比表 (MB/s)
| Library | canana.json | citm_catalog.json | twitter.json |
| - | - | - | - |
| qjson | 296.474 | 745.672 | 615.806 |
| Nlohmann Json | 115.010 | 363.779 | 175.228 |
| Rapid Json | 625.880 | 1591.488 | 579.652 |

### 序列化速度对比表 (MB/s)
| Library | canana.json | citm_catalog.json | twitter.json |
| - | - | - | - |
| qjson | 1873.272 | 3894.066 | 3740.737 |
| Nlohmann Json | 51.979 | 523.084 | 309.168 |
| Rapid Json | 307.825 | 2491.967 | 1221.620 |

### 统计速度对比表 (MB/s)
| Library | canana.json | citm_catalog.json | twitter.json |
| - | - | - | - |
| qjson | 5963.249 | 5968.080 | 3273.145 |
| Nlohmann Json | 4557.897 | 4601.090 | 1930.316 |
| Rapid Json | 6545.029 | 14707.054 | 13383.526 |

+ 测试环境：Intel i7-12700H @ 2.68GHz, 100,000次迭代

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
