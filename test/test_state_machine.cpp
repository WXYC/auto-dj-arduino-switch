#include <gtest/gtest.h>
#include "state_machine.h"
#include "utils.h"

// ========== Helpers ==========

Context makeContext(State state, int radioShowID = -1, int retryCount = 0,
                    unsigned long lastPollTime = 0) {
    Context ctx;
    ctx.state = state;
    ctx.radioShowID = radioShowID;
    ctx.retryCount = retryCount;
    ctx.lastPollTime = lastPollTime;
    return ctx;
}

Inputs makeInputs() {
    Inputs in;
    in.relayStateChanged = false;
    in.autoDJActive = false;
    in.wifiConnected = true;
    in.epochTime = 1705347000UL; // valid NTP time
    in.currentMillis = 100000;
    in.startShowResult = -1;
    in.endShowResult = false;
    in.pollNewTrack = false;
    in.pollLiveDJ = false;
    in.pollIntervalMs = 20000;
    in.maxRetries = 3;
    in.retryBackoffMs = 2000;
    return in;
}

// ========== BOOTING ==========

TEST(StateMachine, BootingStaysInBooting) {
    Context ctx = makeContext(BOOTING);
    Inputs in = makeInputs();

    TickResult r = tick(ctx, in);

    EXPECT_EQ(r.context.state, BOOTING);
    EXPECT_FALSE(r.addEntry);
    EXPECT_EQ(r.delayMs, 0UL);
}

// ========== CONNECTING_WIFI ==========

TEST(StateMachine, ConnectingWifiToIdleWhenConnectedNoPriorShow) {
    Context ctx = makeContext(CONNECTING_WIFI);
    Inputs in = makeInputs();
    in.wifiConnected = true;

    TickResult r = tick(ctx, in);

    EXPECT_EQ(r.context.state, IDLE);
    EXPECT_EQ(r.context.retryCount, 0);
}

TEST(StateMachine, ConnectingWifiToAutoDJActiveWhenConnectedWithPriorShow) {
    Context ctx = makeContext(CONNECTING_WIFI, /*radioShowID=*/42);
    Inputs in = makeInputs();
    in.wifiConnected = true;

    TickResult r = tick(ctx, in);

    EXPECT_EQ(r.context.state, AUTO_DJ_ACTIVE);
    EXPECT_EQ(r.context.radioShowID, 42);
    EXPECT_EQ(r.context.retryCount, 0);
}

TEST(StateMachine, ConnectingWifiStaysWhenNotConnected) {
    Context ctx = makeContext(CONNECTING_WIFI);
    Inputs in = makeInputs();
    in.wifiConnected = false;

    TickResult r = tick(ctx, in);

    EXPECT_EQ(r.context.state, CONNECTING_WIFI);
}

// ========== IDLE ==========

TEST(StateMachine, IdleToStartingShowOnRelayActivation) {
    Context ctx = makeContext(IDLE);
    Inputs in = makeInputs();
    in.relayStateChanged = true;
    in.autoDJActive = true;

    TickResult r = tick(ctx, in);

    EXPECT_EQ(r.context.state, STARTING_SHOW);
    EXPECT_EQ(r.context.retryCount, 0);
}

TEST(StateMachine, IdleStaysOnNoChange) {
    Context ctx = makeContext(IDLE);
    Inputs in = makeInputs();

    TickResult r = tick(ctx, in);

    EXPECT_EQ(r.context.state, IDLE);
}

TEST(StateMachine, IdleStaysOnRelayChangedButInactive) {
    Context ctx = makeContext(IDLE);
    Inputs in = makeInputs();
    in.relayStateChanged = true;
    in.autoDJActive = false;

    TickResult r = tick(ctx, in);

    EXPECT_EQ(r.context.state, IDLE);
}

// ========== STARTING_SHOW ==========

TEST(StateMachine, StartingShowSuccessSavesShowIDAndTransitions) {
    Context ctx = makeContext(STARTING_SHOW);
    Inputs in = makeInputs();
    in.startShowResult = 42;

    TickResult r = tick(ctx, in);

    EXPECT_EQ(r.context.state, AUTO_DJ_ACTIVE);
    EXPECT_EQ(r.context.radioShowID, 42);
    EXPECT_EQ(r.context.retryCount, 0);
    EXPECT_EQ(r.context.lastPollTime, 0UL);
}

TEST(StateMachine, StartingShowErrorOnNoNTP) {
    Context ctx = makeContext(STARTING_SHOW);
    Inputs in = makeInputs();
    in.epochTime = 0;

    TickResult r = tick(ctx, in);

    EXPECT_EQ(r.context.state, ERROR_STATE);
}

TEST(StateMachine, StartingShowRetryOnFailure) {
    Context ctx = makeContext(STARTING_SHOW);
    Inputs in = makeInputs();
    in.startShowResult = -1;

    TickResult r = tick(ctx, in);

    EXPECT_EQ(r.context.state, STARTING_SHOW);
    EXPECT_EQ(r.context.retryCount, 1);
    EXPECT_EQ(r.delayMs, 2000UL); // retryBackoffMs * 1
}

