#include <gtest/gtest.h>
#include "utils.h"

TEST(ParseRadioShowID, ValidLocation) {
    EXPECT_EQ(parseRadioShowID("/playlists/flowsheet?mode=modifyFlowsheet&radioShowID=12345"), 12345);
}

TEST(ParseRadioShowID, TrailingParams) {
    EXPECT_EQ(parseRadioShowID("/playlists/flowsheet?radioShowID=999&other=1"), 999);
}

TEST(ParseRadioShowID, MissingRadioShowID) {
    EXPECT_EQ(parseRadioShowID("/playlists/flowsheet?mode=view"), -1);
}

TEST(ParseRadioShowID, EmptyString) {
    EXPECT_EQ(parseRadioShowID(""), -1);
}

TEST(ParseRadioShowID, NonNumericID) {
    EXPECT_EQ(parseRadioShowID("radioShowID=abc"), -1);
}

TEST(ParseRadioShowID, ZeroID) {
    EXPECT_EQ(parseRadioShowID("radioShowID=0"), -1);
}

TEST(ParseRadioShowID, LargeID) {
    EXPECT_EQ(parseRadioShowID("radioShowID=99999"), 99999);
}
