// #include <chrono>
#include <benchmark/benchmark.h>
#include <charconv>
#include <iostream>
#include <vector>

#include "qlib/string.h"
#include "qlib/vector.h"

using namespace qlib;

std::vector<int32_t> compare;

bool_t compare_init = []() {
    compare.reserve(1000);
    for (auto i = 0u; i < 1000; ++i) {
        compare.emplace_back(i);
    }
    return True;
}();

static auto benchmark_stl_vector(benchmark::State& state) {
    std::vector<int32_t> vec;

    vec.reserve(1000);
    for (auto i = 0u; i < 1000; ++i) {
        vec.emplace_back(i);
    }

    auto ok = equal(vec.begin(), vec.end(), compare.begin());
    if (!ok) {
        throw exception();
    }

    for (auto _ : state) {
        vector_t<int32_t> vec(1000);
        for (auto i = 0u; i < 1000; ++i) {
            vec.emplace_back(i);
        }
        benchmark::DoNotOptimize(vec);
    }
}

static auto benchmark_vector(benchmark::State& state) {
    vector_t<int32_t> vec(1000);

    for (auto i = 0u; i < 1000; ++i) {
        vec.emplace_back(i);
    }

    auto ok = equal(vec.begin(), vec.end(), compare.begin());
    if (!ok) {
        throw exception();
    }

    for (auto _ : state) {
        vector_t<int32_t> vec(1000);
        for (auto i = 0u; i < 1000; ++i) {
            vec.emplace_back(i);
        }
        benchmark::DoNotOptimize(vec);
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
        // benchmark::ReportUnrecognizedArguments(argc, argv);

        // 注册测试用例
        BENCHMARK(benchmark_stl_vector)->Iterations(iterations);
        BENCHMARK(benchmark_vector)->Iterations(iterations);

        benchmark::RunSpecifiedBenchmarks();
        benchmark::Shutdown();
    } while (false);

    return result;
}
