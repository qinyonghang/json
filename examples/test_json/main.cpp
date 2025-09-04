#include <gtest/gtest.h>

#include "qlib/json.h"
#include "qlib/string.h"

using namespace qlib;

template <class JsonType>
static void json_dump() {
    using json_type = JsonType;
    auto build_type = json_type::object({{"type", "STRING"}, {"value", "Release"}});
    auto install_prefix =
        json_type::object({{"type", "STRING"}, {"value", "${sourceDir}/install"}});
    auto cmake_msvc_runtime_library =
        json_type::object({{"type", "STRING"}, {"value", "MultiThreaded"}});
    auto windows_condition =
        json_type::object({{"type", "equals"}, {"lhs", "${hostSystemName}"}, {"rhs", "Windows"}});
    auto windows_cacheVariables =
        json_type::object({{"CMAKE_BUILD_TYPE", build_type},
                           {"CMAKE_INSTALL_PREFIX", install_prefix},
                           {"CMAKE_MSVC_RUNTIME_LIBRARY", cmake_msvc_runtime_library}});
    auto windows_configurePresets =
        json_type::object({{"name", "windows"},
                           {"condition", std::move(windows_condition)},
                           {"displayName", "Windows64 Configuration"},
                           {"description", "Windows64 configuration for building the project."},
                           {"binaryDir", "${sourceDir}/build/windows"},
                           {"generator", "Visual Studio 17 2022"},
                           {"architecture", "x64"},
                           {"cacheVariables", std::move(windows_cacheVariables)}});
    auto linux_condition =
        json_type::object({{"type", "equals"}, {"lhs", "${hostSystemName}"}, {"rhs", "Linux"}});
    auto linux_cacheVariables = json_type::object(
        {{"CMAKE_BUILD_TYPE", build_type}, {"CMAKE_INSTALL_PREFIX", install_prefix}});
    auto linux_configurePresets =
        json_type::object({{"name", "linux"},
                           {"condition", std::move(linux_condition)},
                           {"displayName", "Linux Configuration"},
                           {"description", "Linux configuration for building the project."},
                           {"binaryDir", "${sourceDir}/build/linux"},
                           {"generator", "Ninja"},
                           {"cacheVariables", std::move(linux_cacheVariables)}});
    auto dlinux_condition =
        json_type::object({{"type", "equals"}, {"lhs", "${hostSystemName}"}, {"rhs", "Linux"}});
    auto dlinux_cacheVariables = json_type::object(
        {{"CMAKE_BUILD_TYPE", json_type::object({{"type", "STRING"}, {"value", "Debug"}})},
         {"CMAKE_INSTALL_PREFIX", install_prefix}});
    auto dlinux_configurePresets =
        json_type::object({{"name", "dlinux"},
                           {"condition", std::move(dlinux_condition)},
                           {"displayName", "Linux Debug Configuration"},
                           {"description", "Linux Debug configuration for building the project."},
                           {"binaryDir", "${sourceDir}/build/dlinux"},
                           {"generator", "Ninja"},
                           {"cacheVariables", std::move(dlinux_cacheVariables)}});
    auto configurePresets =
        json_type::array({std::move(windows_configurePresets), std::move(linux_configurePresets),
                          std::move(dlinux_configurePresets)});
    auto targets = json_type::array({"ALL_BUILD"});
    auto windows_buildPresets = json_type::object({{"name", "windows"},
                                                   {"configurePreset", "windows"},
                                                   {"configuration", "Release"},
                                                   {"targets", std::move(targets)}});
    auto linux_buildPresets = json_type::object({
        {"name", "linux"},
        {"configurePreset", "linux"},
        {"configuration", "Release"},
    });
    auto dlinux_buildPresets = json_type::object({
        {"name", "dlinux"},
        {"configurePreset", "dlinux"},
        {"configuration", "Debug"},
    });
    auto buildPresets = json_type::array({
        std::move(windows_buildPresets),
        std::move(linux_buildPresets),
        std::move(dlinux_buildPresets),
    });
    auto cmakeMinimumRequired = json_type::object({{"major", 3}, {"minor", 23}, {"patch", 0}});
    auto value = json_type::object({{"version", 10},
                                    {"cmakeMinimumRequired", std::move(cmakeMinimumRequired)},
                                    {"configurePresets", std::move(configurePresets)},
                                    {"buildPresets", std::move(buildPresets)}});
    constexpr auto text =
        "{\"version\":10,\"cmakeMinimumRequired\":{\"major\":3,\"minor\":23,\"patch\":0},"
        "\"configurePresets\":[{\"name\":\"windows\",\"condition\":{\"type\":\"equals\",\"lhs\":"
        "\"${hostSystemName}\",\"rhs\":\"Windows\"},\"displayName\":\"Windows64 "
        "Configuration\",\"description\":\"Windows64 configuration for building the "
        "project.\",\"binaryDir\":\"${sourceDir}/build/windows\",\"generator\":\"Visual Studio "
        "17 2022\",\"architecture\":\"x64\",\"cacheVariables\":{\"CMAKE_BUILD_TYPE\":{\"type\":"
        "\"STRING\",\"value\":\"Release\"},\"CMAKE_INSTALL_PREFIX\":{\"type\":\"STRING\","
        "\"value\":\"${sourceDir}/"
        "install\"},\"CMAKE_MSVC_RUNTIME_LIBRARY\":{\"type\":\"STRING\",\"value\":"
        "\"MultiThreaded\"}}},{\"name\":\"linux\",\"condition\":{\"type\":\"equals\",\"lhs\":\"${"
        "hostSystemName}\","
        "\"rhs\":\"Linux\"},\"displayName\":\"Linux Configuration\",\"description\":\"Linux "
        "configuration for building the project.\",\"binaryDir\":\"${sourceDir}/build/"
        "linux\",\"generator\":\"Ninja\",\"cacheVariables\":{\"CMAKE_BUILD_TYPE\":{\"type\":"
        "\"STRING\","
        "\"value\":\"Release\"},\"CMAKE_INSTALL_PREFIX\":{\"type\":\"STRING\",\"value\":\"${"
        "sourceDir}/"
        "install\"}}},{\"name\":\"dlinux\",\"condition\":{\"type\":\"equals\",\"lhs\":\"${"
        "hostSystemName}\",\"rhs\":\"Linux\"},\"displayName\":\"Linux Debug "
        "Configuration\",\"description\":\"Linux Debug configuration for building the "
        "project.\",\"binaryDir\":\"${sourceDir}/build/"
        "dlinux\",\"generator\":\"Ninja\",\"cacheVariables\":{\"CMAKE_BUILD_TYPE\":{\"type\":"
        "\"STRING\",\"value\":\"Debug\"},\"CMAKE_INSTALL_PREFIX\":{\"type\":\"STRING\",\"value\":"
        "\"${sourceDir}/"
        "install\"}}}],\"buildPresets\":[{\"name\":\"windows\",\"configurePreset\":\"windows\","
        "\"configuration\":\"Release\",\"targets\":[\"ALL_BUILD\"]},{\"name\":\"linux\","
        "\"configurePreset\":\"linux\",\"configuration\":\"Release\"},{\"name\":\"dlinux\","
        "\"configurePreset\":\"dlinux\",\"configuration\":\"Debug\"}]}";
    EXPECT_EQ(value, text);
}

