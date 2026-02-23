#include <gtest/gtest.h>
#include "azuracast_client.h"
#include "fake_client.h"
#include "test_helpers.h"
#include <string>
#include <sstream>

// All tests use poll(Client&) with a FakeClient pre-loaded with HTTP responses.

static const char* HOST = "remote.wxyc.org";
static const int PORT = 443;
static const char* PATH = "/api/nowplaying_static/main.json";

// ========== Test Helpers ==========

std::string makeAzuraCastResponse(int shId, const char* artist, const char* title,
                                   const char* album, bool isLive) {
    std::ostringstream json;
    json << "{\"now_playing\":{\"sh_id\":" << shId
         << ",\"song\":{\"artist\":\"" << artist
         << "\",\"title\":\"" << title
         << "\",\"album\":\"" << album
         << "\"}},\"live\":{\"is_live\":" << (isLive ? "true" : "false") << "}}";

    std::string body = json.str();
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: application/json\r\n"
             << "Content-Length: " << body.size() << "\r\n"
             << "Connection: close\r\n"
             << "\r\n"
             << body;
    return response.str();
}

std::string makeHttpResponse(int statusCode, const std::string& body) {
    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " Error\r\n"
             << "Content-Length: " << body.size() << "\r\n"
             << "Connection: close\r\n"
             << "\r\n"
             << body;
    return response.str();
}

// ========== Tests ==========

TEST(AzuraCastClient, NewTrack_ReturnsTrue) {
    AzuraCastClient client(HOST, PORT, PATH);
    FakeClient fake;
    fake.preloadResponse(makeAzuraCastResponse(42, "Yo La Tengo", "Autumn Sweater", "I Can Hear the Heart Beating as One", false));

    EXPECT_TRUE(client.poll(fake));
    EXPECT_EQ(client.getShId(), 42);
}

TEST(AzuraCastClient, SameTrack_ReturnsFalse) {
    AzuraCastClient client(HOST, PORT, PATH);

    FakeClient fake1;
    fake1.preloadResponse(makeAzuraCastResponse(42, "Artist", "Title", "Album", false));
    ASSERT_TRUE(client.poll(fake1));

    FakeClient fake2;
    fake2.preloadResponse(makeAzuraCastResponse(42, "Artist", "Title", "Album", false));
    EXPECT_FALSE(client.poll(fake2));
}

TEST(AzuraCastClient, DifferentTrack_ReturnsTrue) {
    AzuraCastClient client(HOST, PORT, PATH);

    FakeClient fake1;
    fake1.preloadResponse(makeAzuraCastResponse(42, "Artist A", "Title A", "Album A", false));
    ASSERT_TRUE(client.poll(fake1));

    FakeClient fake2;
    fake2.preloadResponse(makeAzuraCastResponse(43, "Artist B", "Title B", "Album B", false));
    EXPECT_TRUE(client.poll(fake2));
}

TEST(AzuraCastClient, ParsesTrackFields) {
    AzuraCastClient client(HOST, PORT, PATH);
    FakeClient fake;
    fake.preloadResponse(makeAzuraCastResponse(99, "Broadcast", "Echo's Answer", "Tender Buttons", false));

    ASSERT_TRUE(client.poll(fake));
    EXPECT_EQ(client.getArtist(), "Broadcast");
    EXPECT_EQ(client.getTitle(), "Echo's Answer");
    EXPECT_EQ(client.getAlbum(), "Tender Buttons");
}

TEST(AzuraCastClient, DetectsLiveDJ) {
    AzuraCastClient client(HOST, PORT, PATH);
    FakeClient fake;
    fake.preloadResponse(makeAzuraCastResponse(10, "Live DJ", "Song", "Album", true));

    ASSERT_TRUE(client.poll(fake));
    EXPECT_TRUE(client.isLiveDJ());
}

TEST(AzuraCastClient, HttpNon200_ReturnsFalse) {
    AzuraCastClient client(HOST, PORT, PATH);
    FakeClient fake;
    fake.preloadResponse(makeHttpResponse(500, "Internal Server Error"));

    EXPECT_FALSE(client.poll(fake));
}

TEST(AzuraCastClient, MalformedJson_ReturnsFalse) {
    AzuraCastClient client(HOST, PORT, PATH);
    FakeClient fake;
    std::string body = "not json at all {{{";
    fake.preloadResponse(makeHttpResponse(200, body));

    EXPECT_FALSE(client.poll(fake));
}

TEST(AzuraCastClient, MissingShId_ReturnsFalse) {
    AzuraCastClient client(HOST, PORT, PATH);
    FakeClient fake;
    std::string body = "{\"now_playing\":{\"song\":{\"artist\":\"A\",\"title\":\"T\",\"album\":\"Al\"}},\"live\":{\"is_live\":false}}";
    fake.preloadResponse(makeHttpResponse(200, body));

    EXPECT_FALSE(client.poll(fake));
}

TEST(AzuraCastClient, SpecialCharsInTrackData) {
    AzuraCastClient client(HOST, PORT, PATH);
    FakeClient fake;
    // Use JSON escape sequences for special characters
    std::string body = "{\"now_playing\":{\"sh_id\":77,\"song\":{\"artist\":\"Bj\\u00f6rk\",\"title\":\"Hunter\",\"album\":\"Homogenic\"}},\"live\":{\"is_live\":false}}";
    fake.preloadResponse(makeHttpResponse(200, body));

    ASSERT_TRUE(client.poll(fake));
    // ArduinoJson decodes \\u00f6 to the UTF-8 byte sequence for o-umlaut
    EXPECT_EQ(std::string(client.getArtist().c_str()).substr(0, 2), "Bj");
    EXPECT_EQ(client.getTitle(), "Hunter");
}
