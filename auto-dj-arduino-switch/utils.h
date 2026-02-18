#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

/**
 * URL-encodes a string for use in HTTP form bodies.
 * Unreserved characters (alphanumeric, '-', '_', '.', '~') pass through.
 * Spaces become '+'. All other bytes are percent-encoded.
 */
String urlEncode(const String& str);

/**
 * Parses the radioShowID from a Location header value.
 * Looks for "radioShowID=<digits>" in the string.
 * Returns the ID, or -1 if not found or not a valid positive integer.
 */
int parseRadioShowID(const String& location);

/**
 * Truncates an epoch-seconds value to the hour boundary and converts to milliseconds.
 * Returns 0 if epochSeconds is 0 (no NTP time available).
 */
unsigned long currentHourMs(unsigned long epochSeconds);

#endif