TEST(StateMachine, StartingShowRetryBackoffScales) {
    Context ctx = makeContext(STARTING_SHOW, /*radioShowID=*/-1, /*retryCount=*/1);
    Inputs in = makeInputs();
    in.startShowResult = -1;

    TickResult r = tick(ctx, in);

    EXPECT_EQ(r.context.state, STARTING_SHOW);
    EXPECT_EQ(r.context.retryCount, 2);
    EXPECT_EQ(r.delayMs, 4000UL); // retryBackoffMs * 2
}

TEST(StateMachine, StartingShowErrorOnMaxRetries) {
    Context ctx = makeContext(STARTING_SHOW, /*radioShowID=*/-1, /*retryCount=*/2);
    Inputs in = makeInputs();
    in.maxRetries = 3;
    in.startShowResult = -1;

    TickResult r = tick(ctx, in);

    EXPECT_EQ(r.context.state, ERROR_STATE);
    EXPECT_EQ(r.context.retryCount, 0);
}

// ========== AUTO_DJ_ACTIVE ==========

TEST(StateMachine, AutoDJActiveToEndingShowOnRelayDeactivation) {
    Context ctx = makeContext(AUTO_DJ_ACTIVE, /*radioShowID=*/42);
    Inputs in = makeInputs();
    in.relayStateChanged = true;
    in.autoDJActive = false;

    TickResult r = tick(ctx, in);

    EXPECT_EQ(r.context.state, ENDING_SHOW);
    EXPECT_EQ(r.context.retryCount, 0);
}

TEST(StateMachine, AutoDJActiveAddsEntryOnNewTrackNotLiveDJ) {
    Context ctx = makeContext(AUTO_DJ_ACTIVE, /*radioShowID=*/42, /*retryCount=*/0,
                              /*lastPollTime=*/50000);
    Inputs in = makeInputs();
    in.currentMillis = 100000;
    in.pollIntervalMs = 20000;
    in.pollNewTrack = true;
    in.pollLiveDJ = false;
    in.artist = "Broadcast";
    in.title = "Echo's Answer";
    in.album = "Tender Buttons";

    TickResult r = tick(ctx, in);

    EXPECT_EQ(r.context.state, AUTO_DJ_ACTIVE);
    EXPECT_TRUE(r.addEntry);
    EXPECT_EQ(r.addEntryArtist, "Broadcast");
    EXPECT_EQ(r.addEntryTitle, "Echo's Answer");
    EXPECT_EQ(r.addEntryAlbum, "Tender Buttons");
    EXPECT_EQ(r.addEntryHourMs, currentHourMs(in.epochTime));
}

TEST(StateMachine, AutoDJActiveNoEntryWhenLiveDJ) {
    Context ctx = makeContext(AUTO_DJ_ACTIVE, /*radioShowID=*/42, /*retryCount=*/0,
                              /*lastPollTime=*/50000);
    Inputs in = makeInputs();
    in.currentMillis = 100000;
    in.pollIntervalMs = 20000;
    in.pollNewTrack = true;
    in.pollLiveDJ = true;

    TickResult r = tick(ctx, in);

    EXPECT_EQ(r.context.state, AUTO_DJ_ACTIVE);
    EXPECT_FALSE(r.addEntry);
}

TEST(StateMachine, AutoDJActiveNoActionWhenPollIntervalNotElapsed) {
    Context ctx = makeContext(AUTO_DJ_ACTIVE, /*radioShowID=*/42, /*retryCount=*/0,
                              /*lastPollTime=*/95000);
    Inputs in = makeInputs();
    in.currentMillis = 100000;
    in.pollIntervalMs = 20000;
    in.pollNewTrack = true; // would trigger entry if interval had elapsed

    TickResult r = tick(ctx, in);

    EXPECT_EQ(r.context.state, AUTO_DJ_ACTIVE);
    EXPECT_FALSE(r.addEntry);
    EXPECT_EQ(r.context.lastPollTime, 95000UL); // unchanged
}

TEST(StateMachine, AutoDJActiveUpdatesLastPollTime) {
    Context ctx = makeContext(AUTO_DJ_ACTIVE, /*radioShowID=*/42, /*retryCount=*/0,
                              /*lastPollTime=*/50000);
    Inputs in = makeInputs();
    in.currentMillis = 100000;
    in.pollIntervalMs = 20000;
    in.pollNewTrack = false;

    TickResult r = tick(ctx, in);

    EXPECT_EQ(r.context.lastPollTime, 100000UL);
}

TEST(StateMachine, AutoDJActiveNoEntryWhenNoNewTrack) {
    Context ctx = makeContext(AUTO_DJ_ACTIVE, /*radioShowID=*/42, /*retryCount=*/0,
                              /*lastPollTime=*/50000);
    Inputs in = makeInputs();
    in.currentMillis = 100000;
    in.pollIntervalMs = 20000;
    in.pollNewTrack = false;

    TickResult r = tick(ctx, in);

    EXPECT_FALSE(r.addEntry);
}

