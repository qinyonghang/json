#include <gtest/gtest.h>

#include "qlib/vector.h"

using namespace qlib;

TEST(Json, Vector) {
    EXPECT_EQ(sizeof(vector_t<uint32_t>), 16);
    vector_t<uint32_t> vec(256u);
    for (auto i = 0u; i < 256u; ++i) {
        vec.emplace_back(i);
    }
    EXPECT_EQ(vec.size(), 256u);
    uint32_t size{0u};
    for (auto& value : vec) {
        EXPECT_EQ(value, size);
        size++;
    }
}

TEST(Json, VectorPool) {
    EXPECT_EQ(sizeof(vector_t<uint32_t, pool_allocator_t>), 24);
    pool_allocator_t pool;
    vector_t<uint32_t, pool_allocator_t> vec(256u, pool);
    for (auto i = 0u; i < 256u; ++i) {
        vec.emplace_back(i);
    }
    EXPECT_EQ(vec.size(), 256u);
    uint32_t size{0u};
    for (auto& value : vec) {
        EXPECT_EQ(value, size);
        size++;
    }
}

int32_t main(int32_t argc, char* argv[]) {
    int32_t result{0};

    do {
        testing::InitGoogleTest(&argc, argv);
        result = RUN_ALL_TESTS();
    } while (false);

    return result;
}
