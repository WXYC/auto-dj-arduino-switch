#ifndef FLOWSHEET_CLIENT_H
#define FLOWSHEET_CLIENT_H

#include <Arduino.h>

/**
 * Manages HTTP POST calls to the tubafrenzy flowsheet API.
 *
 * All requests authenticate via the X-Auto-DJ-Key header, which is checked
 * by XYCCatalogServlet.validateControlRoomAccess() on the server side.
 *
 * The servlets respond with HTTP 302 redirects on success. ArduinoHttpClient
 * does not follow redirects by default, so we read the Location header
 * directly (needed for extracting radioShowID from startRadioShow).
 */
class FlowsheetClient {
public:
    FlowsheetClient(const char* host, int port, const char* apiKey);

    /**
     * Starts a new radio show. Returns the radioShowID on success, or -1 on failure.
     * Parses the radioShowID from the Location header of the 302 redirect.
     */
    int startShow(unsigned long startingHourMs);

    /**
     * Adds a flowsheet entry with autoBreakpoint=true (server handles hourly
     * breakpoints automatically via FlowsheetEntryService.createEntryWithAutoBreakpoints()).
     */
    bool addEntry(int radioShowID, unsigned long workingHourMs,
                  const String& artist, const String& title, const String& album);

    /**
     * Ends the radio show. Uses mode=signoffConfirm to skip the interactive JSP
     * confirmation page.
     */
    bool endShow(int radioShowID);

private:
    const char* host;
    int port;
    const char* apiKey;

    int postForm(const char* path, const String& body);
    String getLocationHeader(const char* path, const String& body);
};

#endif