// ========== ENDING_SHOW ==========

TEST(StateMachine, EndingShowSuccessClearsShowAndGoesIdle) {
    Context ctx = makeContext(ENDING_SHOW, /*radioShowID=*/42);
    Inputs in = makeInputs();
    in.endShowResult = true;

    TickResult r = tick(ctx, in);

    EXPECT_EQ(r.context.state, IDLE);
    EXPECT_EQ(r.context.radioShowID, -1);
    EXPECT_EQ(r.context.retryCount, 0);
}

TEST(StateMachine, EndingShowRetryOnFailure) {
    Context ctx = makeContext(ENDING_SHOW, /*radioShowID=*/42);
    Inputs in = makeInputs();
    in.endShowResult = false;

    TickResult r = tick(ctx, in);

    EXPECT_EQ(r.context.state, ENDING_SHOW);
    EXPECT_EQ(r.context.retryCount, 1);
    EXPECT_EQ(r.delayMs, 2000UL);
}

TEST(StateMachine, EndingShowForcedIdleOnMaxRetries) {
    Context ctx = makeContext(ENDING_SHOW, /*radioShowID=*/42, /*retryCount=*/2);
    Inputs in = makeInputs();
    in.maxRetries = 3;
    in.endShowResult = false;

    TickResult r = tick(ctx, in);

    EXPECT_EQ(r.context.state, IDLE);
    EXPECT_EQ(r.context.radioShowID, -1);
    EXPECT_EQ(r.context.retryCount, 0);
}

// ========== ERROR_STATE ==========

TEST(StateMachine, ErrorStateToConnectingWifiOnWifiLost) {
    Context ctx = makeContext(ERROR_STATE);
    Inputs in = makeInputs();
    in.wifiConnected = false;

    TickResult r = tick(ctx, in);

    EXPECT_EQ(r.context.state, CONNECTING_WIFI);
    EXPECT_EQ(r.context.retryCount, 0);
}

TEST(StateMachine, ErrorStateToStartingShowWhenAutoDJActiveNoShow) {
    Context ctx = makeContext(ERROR_STATE);
    Inputs in = makeInputs();
    in.autoDJActive = true;

    TickResult r = tick(ctx, in);

    EXPECT_EQ(r.context.state, STARTING_SHOW);
    EXPECT_EQ(r.context.retryCount, 0);
}

TEST(StateMachine, ErrorStateToIdleWhenAutoDJInactive) {
    Context ctx = makeContext(ERROR_STATE);
    Inputs in = makeInputs();
    in.autoDJActive = false;

    TickResult r = tick(ctx, in);

    EXPECT_EQ(r.context.state, IDLE);
    EXPECT_EQ(r.context.retryCount, 0);
}

TEST(StateMachine, ErrorStateAlwaysReturnsDelay) {
    Context ctx = makeContext(ERROR_STATE);
    Inputs in = makeInputs();
    in.autoDJActive = false; // will transition to IDLE

    TickResult r = tick(ctx, in);

    EXPECT_EQ(r.context.state, IDLE); // transition occurred
    EXPECT_EQ(r.delayMs, in.retryBackoffMs); // delay still fires
}

// ========== WiFi Loss (parameterized) ==========

class WifiLossTest : public ::testing::TestWithParam<State> {};

TEST_P(WifiLossTest, TransitionsToConnectingWifiAndPreservesShowID) {
    State state = GetParam();
    Context ctx = makeContext(state, /*radioShowID=*/42, /*retryCount=*/2);
    Inputs in = makeInputs();
    in.wifiConnected = false;

    TickResult r = tick(ctx, in);

    EXPECT_EQ(r.context.state, CONNECTING_WIFI);
    EXPECT_EQ(r.context.radioShowID, 42);
}

INSTANTIATE_TEST_SUITE_P(
    AllActiveStates,
    WifiLossTest,
    ::testing::Values(IDLE, STARTING_SHOW, AUTO_DJ_ACTIVE, ENDING_SHOW, ERROR_STATE)
);

TEST(StateMachine, BootingDoesNotTransitionOnWifiLoss) {
    Context ctx = makeContext(BOOTING);
    Inputs in = makeInputs();
    in.wifiConnected = false;

    TickResult r = tick(ctx, in);

    EXPECT_EQ(r.context.state, BOOTING);
}

// ========== stateName ==========

TEST(StateName, ReturnsHumanReadableNames) {
    EXPECT_STREQ(stateName(BOOTING), "BOOTING");
    EXPECT_STREQ(stateName(CONNECTING_WIFI), "CONNECTING_WIFI");
    EXPECT_STREQ(stateName(IDLE), "IDLE");
    EXPECT_STREQ(stateName(STARTING_SHOW), "STARTING_SHOW");
    EXPECT_STREQ(stateName(AUTO_DJ_ACTIVE), "AUTO_DJ_ACTIVE");
    EXPECT_STREQ(stateName(ENDING_SHOW), "ENDING_SHOW");
    EXPECT_STREQ(stateName(ERROR_STATE), "ERROR");
}
