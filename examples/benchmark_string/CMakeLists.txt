
set(TARGET_NAME benchmark_string)

message(STATUS "Build ${TARGET_NAME}:")
add_executable(${TARGET_NAME} main.cpp)

set_target_properties(${TARGET_NAME} PROPERTIES
    C_STANDARD 17
    C_STANDARD_REQUIRED ON
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

target_link_libraries(${TARGET_NAME} PRIVATE
    qlib::json
    -lbenchmark
)

# target_compile_options(${TARGET_NAME} PRIVATE
#     $<$<CONFIG:Release>:-O3 -march=native -DNDEBUG
#         -funroll-loops
#         -ffast-math          # 如果适用（JSON解析通常不涉及浮点运算）
#         -fomit-frame-pointer
#     >
# )

# include(CheckIPOSupported)
# check_ipo_supported(RESULT ipo_supported OUTPUT ipo_output)

# if(ipo_supported)
#     set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
#     # 设置 LTO 并行作业数
#     if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
#         # 对于 GCC，设置并行 LTO 作业
#         set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto=auto")
#     elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
#         # 对于 Clang，启用并行 LTO
#         set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto=thin")
#     endif()
# endif()

if(MSVC)
    target_compile_options(${TARGET_NAME} PRIVATE /utf-8)
endif()
