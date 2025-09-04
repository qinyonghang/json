// #include <chrono>
#include <benchmark/benchmark.h>
#include <array>
#include <charconv>
#include <cmath>
#include <iostream>

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
    const auto capacity = memory::align_up((string::strlen(text1) + 1), sizeof(void*)) *
        std::distance(state.begin(), state.end());
    using string_t = string::value<char, pool_allocator_t>;
    pool_allocator_t pool(capacity);
    for (auto _ : state) {
        string_t str(text1, pool);
        benchmark::DoNotOptimize(str);
    }
}

static auto benchmark_string_stack1(benchmark::State& state) {
    constexpr auto capacity = 3 * 1024 * 1024;
    const auto need_capacity = memory::align_up((string::strlen(text1) + 1), sizeof(void*)) *
        std::distance(state.begin(), state.end());
    if (need_capacity == 0 || need_capacity > capacity) {
        std::cerr << "capacity must be between 0 and 512KB" << std::endl;
        std::cerr << "Need capacity: " << need_capacity << std::endl;
        std::exit(1);
    }
    using allocator_type = stack_allocator_t<capacity>;
    using string_t = string::value<char, allocator_type>;
    allocator_type allocator;
    for (auto _ : state) {
        string_t str(text1, allocator);
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
    const auto capacity = memory::align_up((string::strlen(text2) + 1), sizeof(void*)) *
        std::distance(state.begin(), state.end());
    using string_t = string::value<char, pool_allocator_t>;
    pool_allocator_t pool(capacity);
    for (auto _ : state) {
        string_t str(text2, pool);
        benchmark::DoNotOptimize(str);
    }
}

static auto benchmark_string_stack2(benchmark::State& state) {
    constexpr auto capacity = 3 * 1024 * 1024;
    const auto need_capacity = memory::align_up((string::strlen(text2) + 1), sizeof(void*)) *
        std::distance(state.begin(), state.end());
    if (need_capacity == 0 || need_capacity > capacity) {
        std::cerr << "capacity must be between 0 and 512KB" << std::endl;
        std::cerr << "Need capacity: " << need_capacity << std::endl;
        std::exit(1);
    }
    using allocator_type = stack_allocator_t<capacity>;
    using string_t = string::value<char, allocator_type>;
    allocator_type allocator;
    for (auto _ : state) {
        string_t str(text2, allocator);
        benchmark::DoNotOptimize(str);
    }
}

static auto benchmark_stl_equal(benchmark::State& state) {
    constexpr auto text1_len = string::strlen(text1);
    constexpr auto text2_len = string::strlen(text2);
    string_t dest1(text1_len);
    string_t dest2(text2_len);
    std::copy(text1, text1 + text1_len, dest1.begin());
    std::copy(text2, text2 + text2_len, dest2.begin());
    bool_t ok1 = std::equal(text1, text1 + text1_len, dest1.begin());
    bool_t ok2 = std::equal(text2, text2 + text2_len, dest2.begin());
    if (!(ok1 && ok2)) {
        throw std::runtime_error("not equal");
    }
    for (auto _ : state) {
        bool_t ok1 = std::equal(text1, text1 + text1_len, dest1.begin());
        bool_t ok2 = std::equal(text2, text2 + text2_len, dest2.begin());
        // assert(ok && ok2);
        benchmark::DoNotOptimize(ok1);
        benchmark::DoNotOptimize(ok2);
    }
}

static auto benchmark_equal(benchmark::State& state) {
    constexpr auto text1_len = string::strlen(text1);
    constexpr auto text2_len = string::strlen(text2);
    string_t dest1(text1_len);
    string_t dest2(text2_len);
    std::copy(text1, text1 + text1_len, dest1.begin());
    std::copy(text2, text2 + text2_len, dest2.begin());
    bool_t ok1 = equal(text1, text1 + text1_len, dest1.begin());
    bool_t ok2 = equal(text2, text2 + text2_len, dest2.begin());
    if (!(ok1 && ok2)) {
        throw std::runtime_error("not equal");
    }
    for (auto _ : state) {
        bool_t ok1 = equal(text1, text1 + text1_len, dest1.begin());
        bool_t ok2 = equal(text2, text2 + text2_len, dest2.begin());
        // assert(ok && ok2);
        benchmark::DoNotOptimize(ok1);
        benchmark::DoNotOptimize(ok2);
    }
}

static auto benchmark_stl_copy(benchmark::State& state) {
    constexpr auto text1_len = string::strlen(text1);
    constexpr auto text2_len = string::strlen(text2);
    string_t dest1(text1_len);
    string_t dest2(text2_len);
    std::copy(text1, text1 + text1_len, dest1.begin());
    std::copy(text2, text2 + text2_len, dest2.begin());
    auto ok1 = std::equal(text1, text1 + text1_len, dest1.begin());
    auto ok2 = std::equal(text2, text2 + text2_len, dest2.begin());
    if (!(ok1 && ok2)) {
        throw std::runtime_error("not equal");
    }
    for (auto _ : state) {
        std::copy(text1, text1 + text1_len, dest1.begin());
        std::copy(text2, text2 + text2_len, dest2.begin());
        benchmark::DoNotOptimize(dest1);
        benchmark::DoNotOptimize(dest2);
    }
}

static auto benchmark_copy(benchmark::State& state) {
    constexpr auto text1_len = string::strlen(text1);
    constexpr auto text2_len = string::strlen(text2);
    string_t dest1(text1_len);
    string_t dest2(text2_len);
    copy(text1, text1 + text1_len, dest1.begin());
    copy(text2, text2 + text2_len, dest2.begin());
    auto ok1 = std::equal(text1, text1 + text1_len, dest1.begin());
    auto ok2 = std::equal(text2, text2 + text2_len, dest2.begin());
    if (!(ok1 && ok2)) {
        throw std::runtime_error("not equal");
    }
    for (auto _ : state) {
        copy(text1, text1 + text1_len, dest1.begin());
        copy(text2, text2 + text2_len, dest2.begin());
        benchmark::DoNotOptimize(dest1);
        benchmark::DoNotOptimize(dest2);
    }
}

static auto benchmark_int64(benchmark::State& state) {
    using value_type = qlib::int64_t;

    value_type value{-1234567890123456789};

    auto str = string_t::from(value);
    throw_if(str != std::to_string(value).c_str());

    for (auto _ : state) {
        auto str = string_t::from(value);
        benchmark::DoNotOptimize(value);
        benchmark::DoNotOptimize(str);
    }
}

static auto benchmark_stl_int64(benchmark::State& state) {
    using value_type = qlib::int64_t;

    value_type value{-1234567890123456789};
    char stream[100];

    auto to_result = std::to_chars(stream, stream + 100, value);
    *to_result.ptr = '\0';
    throw_if(std::to_string(value) != stream || to_result.ec != std::errc{});
    // decltype(value) _value{};
    // auto from_result = std::from_chars(stream, stream + 100, _value);
    // throw_if(_value != value || from_result.ec != std::errc{});

    for (auto _ : state) {
        std::to_chars(stream, stream + 100, value);
        // std::from_chars(stream, stream + 100, _value);
        benchmark::DoNotOptimize(stream);
        benchmark::DoNotOptimize(value);
        // benchmark::DoNotOptimize(_value);
    }
}

static auto benchmark_uint64(benchmark::State& state) {
    using value_type = qlib::uint64_t;

    value_type value{1234567890123456789u};
    char stream[100];

    auto last = qlib::string::to_chars(stream, stream + 100, value);
    *last = '\0';
    throw_if(std::to_string(value) != stream);

    for (auto _ : state) {
        auto last = qlib::string::to_chars(stream, stream + 100, value);
        *last = '\0';
        benchmark::DoNotOptimize(value);
        benchmark::DoNotOptimize(stream);
    }
}

static auto benchmark_from_uint64(benchmark::State& state) {
    using value_type = qlib::uint64_t;

    value_type value{1234567890123456789u};

    auto str = string_t::from(value);
    throw_if(str != std::to_string(value).c_str());

    for (auto _ : state) {
        auto str = string_t::from(value);
        benchmark::DoNotOptimize(value);
        benchmark::DoNotOptimize(str);
    }
}

static auto benchmark_stl_uint64(benchmark::State& state) {
    using value_type = qlib::uint64_t;

    value_type value{1234567890123456789u};
    char stream[100];

    auto to_result = std::to_chars(stream, stream + 100, value);
    *to_result.ptr = '\0';
    throw_if(std::to_string(value) != stream);
    // decltype(value) _value{};
    // std::from_chars(stream, stream + 100, _value);
    // throw_if(_value != value);

    for (auto _ : state) {
        auto to_result = std::to_chars(stream, stream + 100, value);
        *to_result.ptr = '\0';
        // std::from_chars(stream, stream + 100, _value);
        benchmark::DoNotOptimize(stream);
        benchmark::DoNotOptimize(value);
        // benchmark::DoNotOptimize(_value);
    }
}

static auto benchmark_float64(benchmark::State& state) {
    using value_type = qlib::float64_t;

    value_type value{-0.123456};
    auto str = string_t::from(value);
    throw_if(str != std::to_string(value).c_str());

    for (auto _ : state) {
        auto str = string_t::from(value);
        benchmark::DoNotOptimize(value);
        benchmark::DoNotOptimize(str);
    }
}

static auto benchmark_stl_float64(benchmark::State& state) {
    using value_type = qlib::float64_t;

    value_type value{-0.123456};
    char stream[100];

    auto to_result = std::to_chars(stream, stream + 100, value);
    *to_result.ptr = '\0';
    throw_if(std::to_string(value) != stream);
    // decltype(value) _value{};
    // std::from_chars(stream, stream + 100, _value);
    // throw_if(std::abs(value - _value) > 0.000001);

    for (auto _ : state) {
        std::to_chars(stream, stream + 100, value);
        // std::from_chars(stream, stream + 100, _value);
        benchmark::DoNotOptimize(stream);
        benchmark::DoNotOptimize(value);
        // benchmark::DoNotOptimize(_value);
    }
}

int32_t main(int32_t argc, char* argv[]) {
    int32_t result{0};

    do {
        auto iterations = 1000u;

        if (argc > 1) {
            auto [ptr, ec] =
                std::from_chars(argv[1], argv[1] + string::strlen(argv[1]), iterations);
            if (ec != std::errc{}) {
                result = -1;
                break;
            }
        }

        benchmark::Initialize(&argc, argv);
        // benchmark::ReportUnrecognizedArguments(argc, argv);

        BENCHMARK(benchmark_string1)->Iterations(iterations);
        BENCHMARK(benchmark_string_view1)->Iterations(iterations);
        BENCHMARK(benchmark_string_pool1)->Iterations(iterations);
        BENCHMARK(benchmark_string_stack1)->Iterations(iterations);
        BENCHMARK(benchmark_string2)->Iterations(iterations);
        BENCHMARK(benchmark_string_view2)->Iterations(iterations);
        BENCHMARK(benchmark_string_pool2)->Iterations(iterations);
        BENCHMARK(benchmark_string_stack2)->Iterations(iterations);
        BENCHMARK(benchmark_stl_equal)->Iterations(iterations);
        BENCHMARK(benchmark_equal)->Iterations(iterations);
        BENCHMARK(benchmark_stl_copy)->Iterations(iterations);
        BENCHMARK(benchmark_copy)->Iterations(iterations);
        BENCHMARK(benchmark_int64)->Iterations(iterations);
        BENCHMARK(benchmark_stl_int64)->Iterations(iterations);
        BENCHMARK(benchmark_uint64)->Iterations(iterations);
        BENCHMARK(benchmark_from_uint64)->Iterations(iterations);
        BENCHMARK(benchmark_stl_uint64)->Iterations(iterations);
        BENCHMARK(benchmark_float64)->Iterations(iterations);
        BENCHMARK(benchmark_stl_float64)->Iterations(iterations);

        benchmark::RunSpecifiedBenchmarks();
        benchmark::Shutdown();
    } while (false);

    return result;
}
