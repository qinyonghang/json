#include <iostream>

#include "qlib/json.h"
#include "qlib/string.h"

constexpr static inline auto text =
    "{\"version\":10,\"cmakeMinimumRequired\":{\"major\":3,\"minor\":23,\"patch\":0},"
    "\"configurePresets\":[{\"name\":\"windows\",\"condition\":{\"type\":\"equals\",\"lhs\":\"$"
    "{"
    "hostSystemName}\",\"rhs\":\"Windows\"},\"displayName\":\"Windows64 "
    "Configuration\",\"description\":\"Windows64 configuration for building the "
    "project.\",\"binaryDir\":\"${sourceDir}/build/windows\",\"generator\":\"Visual Studio 17 "
    "2022\",\"architecture\":\"x64\",\"cacheVariables\":{\"CMAKE_BUILD_TYPE\":{\"type\":"
    "\"STRING\","
    "\"value\":\"Release\"},\"CMAKE_INSTALL_PREFIX\":{\"type\":\"STRING\",\"value\":\"${"
    "sourceDir}/"
    "install\"},\"CMAKE_MSVC_RUNTIME_LIBRARY\":{\"type\":\"STRING\",\"value\":"
    "\"MultiThreaded\"}}},"
    "{\"name\":\"linux\",\"condition\":{\"type\":\"equals\",\"lhs\":\"${hostSystemName}\","
    "\"rhs\":"
    "\"Linux\"},\"displayName\":\"Linux Configuration\",\"description\":\"Linux configuration "
    "for "
    "building the "
    "project.\",\"binaryDir\":\"${sourceDir}/build/"
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
    "\"${"
    "sourceDir}/"
    "install\"}}}],\"buildPresets\":[{\"name\":\"windows\",\"configurePreset\":\"windows\","
    "\"configuration\":\"Release\",\"targets\":[\"ALL_BUILD\"]},{\"name\":\"linux\","
    "\"configurePreset\":\"linux\",\"configuration\":\"Release\"},{\"name\":\"dlinux\","
    "\"configurePreset\":\"dlinux\",\"configuration\":\"Debug\"}]}";

int32_t main(int32_t argc, char* argv[]) {
    int32_t result{0};

    do {
        using namespace qlib;

        constexpr auto text_len = string::strlen(text);
        constexpr auto begin = text;
        constexpr auto end = begin + text_len;

        using json_type = json_view_t;
        auto build_type = json_type::object({{"type", "STRING"}, {"value", "Release"}});
        auto install_prefix =
            json_type::object({{"type", "STRING"}, {"value", "${sourceDir}/install"}});
        auto cmake_msvc_runtime_library =
            json_type::object({{"type", "STRING"}, {"value", "MultiThreaded"}});
        auto windows_condition = json_type::object(
            {{"type", "equals"}, {"lhs", "${hostSystemName}"}, {"rhs", "Windows"}});
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
        auto dlinux_configurePresets = json_type::object(
            {{"name", "dlinux"},
             {"condition", std::move(dlinux_condition)},
             {"displayName", "Linux Debug Configuration"},
             {"description", "Linux Debug configuration for building the project."},
             {"binaryDir", "${sourceDir}/build/dlinux"},
             {"generator", "Ninja"},
             {"cacheVariables", std::move(dlinux_cacheVariables)}});
        auto configurePresets = json_type::array({std::move(windows_configurePresets),
                                                  std::move(linux_configurePresets),
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
        auto value = json_type::object({});
        value["version"] = 10;
        value["cmakeMinimumRequired"] =
            json_type::object({{"major", 3}, {"minor", 23}, {"patch", 0}});
        value["configurePresets"] = std::move(configurePresets);
        value["buildPresets"] = json_type::array({
            std::move(windows_buildPresets),
            std::move(linux_buildPresets),
            std::move(dlinux_buildPresets),
        });
        std::cout << "json text: " << value << std::endl;
        std::cout << "equal: " << (value == text) << std::endl;
    } while (false);

    return result;
}
