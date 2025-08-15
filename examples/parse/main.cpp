#include <iostream>
#include <functional>

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

        json_view_t json;
        result = json::parse(&json, begin, end);
        if (0 != result) {
            std::cout << "json::parse return " << result << std::endl;
            break;
        }

        std::cout << "json text: " << json << std::endl;
        std::cout << "equal: " << (json == text) << std::endl;

        auto version = json["version"].get<int32_t>();
        std::cout << "version: " << version << std::endl;

        auto& cmakeMinimumRequired = json["cmakeMinimumRequired"];
        auto major = cmakeMinimumRequired["major"].get<int32_t>();
        auto minor = cmakeMinimumRequired["minor"].get<int32_t>();
        auto patch = cmakeMinimumRequired["patch"].get<int32_t>();
        std::cout << "cmakeMinimumRequired: " << major << "." << minor << "." << patch << std::endl;

        auto& configurePresets = json["configurePresets"].array();
        std::cout << "configurePresets: " << std::endl;
        for (auto& configurePreset : configurePresets) {
            auto name = configurePreset["name"].get<string_t>();
            auto displayName = configurePreset["displayName"].get<string_t>();
            std::cout << "    name: " << name << std::endl;
            std::cout << "    displayName: " << displayName << std::endl;
        }

        auto& buildPresets = json["buildPresets"].array();
        std::cout << "buildPresets: " << std::endl;
        for (auto& buildPreset : buildPresets) {
            auto name = buildPreset["name"].get<string_t>();
            auto configurePreset = buildPreset["configurePreset"].get<string_t>();
            auto configuration = buildPreset["configuration"].get<string_t>();
            std::cout << "    name: " << name << std::endl;
            std::cout << "    configurePreset: " << configurePreset << std::endl;
            std::cout << "    configuration: " << configuration << std::endl;
            auto& targets = buildPreset["targets"];
            if (targets) {
                std::cout << "    targets: " << std::endl;
                for (auto& target : targets.array()) {
                    auto name = target.get<string_t>();
                    std::cout << "        " << name << std::endl;
                }
            }
        }
    } while (false);

    return result;
}
