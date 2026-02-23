/**
 * Test helpers for controlling the desktop shim.
 *
 * Include this in test files that need to manipulate millis() time.
 */
#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

void setMillis(unsigned long ms);
void advanceMillis(unsigned long ms);

#endif // TEST_HELPERS_H
