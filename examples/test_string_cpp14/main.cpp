#include <gtest/gtest.h>

#include "qlib/string.h"

using namespace qlib;

constexpr static auto text1 = "hello world!";

constexpr static auto text2 = R"({
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

TEST(Json, String) {
    EXPECT_EQ(sizeof(string_t), 16);
    string_t str("Hello World!");
    EXPECT_EQ(str, "Hello World!");
}

TEST(Json, StringView) {
    EXPECT_EQ(sizeof(string_view_t), 16);
    string_view_t str("Hello World!");
    EXPECT_EQ(str, "Hello World!");
}

TEST(Json, StringPool) {
    using string_t = string::value<char, pool_allocator_t>;
    EXPECT_EQ(sizeof(string_t), 24);
    pool_allocator_t pool;
    string_t str("Hello World!", pool);
    EXPECT_EQ(str, "Hello World!");
}

TEST(Json, StringStack) {
    using allocator_type = stack_allocator_t<>;
    using string_t = string::value<char, allocator_type>;
    EXPECT_EQ(sizeof(string_t), 24);
    allocator_type allocator;
    string_t str1(text1, allocator);
    EXPECT_EQ(str1, text1);
    string_t str2(text2, allocator);
    EXPECT_EQ(str2, text2);
}

int32_t main(int32_t argc, char* argv[]) {
    int32_t result{0};

    do {
        testing::InitGoogleTest(&argc, argv);
        result = RUN_ALL_TESTS();
    } while (false);

    return result;
}
