#include "state_machine.h"

// LED is on when auto-DJ is active. When the management channel is up the
// orchestrator owns that truth; when it is down we fall back to the relay so the
// studio LED is still meaningful offline.
static int ledFor(const Context& ctx) {
    if (ctx.channelUp) return ctx.orchestratorActive ? 1 : 0;
    return ctx.relayAutoDJActive ? 1 : 0;
}

TickResult tick(const Context& ctx, const Inputs& in) {
    TickResult r;
    r.context = ctx;
    r.sendHeartbeat = false;
    r.sendButtonToggle = false;
    r.sendAck = false;
    r.ackStatus = ACK_OK;
    r.doRestart = false;
    r.setLed = -1;
    r.delayMs = 0;

    // Always fold in the latest relay level and channel state.
    r.context.relayAutoDJActive = in.relayAutoDJActive;
    r.context.channelUp = in.channelUp;

    switch (ctx.state) {
        case BOOTING:
            r.context.state = CONNECTING;
            r.context.retryCount = 0;
            break;

        case CONNECTING:
            if (in.ethernetLinkUp && in.channelUp) {
                r.context.state = CONNECTED;
                r.context.transport = TRANSPORT_ETHERNET;
                r.context.retryCount = 0;
            } else if (!in.ethernetLinkUp && in.wifiConnected) {
                r.context.state = CONNECTED;
                r.context.transport = TRANSPORT_WIFI;
                r.context.retryCount = 0;
            } else {
                int rc = ctx.retryCount + 1;
                if (rc >= in.maxRetries) {
                    r.context.state = ERROR_STATE;
                    r.context.retryCount = 0;
                } else {
                    r.context.retryCount = rc;
                    r.delayMs = in.retryBackoffMs * (unsigned long)rc;
                }
            }
            break;

        case CONNECTED: {
            bool stillConnected =
                (ctx.transport == TRANSPORT_ETHERNET && in.ethernetLinkUp && in.channelUp) ||
                (ctx.transport == TRANSPORT_WIFI && in.wifiConnected);
            if (!stillConnected) {
                r.context.state = CONNECTING;
                r.context.transport = TRANSPORT_NONE;
                break;
            }

            // Report a relay change promptly.
            if (in.relayChanged) {
                r.sendHeartbeat = true;
            }

            // A debounced button press toggles activation (orchestrator decides).
            if (in.buttonPressed) {
                r.sendButtonToggle = true;
            }

            // Inbound commands.
            if (in.gotCommand) {
                r.sendAck = true;
                switch (in.commandAction) {
                    case CMD_PAUSE:
                        r.context.orchestratorActive = false;
                        break;
                    case CMD_RESUME:
                        r.context.orchestratorActive = true;
                        break;
                    case CMD_PING:
                        r.sendHeartbeat = true;
                        break;
                    case CMD_RESTART:
                        r.doRestart = true;
                        break;
                    case CMD_SET_CONFIG:
                    case CMD_END_SHOW:
                        // Acked; no reporter-side effect.
                        break;
                    default:
                        r.ackStatus = ACK_UNKNOWN_COMMAND;
                        break;
                }
            }

            // The orchestrator's reported active flag drives the LED.
            if (in.gotActiveResult) {
                r.context.orchestratorActive = in.activeResult;
            }

            // Periodic heartbeat.
            if (in.currentMillis - ctx.lastHeartbeatMs >= in.heartbeatIntervalMs) {
                r.sendHeartbeat = true;
            }
            if (r.sendHeartbeat) {
                r.context.lastHeartbeatMs = in.currentMillis;
            }
            break;
        }

        case ERROR_STATE:
            if (in.ethernetLinkUp || in.wifiConnected) {
                r.context.state = CONNECTING;
                r.context.retryCount = 0;
            } else {
                r.delayMs = in.retryBackoffMs;
            }
            break;
    }

    r.setLed = ledFor(r.context);
    return r;
}

const char* stateName(State s) {
    switch (s) {
        case BOOTING: return "BOOTING";
        case CONNECTING: return "CONNECTING";
        case CONNECTED: return "CONNECTED";
        case ERROR_STATE: return "ERROR_STATE";
        default: return "UNKNOWN";
    }
}
