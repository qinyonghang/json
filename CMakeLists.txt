cmake_minimum_required(VERSION 3.22)

if(NOT DEFINED PROJECT_NAME)
project(qjson C CXX)
endif()

add_library(qjson INTERFACE)
add_library(qlib::json ALIAS qjson)
set_target_properties(qjson PROPERTIES 
    INTERFACE_INCLUDE_DIRECTORIES 
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>;$<INSTALL_INTERFACE:include>"
)

option(BUILD_EXAMPLES "Build examples" ON)

if (BUILD_EXAMPLES)
file(GLOB examples ${CMAKE_CURRENT_LIST_DIR}/examples/*)
foreach(example ${examples})
    if (IS_DIRECTORY ${example})
        add_subdirectory(${example})
    endif ()
endforeach()
endif()

install(TARGETS qjson EXPORT qjson)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include DESTINATION .)
# 安装导出文件
install(EXPORT qjson 
        FILE qjson-targets.cmake
        # NAMESPACE qlib::
        DESTINATION lib/cmake/qjson)

# 创建配置文件
include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/qjson-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/qjson-config.cmake"
    INSTALL_DESTINATION lib/cmake/qjson
)

# 安装配置文件
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/qjson-config.cmake"
        DESTINATION lib/cmake/qjson)
