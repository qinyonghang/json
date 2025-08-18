
set(TARGET_NAME test_string_cpp14)

message(STATUS "Build ${TARGET_NAME}:")
add_executable(${TARGET_NAME} main.cpp)
add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})

set_target_properties(${TARGET_NAME} PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

target_link_libraries(${TARGET_NAME} PRIVATE qlib::json -lgtest)

if(MSVC)
    target_compile_options(${TARGET_NAME} PRIVATE /utf-8)
endif()
