#include <gtest/gtest.h>
#include "state_machine.h"
#include "test_helpers.h"

// Reporter state machine: connectivity + LED, no show lifecycle.

static Inputs defaultInputs() {
    Inputs in;
    in.relayChanged = false;
    in.relayAutoDJActive = true;
    in.buttonPressed = false;
    in.ethernetLinkUp = false;
    in.wifiConnected = false;
    in.channelUp = false;
    in.gotCommand = false;
    in.commandAction = CMD_NONE;
    in.gotActiveResult = false;
    in.activeResult = false;
    in.currentMillis = 1000;
    in.heartbeatIntervalMs = 30000;
    in.retryBackoffMs = 2000;
    in.maxRetries = 3;
    return in;
}

static Context connectedCtx() {
    Context c;
    c.state = CONNECTED;
    c.relayAutoDJActive = true;
    c.orchestratorActive = false;
    c.transport = TRANSPORT_ETHERNET;
    c.channelUp = true;
    c.retryCount = 0;
    c.lastHeartbeatMs = 1000;
    return c;
}

// Inputs that keep a CONNECTED ethernet context connected.
static Inputs connectedInputs() {
    Inputs in = defaultInputs();
    in.ethernetLinkUp = true;
    in.channelUp = true;
    return in;
}

TEST(StateMachine, BootingToConnecting) {
    Context ctx;
    ctx.state = BOOTING;
    ctx.retryCount = 5;
    TickResult r = tick(ctx, defaultInputs());
    EXPECT_EQ(r.context.state, CONNECTING);
    EXPECT_EQ(r.context.retryCount, 0);
}

TEST(StateMachine, ConnectingToConnectedEthernet) {
    Context ctx;
    ctx.state = CONNECTING;
    ctx.retryCount = 1;
    Inputs in = defaultInputs();
    in.ethernetLinkUp = true;
    in.channelUp = true;
    TickResult r = tick(ctx, in);
    EXPECT_EQ(r.context.state, CONNECTED);
    EXPECT_EQ(r.context.transport, TRANSPORT_ETHERNET);
    EXPECT_EQ(r.context.retryCount, 0);
}

TEST(StateMachine, ConnectingToConnectedWiFi) {
    Context ctx;
    ctx.state = CONNECTING;
    Inputs in = defaultInputs();
    in.ethernetLinkUp = false;
    in.wifiConnected = true;
    TickResult r = tick(ctx, in);
    EXPECT_EQ(r.context.state, CONNECTED);
    EXPECT_EQ(r.context.transport, TRANSPORT_WIFI);
}

TEST(StateMachine, ConnectingRetriesThenErrors) {
    Context ctx;
    ctx.state = CONNECTING;
    ctx.retryCount = 0;
    Inputs in = defaultInputs(); // no link

    TickResult r1 = tick(ctx, in);
    EXPECT_EQ(r1.context.state, CONNECTING);
    EXPECT_EQ(r1.context.retryCount, 1);
    EXPECT_EQ(r1.delayMs, 2000UL);

    TickResult r2 = tick(r1.context, in);
    EXPECT_EQ(r2.context.retryCount, 2);
    EXPECT_EQ(r2.delayMs, 4000UL);

    TickResult r3 = tick(r2.context, in);
    EXPECT_EQ(r3.context.state, ERROR_STATE);
    EXPECT_EQ(r3.context.retryCount, 0);
}

TEST(StateMachine, ConnectedToConnectingOnChannelDrop) {
    Context ctx = connectedCtx();
    Inputs in = connectedInputs();
    in.channelUp = false; // WS dropped
    TickResult r = tick(ctx, in);
    EXPECT_EQ(r.context.state, CONNECTING);
    EXPECT_EQ(r.context.transport, TRANSPORT_NONE);
}

TEST(StateMachine, RelayChangeTriggersHeartbeatAndUpdatesLevel) {
    Context ctx = connectedCtx();
    Inputs in = connectedInputs();
    in.relayChanged = true;
    in.relayAutoDJActive = false;
    TickResult r = tick(ctx, in);
    EXPECT_TRUE(r.sendHeartbeat);
    EXPECT_FALSE(r.context.relayAutoDJActive);
}

TEST(StateMachine, ButtonPressTriggersSendButtonToggle) {
    Context ctx = connectedCtx();
    Inputs in = connectedInputs();
    in.buttonPressed = true;
    TickResult r = tick(ctx, in);
    EXPECT_TRUE(r.sendButtonToggle);
}

