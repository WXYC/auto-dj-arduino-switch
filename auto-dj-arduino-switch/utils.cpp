#include "utils.h"

String urlEncode(const String& str) {
    String encoded;
    encoded.reserve(str.length() * 2);
    for (unsigned int i = 0; i < str.length(); i++) {
        char c = str.charAt(i);
        if (isAlphaNumeric(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded += c;
        } else if (c == ' ') {
            encoded += '+';
        } else {
            encoded += '%';
            if ((unsigned char)c < 0x10) encoded += '0';
            encoded += String((unsigned char)c, HEX);
        }
    }
    return encoded;
}

int parseRadioShowID(const String& location) {
    int idx = location.indexOf("radioShowID=");
    if (idx < 0) {
        return -1;
    }

    String idStr = location.substring(idx + 12); // length of "radioShowID="
    int ampIdx = idStr.indexOf('&');
    if (ampIdx >= 0) {
        idStr = idStr.substring(0, ampIdx);
    }

    int radioShowID = idStr.toInt();
    if (radioShowID <= 0) {
        return -1;
    }

    return radioShowID;
}

unsigned long currentHourMs(unsigned long epochSeconds) {
    if (epochSeconds == 0) return 0;
    unsigned long hourEpoch = epochSeconds - (epochSeconds % 3600);
    return hourEpoch * 1000UL;
}
