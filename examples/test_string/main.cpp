#include <gtest/gtest.h>

#include "qlib/string.h"

TEST(Json, String) {
    using string_t = qlib::string_t;
    EXPECT_EQ(sizeof(string_t), 16);
    string_t str("Hello World!");
    EXPECT_EQ(str, "Hello World!");
}

TEST(Json, StringView) {
    using string_t = qlib::string_view_t;
    EXPECT_EQ(sizeof(string_t), 16);
    string_t str("Hello World!");
    EXPECT_EQ(str, "Hello World!");
}

TEST(Json, StringPool) {
    using string_t = qlib::string::value<char, qlib::pool_allocator_t>;
    EXPECT_EQ(sizeof(string_t), 24);
    qlib::pool_allocator_t pool;
    string_t str("Hello World!", pool);
    EXPECT_EQ(str, "Hello World!");
}

TEST(Json, StringFromInt) {
    using string_t = qlib::string_t;
    auto str = string_t::from(qlib::int64_t(-123456789));
    EXPECT_EQ(str, "-123456789");
}

TEST(Json, StringFromUInt) {
    using string_t = qlib::string_t;
    auto str = string_t::from(qlib::uint64_t(123456789));
    EXPECT_EQ(str, "123456789");
}

TEST(Json, StringFromFloat) {
    using string_t = qlib::string_t;
    auto str = string_t::from(qlib::float64_t(3231.123456));
    EXPECT_EQ(str, "3231.123456");
}

int32_t main(int32_t argc, char* argv[]) {
    int32_t result{0};

    do {
        testing::InitGoogleTest(&argc, argv);
        result = RUN_ALL_TESTS();
    } while (false);

    return result;
}
