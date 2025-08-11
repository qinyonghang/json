
set(TARGET_NAME dump1)

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

if(MSVC)
    target_compile_options(${TARGET_NAME} PRIVATE /utf-8)
endif()
