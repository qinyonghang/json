// #include <chrono>
#include <benchmark/benchmark.h>
#include <array>
#include <charconv>
#include <iostream>
#include <string>

#ifdef HAS_NLOHMANN_JSON
#include "nlohmann/json.hpp"
#endif
#include "qlib/json.h"
#include "qlib/string.h"

using namespace qlib;

constexpr static inline auto text = R"({
    "version": 10,
    "cmakeMinimumRequired": {
        "major": 3,
        "minor": 23,
        "patch": 0
    },
    "configurePresets": [
        {
            "name": "windows",
            "condition": {
                "type": "equals",
                "lhs": "${hostSystemName}",
                "rhs": "Windows"
            },
            "displayName": "Windows64 Configuration",
            "description": "Windows64 configuration for building the project.",
            "binaryDir": "${sourceDir}/build/windows",
            "generator": "Visual Studio 17 2022",
            "architecture": "x64",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": {
                    "type": "STRING",
                    "value": "Release"
                },
                "CMAKE_INSTALL_PREFIX": {
                    "type": "STRING",
                    "value": "${sourceDir}/install"
                },
                "CMAKE_MSVC_RUNTIME_LIBRARY": {
                    "type": "STRING",
                    "value": "MultiThreaded"
                }
            }
        },
        {
            "name": "linux",
            "condition": {
                "type": "equals",
                "lhs": "${hostSystemName}",
                "rhs": "Linux"
            },
            "displayName": "Linux Configuration",
            "description": "Linux configuration for building the project.",
            "binaryDir": "${sourceDir}/build/linux",
            "generator": "Ninja",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": {
                    "type": "STRING",
                    "value": "Release"
                },
                "CMAKE_INSTALL_PREFIX": {
                    "type": "STRING",
                    "value": "${sourceDir}/install"
                }
            }
        },
        {
            "name": "dlinux",
            "condition": {
                "type": "equals",
                "lhs": "${hostSystemName}",
                "rhs": "Linux"
            },
            "displayName": "Linux Debug Configuration",
            "description": "Linux Debug configuration for building the project.",
            "binaryDir": "${sourceDir}/build/dlinux",
            "generator": "Ninja",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": {
                    "type": "STRING",
                    "value": "Debug"
                },
                "CMAKE_INSTALL_PREFIX": {
                    "type": "STRING",
                    "value": "${sourceDir}/install"
                }
            }
        }
    ],
    "buildPresets": [
        {
            "name": "windows",
            "configurePreset": "windows",
            "configuration": "Release",
            "targets": [
                "ALL_BUILD"
            ]
        },
        {
            "name": "linux",
            "configurePreset": "linux",
            "configuration": "Release"
        },
        {
            "name": "dlinux",
            "configurePreset": "dlinux",
            "configuration": "Debug"
        }
    ]
})";

constexpr static inline auto text_len = string::strlen(text);

constexpr static inline auto begin = text;
constexpr static inline auto end = begin + text_len;

#define PARSE_WITH_JSON(json_t)                                                                    \
    do {                                                                                           \
        using json_type = json_t;                                                                  \
        using string_t = typename json_type::string_type;                                          \
                                                                                                   \
        json_type json;                                                                            \
        auto result = json::parse(&json, begin, end);                                              \
        if (0 != result) {                                                                         \
            std::cout << "json::parse return " << result << std::endl;                             \
            std::exit(0);                                                                          \
        }                                                                                          \
                                                                                                   \
        auto version = json["version"].get<int32_t>();                                             \
        assert(version == 10);                                                                     \
                                                                                                   \
        auto& cmakeRequired = json["cmakeMinimumRequired"];                                        \
        auto major = cmakeRequired["major"].get<int32_t>();                                        \
        auto minor = cmakeRequired["minor"].get<int32_t>();                                        \
        auto patch = cmakeRequired["patch"].get<int32_t>();                                        \
        assert(major == 3 && minor == 23 && patch == 0);                                           \
                                                                                                   \
        auto& configurePresets = json["configurePresets"].array();                                 \
        std::array<string_t, 3u> presetNames{"windows", "linux", "dlinux"};                        \
        for (size_t i = 0u; i < presetNames.size(); ++i) {                                         \
            auto& configurePreset = configurePresets[i];                                           \
            auto name = configurePreset["name"].get<string_t>();                                   \
            assert(name == presetNames[i]);                                                        \
        }                                                                                          \
                                                                                                   \
        auto& buildPresets = json["buildPresets"].array();                                         \
        for (size_t i = 0u; i < presetNames.size(); ++i) {                                         \
            auto& buildPreset = buildPresets[i];                                                   \
            auto name = buildPreset["name"].get<string_t>();                                       \
            assert(name == presetNames[i]);                                                        \
        }                                                                                          \
                                                                                                   \
        auto fakeValue = json["fakeValue"].get<int32_t>(0u);                                       \
        assert(fakeValue == 0);                                                                    \
                                                                                                   \
        try {                                                                                      \
            auto fakeValue = json["fakeValue"].get<int32_t>();                                     \
            (void)fakeValue;                                                                       \
        } catch (json::not_number const& _) {                                                      \
        }                                                                                          \
                                                                                                   \
        benchmark::DoNotOptimize(json);                                                            \
    } while (false)