TEST(StateMachine, CommandPauseResumeDriveLedAndAck) {
    Context ctx = connectedCtx();
    Inputs pause = connectedInputs();
    pause.gotCommand = true;
    pause.commandAction = CMD_PAUSE;
    TickResult rp = tick(ctx, pause);
    EXPECT_TRUE(rp.sendAck);
    EXPECT_EQ(rp.ackStatus, ACK_OK);
    EXPECT_FALSE(rp.context.orchestratorActive);
    EXPECT_EQ(rp.setLed, 0);

    Inputs resume = connectedInputs();
    resume.gotCommand = true;
    resume.commandAction = CMD_RESUME;
    TickResult rr = tick(rp.context, resume);
    EXPECT_TRUE(rr.context.orchestratorActive);
    EXPECT_EQ(rr.setLed, 1);
}

TEST(StateMachine, CommandPingTriggersHeartbeatAndAck) {
    Context ctx = connectedCtx();
    Inputs in = connectedInputs();
    in.gotCommand = true;
    in.commandAction = CMD_PING;
    TickResult r = tick(ctx, in);
    EXPECT_TRUE(r.sendHeartbeat);
    EXPECT_TRUE(r.sendAck);
    EXPECT_EQ(r.ackStatus, ACK_OK);
}

TEST(StateMachine, CommandRestartSetsDoRestart) {
    Context ctx = connectedCtx();
    Inputs in = connectedInputs();
    in.gotCommand = true;
    in.commandAction = CMD_RESTART;
    TickResult r = tick(ctx, in);
    EXPECT_TRUE(r.doRestart);
    EXPECT_TRUE(r.sendAck);
}

TEST(StateMachine, CommandUnknownAcksUnknownCommand) {
    Context ctx = connectedCtx();
    Inputs in = connectedInputs();
    in.gotCommand = true;
    in.commandAction = CMD_UNKNOWN;
    TickResult r = tick(ctx, in);
    EXPECT_TRUE(r.sendAck);
    EXPECT_EQ(r.ackStatus, ACK_UNKNOWN_COMMAND);
}

TEST(StateMachine, AckResultActiveDrivesLed) {
    Context ctx = connectedCtx();
    Inputs in = connectedInputs();
    in.gotActiveResult = true;
    in.activeResult = true;
    TickResult r = tick(ctx, in);
    EXPECT_TRUE(r.context.orchestratorActive);
    EXPECT_EQ(r.setLed, 1);
}

TEST(StateMachine, HeartbeatIntervalElapsedTriggersHeartbeat) {
    Context ctx = connectedCtx(); // lastHeartbeatMs = 1000
    Inputs in = connectedInputs();
    in.currentMillis = 1000 + 30000;
    TickResult r = tick(ctx, in);
    EXPECT_TRUE(r.sendHeartbeat);
    EXPECT_EQ(r.context.lastHeartbeatMs, 31000UL);
}

TEST(StateMachine, LedFollowsRelayWhenChannelDown) {
    Context ctx;
    ctx.state = CONNECTING;
    ctx.relayAutoDJActive = true;
    ctx.orchestratorActive = false;
    ctx.transport = TRANSPORT_NONE;
    ctx.channelUp = false;
    ctx.retryCount = 0;
    ctx.lastHeartbeatMs = 0;
    Inputs in = defaultInputs();
    in.channelUp = false;
    in.relayAutoDJActive = true;
    TickResult r = tick(ctx, in);
    EXPECT_EQ(r.setLed, 1); // channel down -> LED mirrors relay
}

TEST(StateMachine, ErrorStateRecoversWhenLinkReturns) {
    Context ctx;
    ctx.state = ERROR_STATE;
    ctx.retryCount = 0;
    Inputs in = defaultInputs();
    in.ethernetLinkUp = true;
    TickResult r = tick(ctx, in);
    EXPECT_EQ(r.context.state, CONNECTING);
    EXPECT_EQ(r.context.retryCount, 0);
}

TEST(StateMachine, StateNameMatchesEnum) {
    EXPECT_STREQ(stateName(BOOTING), "BOOTING");
    EXPECT_STREQ(stateName(CONNECTING), "CONNECTING");
    EXPECT_STREQ(stateName(CONNECTED), "CONNECTED");
    EXPECT_STREQ(stateName(ERROR_STATE), "ERROR_STATE");
}
