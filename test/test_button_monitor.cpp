#include <gtest/gtest.h>
#include "button_monitor.h"
#include "test_helpers.h"

// All tests use the parameterized update(millis, reading) which has zero
// hardware dependencies.

static const int BUTTON_PIN = 5;
static const unsigned long DEBOUNCE_MS = 50;

class ButtonMonitorTest : public ::testing::Test {
protected:
    ButtonMonitor button{BUTTON_PIN, DEBOUNCE_MS};
};

TEST_F(ButtonMonitorTest, InitialState_ReleasedNotPressed) {
    EXPECT_FALSE(button.pressed());
    EXPECT_FALSE(button.isDown());
}

TEST_F(ButtonMonitorTest, StableHigh_NoPress) {
    for (unsigned long t = 0; t < 200; t++) {
        button.update(t, HIGH);
        EXPECT_FALSE(button.pressed());
        EXPECT_FALSE(button.isDown());
    }
}

TEST_F(ButtonMonitorTest, PressBeyondDebounce_FiresPressedOnce) {
    button.update(0, HIGH);
    button.update(100, LOW); // press begins, resets debounce timer

    for (unsigned long t = 101; t <= 100 + DEBOUNCE_MS; t++) {
        button.update(t, LOW);
        EXPECT_FALSE(button.pressed()) << "Should not fire at t=" << t;
    }

    // One ms beyond debounce -> a single press edge
    button.update(100 + DEBOUNCE_MS + 1, LOW);
    EXPECT_TRUE(button.pressed());
    EXPECT_TRUE(button.isDown());

    // pressed() is a one-shot: clears on the next update
    button.update(100 + DEBOUNCE_MS + 2, LOW);
    EXPECT_FALSE(button.pressed());
}

TEST_F(ButtonMonitorTest, BounceRejected_WithinDebounce) {
    button.update(0, HIGH);
    button.update(100, LOW);  // start timer
    button.update(120, LOW);  // 20ms < 50ms
    button.update(130, HIGH); // bounce back within window
    for (unsigned long t = 131; t <= 200; t++) {
        button.update(t, HIGH);
    }
    EXPECT_FALSE(button.pressed());
    EXPECT_FALSE(button.isDown());
}

TEST_F(ButtonMonitorTest, ReleaseDoesNotFirePress) {
    // Press, settle, then release: release must NOT produce a press edge.
    button.update(0, HIGH);
    button.update(100, LOW);
    button.update(100 + DEBOUNCE_MS + 1, LOW);
    EXPECT_TRUE(button.pressed());

    button.update(200, LOW); // clears edge
    button.update(300, HIGH); // release begins
    button.update(300 + DEBOUNCE_MS + 1, HIGH);
    EXPECT_FALSE(button.pressed()); // release is not a press
    EXPECT_FALSE(button.isDown());
}

TEST_F(ButtonMonitorTest, RapidBounce_ThenStablePress) {
    button.update(0, HIGH);
    button.update(100, LOW);
    button.update(110, HIGH);
    button.update(120, LOW);
    button.update(130, HIGH);
    button.update(140, LOW);
    EXPECT_FALSE(button.pressed());

    for (unsigned long t = 141; t <= 140 + DEBOUNCE_MS; t++) {
        button.update(t, LOW);
    }
    button.update(140 + DEBOUNCE_MS + 1, LOW);
    EXPECT_TRUE(button.pressed());
}

class ButtonDebounceThresholdTest : public ::testing::TestWithParam<unsigned long> {};

TEST_P(ButtonDebounceThresholdTest, ExactDebounceThreshold) {
    unsigned long debounceMs = GetParam();
    ButtonMonitor button(BUTTON_PIN, debounceMs);

    button.update(0, HIGH);
    button.update(100, LOW);
    button.update(100 + debounceMs, LOW);
    EXPECT_FALSE(button.pressed()) << "Should not fire at exactly " << debounceMs;
    button.update(100 + debounceMs + 1, LOW);
    EXPECT_TRUE(button.pressed()) << "Should fire at " << (debounceMs + 1);
}

INSTANTIATE_TEST_SUITE_P(
    VariousDebounceValues,
    ButtonDebounceThresholdTest,
    ::testing::Values(10UL, 50UL, 100UL, 200UL)
);
