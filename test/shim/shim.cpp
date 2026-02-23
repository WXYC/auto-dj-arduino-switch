/**
 * Desktop shim implementations.
 *
 * Provides global Serial instance, controllable millis() for tests,
 * and Stream methods that depend on millis().
 */
#include "Arduino.h"

// ========== Serial ==========

NullSerial Serial;

// ========== Timing ==========

static unsigned long g_millis = 0;

unsigned long millis() {
    return g_millis;
}

// Test helpers (declared in test_helpers.h)
void setMillis(unsigned long ms) {
    g_millis = ms;
}

void advanceMillis(unsigned long ms) {
    g_millis += ms;
}

// ========== Stream ==========

int Stream::timedRead() {
    unsigned long start = millis();
    do {
        int c = read();
        if (c >= 0) return c;
    } while (millis() - start < _timeout);
    return -1;
}

String Stream::readString() {
    String ret;
    int c = timedRead();
    while (c >= 0) {
        ret += static_cast<char>(c);
        c = timedRead();
    }
    return ret;
}
