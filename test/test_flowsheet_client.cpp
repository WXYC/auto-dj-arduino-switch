#include <gtest/gtest.h>
#include "flowsheet_client.h"
#include "fake_client.h"
#include "test_helpers.h"
#include <string>
#include <sstream>

// All tests use Client& injection with FakeClient pre-loaded HTTP responses.

static const char* HOST = "www.wxyc.info";
static const int PORT = 443;
static const char* API_KEY = "test-api-key";

// ========== Test Helpers ==========

std::string make302Response(const std::string& location) {
    std::ostringstream response;
    response << "HTTP/1.1 302 Found\r\n"
             << "Location: " << location << "\r\n"
             << "Content-Length: 0\r\n"
             << "Connection: close\r\n"
             << "\r\n";
    return response.str();
}

std::string make500Response() {
    std::string body = "Internal Server Error";
    std::ostringstream response;
    response << "HTTP/1.1 500 Internal Server Error\r\n"
             << "Content-Length: " << body.size() << "\r\n"
             << "Connection: close\r\n"
             << "\r\n"
             << body;
    return response.str();
}

std::string make302ResponseNoLocation() {
    return "HTTP/1.1 302 Found\r\n"
           "Content-Length: 0\r\n"
           "Connection: close\r\n"
           "\r\n";
}

// ========== startShow Tests ==========

TEST(FlowsheetClient, StartShow_ReturnsShowID) {
    FlowsheetClient client(HOST, PORT, API_KEY);
    FakeClient fake;
    fake.preloadResponse(make302Response(
        "/playlists/flowsheet?mode=modifyFlowsheet&radioShowID=12345"));

    int result = client.startShow(fake, 1705345200000UL);
    EXPECT_EQ(result, 12345);
}

TEST(FlowsheetClient, StartShow_SendsCorrectBody) {
    FlowsheetClient client(HOST, PORT, API_KEY);
    FakeClient fake;
    fake.preloadResponse(make302Response(
        "/playlists/flowsheet?radioShowID=1"));

    client.startShow(fake, 1705345200000UL);

    std::string request = fake.getWrittenData();
    EXPECT_NE(request.find("djID=0"), std::string::npos) << request;
    EXPECT_NE(request.find("djName=Auto+DJ"), std::string::npos) << request;
    EXPECT_NE(request.find("djHandle=AutoDJ"), std::string::npos) << request;
    EXPECT_NE(request.find("showName=Auto+DJ"), std::string::npos) << request;
    EXPECT_NE(request.find("startingHour=1705345200000"), std::string::npos) << request;
}

TEST(FlowsheetClient, StartShow_Non302_ReturnsNeg1) {
    FlowsheetClient client(HOST, PORT, API_KEY);
    FakeClient fake;
    fake.preloadResponse(make500Response());

    int result = client.startShow(fake, 1705345200000UL);
    EXPECT_EQ(result, -1);
}

TEST(FlowsheetClient, StartShow_NoLocation_ReturnsNeg1) {
    FlowsheetClient client(HOST, PORT, API_KEY);
    FakeClient fake;
    fake.preloadResponse(make302ResponseNoLocation());

    int result = client.startShow(fake, 1705345200000UL);
    EXPECT_EQ(result, -1);
}

// ========== addEntry Tests ==========

TEST(FlowsheetClient, AddEntry_ReturnsTrue_On302) {
    FlowsheetClient client(HOST, PORT, API_KEY);
    FakeClient fake;
    fake.preloadResponse(make302Response("/playlists/flowsheet"));

    bool result = client.addEntry(fake, 12345, 1705345200000UL,
                                   "Yo La Tengo", "Autumn Sweater",
                                   "I Can Hear the Heart Beating as One");
    EXPECT_TRUE(result);
}

TEST(FlowsheetClient, AddEntry_SendsCorrectBody) {
    FlowsheetClient client(HOST, PORT, API_KEY);
    FakeClient fake;
    fake.preloadResponse(make302Response("/playlists/flowsheet"));

    client.addEntry(fake, 12345, 1705345200000UL,
                    "Yo La Tengo", "Autumn Sweater",
                    "I Can Hear the Heart Beating as One");

    std::string request = fake.getWrittenData();
    EXPECT_NE(request.find("radioShowID=12345"), std::string::npos) << request;
    EXPECT_NE(request.find("workingHour=1705345200000"), std::string::npos) << request;
    EXPECT_NE(request.find("artistName=Yo+La+Tengo"), std::string::npos) << request;
    EXPECT_NE(request.find("songTitle=Autumn+Sweater"), std::string::npos) << request;
    EXPECT_NE(request.find("releaseType=otherRelease"), std::string::npos) << request;
    EXPECT_NE(request.find("autoBreakpoint=true"), std::string::npos) << request;
}

TEST(FlowsheetClient, AddEntry_SendsApiKeyHeader) {
    FlowsheetClient client(HOST, PORT, API_KEY);
    FakeClient fake;
    fake.preloadResponse(make302Response("/playlists/flowsheet"));

    client.addEntry(fake, 1, 0UL, "A", "T", "Al");

    std::string request = fake.getWrittenData();
    EXPECT_NE(request.find("X-Auto-DJ-Key: test-api-key"), std::string::npos) << request;
}

TEST(FlowsheetClient, AddEntry_Non302_ReturnsFalse) {
    FlowsheetClient client(HOST, PORT, API_KEY);
    FakeClient fake;
    fake.preloadResponse(make500Response());

    bool result = client.addEntry(fake, 12345, 1705345200000UL,
                                   "Artist", "Title", "Album");
    EXPECT_FALSE(result);
}

// ========== endShow Tests ==========

TEST(FlowsheetClient, EndShow_ReturnsTrue_On302) {
    FlowsheetClient client(HOST, PORT, API_KEY);
    FakeClient fake;
    fake.preloadResponse(make302Response("/playlists/flowsheet"));

    bool result = client.endShow(fake, 12345);
    EXPECT_TRUE(result);
}

TEST(FlowsheetClient, EndShow_SendsSignoffConfirm) {
    FlowsheetClient client(HOST, PORT, API_KEY);
    FakeClient fake;
    fake.preloadResponse(make302Response("/playlists/flowsheet"));

    client.endShow(fake, 12345);

    std::string request = fake.getWrittenData();
    EXPECT_NE(request.find("radioShowID=12345"), std::string::npos) << request;
    EXPECT_NE(request.find("mode=signoffConfirm"), std::string::npos) << request;
}
