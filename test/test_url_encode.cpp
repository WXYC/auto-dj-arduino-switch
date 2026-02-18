#include <gtest/gtest.h>
#include "utils.h"

TEST(UrlEncode, AlphanumericPassthrough) {
    EXPECT_EQ(urlEncode("Hello123"), "Hello123");
}

TEST(UrlEncode, SpaceBecomesPlus) {
    EXPECT_EQ(urlEncode("Hello World"), "Hello+World");
}

TEST(UrlEncode, SpecialCharsEncoded) {
    EXPECT_EQ(urlEncode("a&b=c"), "a%26b%3dc");
}

TEST(UrlEncode, UnreservedPassthrough) {
    EXPECT_EQ(urlEncode("a-b_c.d~e"), "a-b_c.d~e");
}

TEST(UrlEncode, SlashEncoded) {
    EXPECT_EQ(urlEncode("artist/band"), "artist%2fband");
}

TEST(UrlEncode, EmptyString) {
    EXPECT_EQ(urlEncode(""), "");
}

TEST(UrlEncode, HighByte) {
    EXPECT_EQ(urlEncode("\xC3\xA9"), "%c3%a9");
}
