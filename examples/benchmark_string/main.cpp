// #include <chrono>
#include <benchmark/benchmark.h>
#include <array>
#include <iostream>
#include <string>

#include "qlib/string.h"

using namespace qlib;

constexpr static inline auto text1 = "hello world!";

constexpr static inline auto text2 = R"({
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

static auto benchmark_string1(benchmark::State& state) {
    for (auto _ : state) {
        string_t str(text1);
        benchmark::DoNotOptimize(str);
    }
}

static auto benchmark_string_view1(benchmark::State& state) {
    for (auto _ : state) {
        string_view_t str(text1);
        benchmark::DoNotOptimize(str);
    }
}

static auto benchmark_string_pool1(benchmark::State& state) {
    using string_t = string::value<char, pool_allocator_t<>>;
    pool_allocator_t<> pool;
    for (auto _ : state) {
        string_t str(text1, pool);
        benchmark::DoNotOptimize(str);
    }
}

static auto benchmark_string2(benchmark::State& state) {
    for (auto _ : state) {
        string_t str(text2);
        benchmark::DoNotOptimize(str);
    }
}

static auto benchmark_string_view2(benchmark::State& state) {
    for (auto _ : state) {
        string_view_t str(text2);
        benchmark::DoNotOptimize(str);
    }
}

static auto benchmark_string_pool2(benchmark::State& state) {
    using string_t = string::value<char, pool_allocator_t<>>;
    pool_allocator_t<> pool;
    for (auto _ : state) {
        string_t str(text2, pool);
        benchmark::DoNotOptimize(str);
    }
}

int32_t main(int32_t argc, char* argv[]) {
    int32_t result{0};

    do {
        auto iterations = 10000u;

        if (argc > 1) {
            auto [ptr, ec] =
                std::from_chars(argv[1], argv[1] + string::strlen(argv[1]), iterations);
            if (ec != std::errc{}) {
                result = -1;
                break;
            }
        }

        benchmark::Initialize(&argc, argv);
        benchmark::ReportUnrecognizedArguments(argc, argv);

        // 注册测试用例
        BENCHMARK(benchmark_string1)->Iterations(iterations);
        BENCHMARK(benchmark_string_view1)->Iterations(iterations);
        BENCHMARK(benchmark_string_pool1)->Iterations(iterations);
        BENCHMARK(benchmark_string2)->Iterations(iterations);
        BENCHMARK(benchmark_string_view2)->Iterations(iterations);
        BENCHMARK(benchmark_string_pool2)->Iterations(iterations);

        benchmark::RunSpecifiedBenchmarks();
        benchmark::Shutdown();
    } while (false);

    return result;
}
