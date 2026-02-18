#ifndef AZURACAST_CLIENT_H
#define AZURACAST_CLIENT_H

#include <Arduino.h>

/**
 * Polls the AzuraCast now-playing API and detects track changes.
 *
 * Uses the static JSON endpoint (/api/nowplaying_static/main.json) which is
 * Nginx-cached to minimize server load. Parses only the needed fields via
 * an ArduinoJson filter document to keep memory usage low (~1KB parsed output
 * from a ~10KB response).
 *
 * Track changes are detected by comparing now_playing.sh_id (a monotonically
 * increasing song history ID that is unique per play event).
 */
class AzuraCastClient {
public:
    AzuraCastClient(const char* host, int port, const char* path);

    /**
     * Polls the AzuraCast API. Returns true if a new track is detected.
     * Creates and destroys the SSL client within the call to avoid
     * the Giga R1 global WiFiClient crash bug.
     */
    bool poll();

    String getArtist() const;
    String getTitle() const;
    String getAlbum() const;
    int getShId() const;
    bool isLiveDJ() const;

private:
    const char* host;
    int port;
    const char* path;

    int lastShId;
    String artist;
    String title;
    String album;
    bool liveDJ;
};

#endif
