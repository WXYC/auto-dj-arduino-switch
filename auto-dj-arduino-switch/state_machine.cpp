#include "state_machine.h"
#include "utils.h"

TickResult tick(const Context& ctx, const Inputs& inputs) {
    TickResult result;
    result.context = ctx;
    result.addEntry = false;
    result.addEntryHourMs = 0;
    result.delayMs = 0;

    // WiFi loss: any state except BOOTING/CONNECTING_WIFI -> CONNECTING_WIFI.
    // Preserves radioShowID for resumption after reconnect.
    if (result.context.state != BOOTING &&
        result.context.state != CONNECTING_WIFI &&
        !inputs.wifiConnected) {
        result.context.state = CONNECTING_WIFI;
        return result;
    }

    switch (result.context.state) {
        case BOOTING:
            break;

        case CONNECTING_WIFI:
            if (inputs.wifiConnected) {
                if (result.context.radioShowID > 0) {
                    result.context.state = AUTO_DJ_ACTIVE;
                } else {
                    result.context.state = IDLE;
                }
                result.context.retryCount = 0;
            }
            break;

        case IDLE:
            if (inputs.relayStateChanged && inputs.autoDJActive) {
                result.context.state = STARTING_SHOW;
                result.context.retryCount = 0;
            }
            break;

        case STARTING_SHOW:
            if (inputs.epochTime == 0) {
                result.context.state = ERROR_STATE;
                result.context.retryCount = 0;
                break;
            }
            if (inputs.startShowResult > 0) {
                result.context.radioShowID = inputs.startShowResult;
                result.context.lastPollTime = 0;
                result.context.state = AUTO_DJ_ACTIVE;
                result.context.retryCount = 0;
            } else {
                result.context.retryCount++;
                if (result.context.retryCount >= inputs.maxRetries) {
                    result.context.state = ERROR_STATE;
                    result.context.retryCount = 0;
                } else {
                    result.delayMs = inputs.retryBackoffMs * result.context.retryCount;
                }
            }
            break;

        case AUTO_DJ_ACTIVE:
            if (inputs.relayStateChanged && !inputs.autoDJActive) {
                result.context.state = ENDING_SHOW;
                result.context.retryCount = 0;
                break;
            }
            if (inputs.currentMillis - result.context.lastPollTime >= inputs.pollIntervalMs) {
                result.context.lastPollTime = inputs.currentMillis;
                if (inputs.pollNewTrack && !inputs.pollLiveDJ) {
                    unsigned long hourMs = currentHourMs(inputs.epochTime);
                    if (hourMs > 0) {
                        result.addEntry = true;
                        result.addEntryHourMs = hourMs;
                        result.addEntryArtist = inputs.artist;
                        result.addEntryTitle = inputs.title;
                        result.addEntryAlbum = inputs.album;
                    }
                }
            }
            break;

        case ENDING_SHOW:
            if (inputs.endShowResult) {
                result.context.radioShowID = -1;
                result.context.state = IDLE;
                result.context.retryCount = 0;
            } else {
                result.context.retryCount++;
                if (result.context.retryCount >= inputs.maxRetries) {
                    result.context.radioShowID = -1;
                    result.context.state = IDLE;
                    result.context.retryCount = 0;
                } else {
                    result.delayMs = inputs.retryBackoffMs * result.context.retryCount;
                }
            }
            break;

        case ERROR_STATE:
            if (!inputs.wifiConnected) {
                result.context.state = CONNECTING_WIFI;
                result.context.retryCount = 0;
            } else if (inputs.autoDJActive && result.context.radioShowID <= 0) {
                result.context.state = STARTING_SHOW;
                result.context.retryCount = 0;
            } else if (!inputs.autoDJActive) {
                result.context.state = IDLE;
                result.context.retryCount = 0;
            }
            result.delayMs = inputs.retryBackoffMs;
            break;
    }

    return result;
}

const char* stateName(State s) {
    switch (s) {
        case BOOTING:         return "BOOTING";
        case CONNECTING_WIFI: return "CONNECTING_WIFI";
        case IDLE:            return "IDLE";
        case STARTING_SHOW:   return "STARTING_SHOW";
        case AUTO_DJ_ACTIVE:  return "AUTO_DJ_ACTIVE";
        case ENDING_SHOW:     return "ENDING_SHOW";
        case ERROR_STATE:     return "ERROR";
        default:              return "UNKNOWN";
    }
}