static auto benchmark_json_parse(benchmark::State& state) {
    for (auto _ : state) {
        json_t json;
        auto result = json::parse(&json, begin, end);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(json);
    }
}

static auto benchmark_json_pool_parse(benchmark::State& state) {
    for (auto _ : state) {
        pool_allocator_t pool;
        json::value<char, memory_policy_t::copy, pool_allocator_t> json(pool);
        auto result = json::parse(&json, begin, end);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(json);
    }
}

static auto benchmark_json_parse_view(benchmark::State& state) {
    for (auto _ : state) {
        json_view_t json;
        auto result = json::parse(&json, begin, end);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(json);
    }
}

static auto benchmark_json_view_pool_parse(benchmark::State& state) {
    for (auto _ : state) {
        pool_allocator_t pool;
        json::value<char, memory_policy_t::view, pool_allocator_t> json(pool);
        auto result = json::parse(&json, begin, end);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(json);
    }
}

#ifdef HAS_NLOHMANN_JSON
static auto benchmark_nlohmann_json(benchmark::State& state) {
    for (auto _ : state) {
        auto json = nlohmann::json::parse(text);
        benchmark::DoNotOptimize(json);
    }
}
#endif

#define DECLARE_VALUE()                                                                            \
    auto build_type = json_type::object({{"type", "STRING"}, {"value", "Release"}});               \
    auto install_prefix =                                                                          \
        json_type::object({{"type", "STRING"}, {"value", "${sourceDir}/install"}});                \
    auto cmake_msvc_runtime_library =                                                              \
        json_type::object({{"type", "STRING"}, {"value", "MultiThreaded"}});                       \
    auto windows_condition =                                                                       \
        json_type::object({{"type", "equals"}, {"lhs", "${hostSystemName}"}, {"rhs", "Windows"}}); \
    auto windows_cacheVariables =                                                                  \
        json_type::object({{"CMAKE_BUILD_TYPE", build_type},                                       \
                           {"CMAKE_INSTALL_PREFIX", install_prefix},                               \
                           {"CMAKE_MSVC_RUNTIME_LIBRARY", cmake_msvc_runtime_library}});           \
    auto windows_configurePresets =                                                                \
        json_type::object({{"name", "windows"},                                                    \
                           {"condition", std::move(windows_condition)},                            \
                           {"displayName", "Windows64 Configuration"},                             \
                           {"description", "Windows64 configuration for building the project."},   \
                           {"binaryDir", "${sourceDir}/build/windows"},                            \
                           {"generator", "Visual Studio 17 2022"},                                 \
                           {"architecture", "x64"},                                                \
                           {"cacheVariables", std::move(windows_cacheVariables)}});                \
    auto linux_condition =                                                                         \
        json_type::object({{"type", "equals"}, {"lhs", "${hostSystemName}"}, {"rhs", "Linux"}});   \
    auto linux_cacheVariables = json_type::object(                                                 \
        {{"CMAKE_BUILD_TYPE", build_type}, {"CMAKE_INSTALL_PREFIX", install_prefix}});             \
    auto linux_configurePresets =                                                                  \
        json_type::object({{"name", "linux"},                                                      \
                           {"condition", std::move(linux_condition)},                              \
                           {"displayName", "Linux Configuration"},                                 \
                           {"description", "Linux configuration for building the project."},       \
                           {"binaryDir", "${sourceDir}/build/linux"},                              \
                           {"generator", "Ninja"},                                                 \
                           {"cacheVariables", std::move(linux_cacheVariables)}});                  \
    auto dlinux_condition =                                                                        \
        json_type::object({{"type", "equals"}, {"lhs", "${hostSystemName}"}, {"rhs", "Linux"}});   \
    auto dlinux_cacheVariables = json_type::object(                                                \
        {{"CMAKE_BUILD_TYPE", json_type::object({{"type", "STRING"}, {"value", "Debug"}})},        \
         {"CMAKE_INSTALL_PREFIX", install_prefix}});                                               \
    auto dlinux_configurePresets =                                                                 \
        json_type::object({{"name", "dlinux"},                                                     \
                           {"condition", std::move(dlinux_condition)},                             \
                           {"displayName", "Linux Debug Configuration"},                           \
                           {"description", "Linux Debug configuration for building the project."}, \
                           {"binaryDir", "${sourceDir}/build/dlinux"},                             \
                           {"generator", "Ninja"},                                                 \
                           {"cacheVariables", std::move(dlinux_cacheVariables)}});                 \
    auto configurePresets =                                                                        \
        json_type::array({std::move(windows_configurePresets), std::move(linux_configurePresets),  \
                          std::move(dlinux_configurePresets)});                                    \
    auto targets = json_type::array({"ALL_BUILD"});                                                \
    auto windows_buildPresets = json_type::object({{"name", "windows"},                            \
                                                   {"configurePreset", "windows"},                 \
                                                   {"configuration", "Release"},                   \
                                                   {"targets", std::move(targets)}});              \
    auto linux_buildPresets = json_type::object({                                                  \
        {"name", "linux"},                                                                         \
        {"configurePreset", "linux"},                                                              \
        {"configuration", "Release"},                                                              \
    });                                                                                            \
    auto dlinux_buildPresets = json_type::object({                                                 \
        {"name", "dlinux"},                                                                        \
        {"configurePreset", "dlinux"},                                                             \
        {"configuration", "Debug"},                                                                \
    });                                                                                            \
    auto buildPresets = json_type::array({                                                         \
        std::move(windows_buildPresets),                                                           \
        std::move(linux_buildPresets),                                                             \
        std::move(dlinux_buildPresets),                                                            \
    });                                                                                            \
    auto cmakeMinimumRequired = json_type::object({{"major", 3}, {"minor", 23}, {"patch", 0}});    \
    auto value = json_type::object({{"version", 10},                                               \
                                    {"cmakeMinimumRequired", std::move(cmakeMinimumRequired)},     \
                                    {"configurePresets", std::move(configurePresets)},             \
                                    {"buildPresets", std::move(buildPresets)}});

