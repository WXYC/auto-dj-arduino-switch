#include <gtest/gtest.h>
#include "utils.h"

TEST(CurrentHourMs, TruncatesToHour) {
    // 1705347000 = Mon Jan 15 2024 17:30:00 UTC -> truncates to 17:00:00 = 1705345200
    EXPECT_EQ(currentHourMs(1705347000UL), 1705345200000UL);
}

TEST(CurrentHourMs, ExactHour) {
    // Already on the hour boundary
    EXPECT_EQ(currentHourMs(1705345200UL), 1705345200000UL);
}

TEST(CurrentHourMs, ZeroEpoch) {
    // Guard: 0 means no NTP time, should return 0
    EXPECT_EQ(currentHourMs(0UL), 0UL);
}

TEST(CurrentHourMs, OneSecondPastHour) {
    EXPECT_EQ(currentHourMs(1705345201UL), 1705345200000UL);
}

TEST(CurrentHourMs, LastSecondOfHour) {
    // 1705348799 = one second before the next hour (18:00:00 = 1705348800)
    EXPECT_EQ(currentHourMs(1705348799UL), 1705345200000UL);
}