TEST(Json, JsonViewDump) {
    json_dump<qlib::json_view_t>();
}

TEST(Json, JsonCopyDump) {
    json_dump<qlib::json_t>();
}

template <class JsonType>
static void json_parse(JsonType& value) {
    constexpr auto text = R"({
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

    constexpr auto text_len = string::strlen(text);
    constexpr auto begin = text;
    constexpr auto end = begin + text_len;

    using json_type = JsonType;
    using string_t = typename json_type::string_type;
    auto result = json::parse(&value, begin, end);
    EXPECT_EQ(result, 0);

    auto version = value["version"].template get<int32_t>();
    EXPECT_EQ(version, 10);

    auto& cmakeMinimumRequired = value["cmakeMinimumRequired"];
    auto major = cmakeMinimumRequired["major"].template get<int32_t>();
    auto minor = cmakeMinimumRequired["minor"].template get<int32_t>();
    auto patch = cmakeMinimumRequired["patch"].template get<int32_t>();
    EXPECT_EQ(major, 3);
    EXPECT_EQ(minor, 23);
    EXPECT_EQ(patch, 0);

    auto& configurePresets = value["configurePresets"].array();
    {
        auto& windows_configurePreset = configurePresets[0];
        auto windows_name = windows_configurePreset["name"].template get<string_t>();
        EXPECT_EQ(windows_name, "windows");
        auto& windows_condition = windows_configurePreset["condition"];
        auto windows_type = windows_condition["type"].template get<string_t>();
        EXPECT_EQ(windows_type, "equals");
        auto windows_lhs = windows_condition["lhs"].template get<string_t>();
        EXPECT_EQ(windows_lhs, "${hostSystemName}");
        auto windows_rhs = windows_condition["rhs"].template get<string_t>();
        EXPECT_EQ(windows_rhs, "Windows");
        auto windows_displayName = windows_configurePreset["displayName"].template get<string_t>();
        EXPECT_EQ(windows_displayName, "Windows64 Configuration");
        auto windows_description = windows_configurePreset["description"].template get<string_t>();
        EXPECT_EQ(windows_description, "Windows64 configuration for building the project.");
        auto windows_binaryDir = windows_configurePreset["binaryDir"].template get<string_t>();
        EXPECT_EQ(windows_binaryDir, "${sourceDir}/build/windows");
        auto windows_generator = windows_configurePreset["generator"].template get<string_t>();
        EXPECT_EQ(windows_generator, "Visual Studio 17 2022");
        auto windows_architecture =
            windows_configurePreset["architecture"].template get<string_t>();
        EXPECT_EQ(windows_architecture, "x64");
        auto& windows_cacheVariables = windows_configurePreset["cacheVariables"];
        auto& windows_build_type = windows_cacheVariables["CMAKE_BUILD_TYPE"];
        auto windows_build_type_type = windows_build_type["type"].template get<string_t>();
        EXPECT_EQ(windows_build_type_type, "STRING");
        auto windows_build_type_value = windows_build_type["value"].template get<string_t>();
        EXPECT_EQ(windows_build_type_value, "Release");
        auto& windows_cmake_install_prefix = windows_cacheVariables["CMAKE_INSTALL_PREFIX"];
        auto windows_cmake_install_prefix_type =
            windows_cmake_install_prefix["type"].template get<string_t>();
        EXPECT_EQ(windows_cmake_install_prefix_type, "STRING");
        auto windows_cmake_install_prefix_value =
            windows_cmake_install_prefix["value"].template get<string_t>();
        EXPECT_EQ(windows_cmake_install_prefix_value, "${sourceDir}/install");
        auto& windows_cmake_msvc_runtime_library =
            windows_cacheVariables["CMAKE_MSVC_RUNTIME_LIBRARY"];
        auto windows_cmake_msvc_runtime_library_type =
            windows_cmake_msvc_runtime_library["type"].template get<string_t>();
        EXPECT_EQ(windows_cmake_msvc_runtime_library_type, "STRING");
        auto windows_cmake_msvc_runtime_library_value =
            windows_cmake_msvc_runtime_library["value"].template get<string_t>();
        EXPECT_EQ(windows_cmake_msvc_runtime_library_value, "MultiThreaded");
    }
    {
        auto& linux_configurePreset = configurePresets[1];
        auto linux_name = linux_configurePreset["name"].template get<string_t>();
        EXPECT_EQ(linux_name, "linux");
        auto& linux_condition = linux_configurePreset["condition"];
        auto linux_type = linux_condition["type"].template get<string_t>();
        EXPECT_EQ(linux_type, "equals");
        auto linux_lhs = linux_condition["lhs"].template get<string_t>();
        EXPECT_EQ(linux_lhs, "${hostSystemName}");
        auto linux_rhs = linux_condition["rhs"].template get<string_t>();
        EXPECT_EQ(linux_rhs, "Linux");
        auto linux_displayName = linux_configurePreset["displayName"].template get<string_t>();
        EXPECT_EQ(linux_displayName, "Linux Configuration");
        auto linux_description = linux_configurePreset["description"].template get<string_t>();
        EXPECT_EQ(linux_description, "Linux configuration for building the project.");
        auto linux_binaryDir = linux_configurePreset["binaryDir"].template get<string_t>();
        EXPECT_EQ(linux_binaryDir, "${sourceDir}/build/linux");
        auto linux_generator = linux_configurePreset["generator"].template get<string_t>();
        EXPECT_EQ(linux_generator, "Ninja");
        auto& linux_cacheVariables = linux_configurePreset["cacheVariables"];
        auto& linux_build_type = linux_cacheVariables["CMAKE_BUILD_TYPE"];
        auto linux_build_type_type = linux_build_type["type"].template get<string_t>();
        EXPECT_EQ(linux_build_type_type, "STRING");
        auto linux_build_type_value = linux_build_type["value"].template get<string_t>();
        EXPECT_EQ(linux_build_type_value, "Release");
        auto& linux_cmake_install_prefix = linux_cacheVariables["CMAKE_INSTALL_PREFIX"];
        auto linux_cmake_install_prefix_type =
            linux_cmake_install_prefix["type"].template get<string_t>();
        EXPECT_EQ(linux_cmake_install_prefix_type, "STRING");
        auto linux_cmake_install_prefix_value =
            linux_cmake_install_prefix["value"].template get<string_t>();
        EXPECT_EQ(linux_cmake_install_prefix_value, "${sourceDir}/install");
    }
    {
        auto& linux_configurePreset = configurePresets[2];
        auto linux_name = linux_configurePreset["name"].template get<string_t>();
        EXPECT_EQ(linux_name, "dlinux");
        auto& linux_condition = linux_configurePreset["condition"];
        auto linux_type = linux_condition["type"].template get<string_t>();
        EXPECT_EQ(linux_type, "equals");
        auto linux_lhs = linux_condition["lhs"].template get<string_t>();
        EXPECT_EQ(linux_lhs, "${hostSystemName}");
        auto linux_rhs = linux_condition["rhs"].template get<string_t>();
        EXPECT_EQ(linux_rhs, "Linux");
        auto linux_displayName = linux_configurePreset["displayName"].template get<string_t>();
        EXPECT_EQ(linux_displayName, "Linux Debug Configuration");
        auto linux_description = linux_configurePreset["description"].template get<string_t>();
        EXPECT_EQ(linux_description, "Linux Debug configuration for building the project.");
        auto linux_binaryDir = linux_configurePreset["binaryDir"].template get<string_t>();
        EXPECT_EQ(linux_binaryDir, "${sourceDir}/build/dlinux");
        auto linux_generator = linux_configurePreset["generator"].template get<string_t>();
        EXPECT_EQ(linux_generator, "Ninja");
        auto& linux_cacheVariables = linux_configurePreset["cacheVariables"];
        auto& linux_build_type = linux_cacheVariables["CMAKE_BUILD_TYPE"];
        auto linux_build_type_type = linux_build_type["type"].template get<string_t>();
        EXPECT_EQ(linux_build_type_type, "STRING");
        auto linux_build_type_value = linux_build_type["value"].template get<string_t>();
        EXPECT_EQ(linux_build_type_value, "Debug");
        auto& linux_cmake_install_prefix = linux_cacheVariables["CMAKE_INSTALL_PREFIX"];
        auto linux_cmake_install_prefix_type =
            linux_cmake_install_prefix["type"].template get<string_t>();
        EXPECT_EQ(linux_cmake_install_prefix_type, "STRING");
        auto linux_cmake_install_prefix_value =
            linux_cmake_install_prefix["value"].template get<string_t>();
        EXPECT_EQ(linux_cmake_install_prefix_value, "${sourceDir}/install");
    }
    auto& buildPresets = value["buildPresets"].array();
    {
        auto& windows_buildPreset = buildPresets[0];
        auto windows_name = windows_buildPreset["name"].template get<string_t>();
        EXPECT_EQ(windows_name, "windows");
        auto windows_configurePreset =
            windows_buildPreset["configurePreset"].template get<string_t>();
        EXPECT_EQ(windows_configurePreset, "windows");
        auto windows_configuration = windows_buildPreset["configuration"].template get<string_t>();
        EXPECT_EQ(windows_configuration, "Release");
        auto& windows_targets = windows_buildPreset["targets"].array();
        EXPECT_EQ(windows_targets.size(), 1);
        EXPECT_EQ(windows_targets.front().template get<string_t>(), "ALL_BUILD");
    }
    {
        auto& linux_buildPreset = buildPresets[1];
        auto linux_name = linux_buildPreset["name"].template get<string_t>();
        EXPECT_EQ(linux_name, "linux");
        auto linux_configurePreset = linux_buildPreset["configurePreset"].template get<string_t>();
        EXPECT_EQ(linux_configurePreset, "linux");
        auto linux_configuration = linux_buildPreset["configuration"].template get<string_t>();
        EXPECT_EQ(linux_configuration, "Release");
    }
    {
        auto& dlinux_buildPreset = buildPresets[2];
        auto dlinux_name = dlinux_buildPreset["name"].template get<string_t>();
        EXPECT_EQ(dlinux_name, "dlinux");
        auto dlinux_configurePreset =
            dlinux_buildPreset["configurePreset"].template get<string_t>();
        EXPECT_EQ(dlinux_configurePreset, "dlinux");
        auto dlinux_configuration = dlinux_buildPreset["configuration"].template get<string_t>();
        EXPECT_EQ(dlinux_configuration, "Debug");
    }
}

TEST(Json, JsonViewParse) {
    json_view_t value;
    json_parse(value);
}

TEST(Json, JsonParse) {
    json_t value;
    json_parse(value);
}

TEST(Json, JsonViewPoolParse) {
    pool_allocator_t pool;
    json::value<char, memory_policy_t::view, pool_allocator_t> value(pool);
    json_parse(value);
}

TEST(Json, JsonPoolParse) {
    pool_allocator_t pool;
    json::value<char, memory_policy_t::copy, pool_allocator_t> value(pool);
    json_parse(value);
}

int32_t main(int32_t argc, char* argv[]) {
    int32_t result{0};

    do {
        testing::InitGoogleTest(&argc, argv);
        result = RUN_ALL_TESTS();
    } while (false);

    return result;
}