static auto benchmark_json_to(benchmark::State& state) {
    for (auto _ : state) {
        using json_type = json_t;
        DECLARE_VALUE();
        benchmark::DoNotOptimize(value);
        auto text = value.to();
        benchmark::DoNotOptimize(text);
    }
}

static auto benchmark_json_view_to(benchmark::State& state) {
    for (auto _ : state) {
        using json_type = json_view_t;
        DECLARE_VALUE();
        benchmark::DoNotOptimize(value);
        auto text = value.to();
        benchmark::DoNotOptimize(text);
    }
}

#ifdef HAS_NLOHMANN_JSON
static auto benchmark_nlohmann_json_to(benchmark::State& state) {
    for (auto _ : state) {
        using json_type = nlohmann::json;
        DECLARE_VALUE();
        benchmark::DoNotOptimize(value);
        auto text = value.dump();
        benchmark::DoNotOptimize(text);
    }
}
#endif

int32_t main(int32_t argc, char* argv[]) {
    int32_t result{0};

    do {
        auto _iterations = 1000u;

        if (argc > 1) {
            auto [ptr, ec] =
                std::from_chars(argv[1], argv[1] + string::strlen(argv[1]), _iterations);
            if (ec != std::errc{}) {
                result = -1;
                break;
            }
        }

        benchmark::Initialize(&argc, argv);
        benchmark::ReportUnrecognizedArguments(argc, argv);

        // 注册测试用例
        BENCHMARK(benchmark_json_parse)->Iterations(_iterations);
        BENCHMARK(benchmark_json_pool_parse)->Iterations(_iterations);
        BENCHMARK(benchmark_json_parse_view)->Iterations(_iterations);
        BENCHMARK(benchmark_json_view_pool_parse)->Iterations(_iterations);
#ifdef HAS_NLOHMANN_JSON
        BENCHMARK(benchmark_nlohmann_json)->Iterations(_iterations);
#endif
        BENCHMARK(benchmark_json_to)->Iterations(_iterations);
        BENCHMARK(benchmark_json_view_to)->Iterations(_iterations);
#ifdef HAS_NLOHMANN_JSON
        BENCHMARK(benchmark_nlohmann_json_to)->Iterations(_iterations);
#endif

        benchmark::RunSpecifiedBenchmarks();
        benchmark::Shutdown();
    } while (false);

    return result;
}
