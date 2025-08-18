#include <gtest/gtest.h>

#include "qlib/string.h"

using namespace qlib;

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

int32_t main(int32_t argc, char* argv[]) {
    int32_t result{0};

    do {
        testing::InitGoogleTest(&argc, argv);
        result = RUN_ALL_TESTS();
    } while (false);

    return result;
}
