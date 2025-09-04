// #include <chrono>
#include <benchmark/benchmark.h>
#include <array>
#include <fstream>
// #include <iostream>
#include <charconv>
#include <string>

#ifdef HAS_NLOHMANN_JSON
#include "nlohmann/json.hpp"
#endif
#include "qlib/json.h"
#include "qlib/string.h"

constexpr static inline auto resources_path = RESOUCES_PATH;
const static inline auto canada_json = std::string(resources_path) + "/canada.json";
const static inline auto citm_catalog_json = std::string(resources_path) + "/citm_catalog.json";
const static inline auto twitter_json = std::string(resources_path) + "/twitter.json";

using namespace qlib;

// template <class JsonType>
// static auto json_parse(std::string const& filepath, benchmark::State& state) {
//     std::ifstream file{filepath};
//     if (!file.is_open()) {
//         throw std::runtime_error("Failed to open file: " + filepath);
//     }
//     std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
//     auto begin = text.data();
//     auto end = begin + text.size();
//     for (auto _ : state) {
//         JsonType json;
//         auto result = json::parse(&json, begin, end);
//         benchmark::DoNotOptimize(result);
//         benchmark::DoNotOptimize(json);
//     }
// }

template <class JsonType>
static auto json_parse(std::string const& filepath, benchmark::State& state) {
    std::ifstream file{filepath};
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filepath);
    }
    std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    auto begin = text.data();
    auto end = begin + text.size();
    for (auto _ : state) {
        typename JsonType::allocator_type pool;
        JsonType json(pool);
        auto result = json::parse(&json, begin, end);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(json);
    }
}

static auto benchmark_json_parse_canada(benchmark::State& state) {
    json_parse<json_t>(canada_json, state);
}

static auto benchmark_json_view_parse_canada(benchmark::State& state) {
    json_parse<json_view_t>(canada_json, state);
}

static auto benchmark_json_pool_parse_canada(benchmark::State& state) {
    json_parse<json_pool_t>(canada_json, state);
}

static auto benchmark_json_view_pool_parse_canada(benchmark::State& state) {
    json_parse<json_view_pool_t>(canada_json, state);
}

static auto benchmark_json_parse_citm_catalog(benchmark::State& state) {
    json_parse<json_t>(citm_catalog_json, state);
}

static auto benchmark_json_view_parse_citm_catalog(benchmark::State& state) {
    json_parse<json_view_t>(citm_catalog_json, state);
}

static auto benchmark_json_pool_parse_citm_catalog(benchmark::State& state) {
    json_parse<json_pool_t>(citm_catalog_json, state);
}

static auto benchmark_json_view_pool_parse_citm_catalog(benchmark::State& state) {
    json_parse<json_view_pool_t>(citm_catalog_json, state);
}

static auto benchmark_json_parse_twitter(benchmark::State& state) {
    json_parse<json_t>(twitter_json, state);
}

static auto benchmark_json_view_parse_twitter(benchmark::State& state) {
    json_parse<json_view_t>(twitter_json, state);
}

static auto benchmark_json_pool_parse_twitter(benchmark::State& state) {
    json_parse<json_pool_t>(twitter_json, state);
}

static auto benchmark_json_view_pool_parse_twitter(benchmark::State& state) {
    json_parse<json_view_pool_t>(twitter_json, state);
}

#ifdef HAS_NLOHMANN_JSON
static auto benchmark_nlohmann_json_parse_canada(benchmark::State& state) {
    std::ifstream file(canada_json);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + canada_json);
    }
    std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    for (auto _ : state) {
        auto json = nlohmann::json::parse(text);
        benchmark::DoNotOptimize(json);
    }
}

static auto benchmark_nlohmann_json_parse_citm_catalog(benchmark::State& state) {
    std::ifstream file(citm_catalog_json);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + citm_catalog_json);
    }
    std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    for (auto _ : state) {
        auto json = nlohmann::json::parse(text);
        benchmark::DoNotOptimize(json);
    }
}

static auto benchmark_nlohmann_json_parse_twitter(benchmark::State& state) {
    std::ifstream file(twitter_json);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + twitter_json);
    }
    std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
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

        BENCHMARK(benchmark_json_parse_canada)->Iterations(_iterations);
        BENCHMARK(benchmark_json_pool_parse_canada)->Iterations(_iterations);
        BENCHMARK(benchmark_json_view_parse_canada)->Iterations(_iterations);
        BENCHMARK(benchmark_json_view_pool_parse_canada)->Iterations(_iterations);
#ifdef HAS_NLOHMANN_JSON
        BENCHMARK(benchmark_nlohmann_json_parse_canada)->Iterations(_iterations);
#endif
        BENCHMARK(benchmark_json_parse_citm_catalog)->Iterations(_iterations);
        BENCHMARK(benchmark_json_view_parse_citm_catalog)->Iterations(_iterations);
        BENCHMARK(benchmark_json_pool_parse_citm_catalog)->Iterations(_iterations);
        BENCHMARK(benchmark_json_view_pool_parse_citm_catalog)->Iterations(_iterations);
#ifdef HAS_NLOHMANN_JSON
        BENCHMARK(benchmark_nlohmann_json_parse_citm_catalog)->Iterations(_iterations);
#endif
        BENCHMARK(benchmark_json_parse_twitter)->Iterations(_iterations);
        BENCHMARK(benchmark_json_view_parse_twitter)->Iterations(_iterations);
        BENCHMARK(benchmark_json_pool_parse_twitter)->Iterations(_iterations);
        BENCHMARK(benchmark_json_view_pool_parse_twitter)->Iterations(_iterations);
#ifdef HAS_NLOHMANN_JSON
        BENCHMARK(benchmark_nlohmann_json_parse_twitter)->Iterations(_iterations);
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
