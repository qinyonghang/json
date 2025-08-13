
set(TARGET_NAME parse_file)

message(STATUS "Build ${TARGET_NAME}:")
add_executable(${TARGET_NAME} main.cpp)

set_target_properties(${TARGET_NAME} PROPERTIES
    C_STANDARD 17
    C_STANDARD_REQUIRED ON
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

target_link_libraries(${TARGET_NAME} PRIVATE qlib::json)
target_compile_definitions(${TARGET_NAME} PRIVATE RESOUCES_PATH="${CMAKE_SOURCE_DIR}/resources")

if(MSVC)
    target_compile_options(${TARGET_NAME} PRIVATE /utf-8)
endif()
