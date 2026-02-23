#include <gtest/gtest.h>
#include "relay_monitor.h"
#include "test_helpers.h"

// All tests use the parameterized update(millis, reading) which has zero
// hardware dependencies. The no-arg update() wrapper only adds
// digitalRead() input and digitalWrite() output.

static const int RELAY_PIN = 2;
static const int LED_PIN = 3;
static const unsigned long DEBOUNCE_MS = 50;

// ========== Helpers ==========

class RelayMonitorTest : public ::testing::Test {
protected:
    RelayMonitor monitor{RELAY_PIN, LED_PIN, DEBOUNCE_MS};

    void feedStableReading(int reading, unsigned long startMs, int count) {
        for (int i = 0; i < count; i++) {
            monitor.update(startMs + i, reading);
        }
    }
};

// ========== Tests ==========

TEST_F(RelayMonitorTest, InitialState_HighNotActive) {
    // After construction (before any update): HIGH = relay open = DJ live
    EXPECT_FALSE(monitor.isAutoDJActive());
    EXPECT_FALSE(monitor.stateChanged());
    EXPECT_EQ(monitor.getLedState(), LOW);
}

TEST_F(RelayMonitorTest, StableHigh_NoChange) {
    // Many updates with HIGH readings: no state change
    for (unsigned long t = 0; t < 200; t++) {
        monitor.update(t, HIGH);
        EXPECT_FALSE(monitor.stateChanged());
        EXPECT_FALSE(monitor.isAutoDJActive());
        EXPECT_EQ(monitor.getLedState(), LOW);
    }
}

TEST_F(RelayMonitorTest, TransitionToLow_BeyondDebounce) {
    // Feed LOW for > debounceMs: stateChanged() fires, isAutoDJActive() true
    unsigned long t = 0;
    monitor.update(t, HIGH); // establish initial state
    t = 100;
    monitor.update(t, LOW); // first LOW reading, resets timer

    // Feed LOW readings until debounce period passes
    for (t = 101; t <= 100 + DEBOUNCE_MS; t++) {
        monitor.update(t, LOW);
        EXPECT_FALSE(monitor.stateChanged()) << "Should not change at t=" << t;
    }

    // One more ms beyond debounce -> transition fires
    monitor.update(100 + DEBOUNCE_MS + 1, LOW);
    EXPECT_TRUE(monitor.stateChanged());
    EXPECT_TRUE(monitor.isAutoDJActive());
}

TEST_F(RelayMonitorTest, BounceRejected_WithinDebounce) {
    // LOW then HIGH within debounce window: state stays HIGH
    monitor.update(0, HIGH);
    monitor.update(100, LOW);  // start debounce timer
    monitor.update(120, LOW);  // 20ms < 50ms debounce
    monitor.update(130, HIGH); // bounce back to HIGH within window

    // Continue with HIGH readings past the original debounce period
    for (unsigned long t = 131; t <= 200; t++) {
        monitor.update(t, HIGH);
    }

    EXPECT_FALSE(monitor.isAutoDJActive());
    EXPECT_FALSE(monitor.stateChanged());
}

TEST_F(RelayMonitorTest, LedTracksState) {
    // After LOW transition: getLedState() HIGH. After back to HIGH: getLedState() LOW
    monitor.update(0, HIGH);
    EXPECT_EQ(monitor.getLedState(), LOW);

    // Transition to LOW (auto DJ active)
    monitor.update(100, LOW);
    monitor.update(100 + DEBOUNCE_MS + 1, LOW);
    EXPECT_EQ(monitor.getLedState(), HIGH); // LED on

    // Transition back to HIGH (DJ live)
    monitor.update(300, HIGH);
    monitor.update(300 + DEBOUNCE_MS + 1, HIGH);
    EXPECT_EQ(monitor.getLedState(), LOW); // LED off
}

TEST_F(RelayMonitorTest, ChangedClearsOnNextUpdate) {
    // After a state change, next update resets changed to false
    monitor.update(0, HIGH);
    monitor.update(100, LOW);
    monitor.update(100 + DEBOUNCE_MS + 1, LOW);
    EXPECT_TRUE(monitor.stateChanged());

    // Next update clears changed
    monitor.update(100 + DEBOUNCE_MS + 2, LOW);
    EXPECT_FALSE(monitor.stateChanged());
}

TEST_F(RelayMonitorTest, RapidBounce_ThenStable) {
    // Alternating readings then stable: only transitions after stable period
    monitor.update(0, HIGH);

    // Rapid bouncing
    monitor.update(100, LOW);
    monitor.update(110, HIGH);
    monitor.update(120, LOW);
    monitor.update(130, HIGH);
    monitor.update(140, LOW);

    // None of these should trigger a transition
    EXPECT_FALSE(monitor.isAutoDJActive());

    // Now hold LOW stable past debounce from last change at t=140
    for (unsigned long t = 141; t <= 140 + DEBOUNCE_MS; t++) {
        monitor.update(t, LOW);
    }
    monitor.update(140 + DEBOUNCE_MS + 1, LOW);
    EXPECT_TRUE(monitor.stateChanged());
    EXPECT_TRUE(monitor.isAutoDJActive());
}

TEST_F(RelayMonitorTest, TransitionBackToHigh) {
    // Stable LOW -> stable HIGH: state changes back, isAutoDJActive() false
    monitor.update(0, HIGH);

    // Transition to LOW
    monitor.update(100, LOW);
    monitor.update(100 + DEBOUNCE_MS + 1, LOW);
    EXPECT_TRUE(monitor.isAutoDJActive());

    // Clear the changed flag
    monitor.update(200, LOW);
    EXPECT_FALSE(monitor.stateChanged());

    // Transition back to HIGH
    monitor.update(300, HIGH);
    monitor.update(300 + DEBOUNCE_MS + 1, HIGH);
    EXPECT_TRUE(monitor.stateChanged());
    EXPECT_FALSE(monitor.isAutoDJActive());
}

// Parameterized test: transition fires at exactly debounceMs + 1, not at debounceMs
class DebounceThresholdTest : public ::testing::TestWithParam<unsigned long> {};

TEST_P(DebounceThresholdTest, ExactDebounceThreshold) {
    unsigned long debounceMs = GetParam();
    RelayMonitor monitor(RELAY_PIN, LED_PIN, debounceMs);

    monitor.update(0, HIGH);
    monitor.update(100, LOW); // start timer at t=100

    // At exactly debounceMs after last change: should NOT transition
    // (condition is > debounceMs, not >=)
    monitor.update(100 + debounceMs, LOW);
    EXPECT_FALSE(monitor.stateChanged())
        << "Should not change at exactly debounceMs=" << debounceMs;

    // One ms later: SHOULD transition
    monitor.update(100 + debounceMs + 1, LOW);
    EXPECT_TRUE(monitor.stateChanged())
        << "Should change at debounceMs+1=" << (debounceMs + 1);
}

INSTANTIATE_TEST_SUITE_P(
    VariousDebounceValues,
    DebounceThresholdTest,
    ::testing::Values(10UL, 50UL, 100UL, 200UL)
);
