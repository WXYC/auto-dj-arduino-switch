/**
 * FakeClient: a test double for Arduino's Client class.
 *
 * Pre-load a complete HTTP response (status line + headers + body).
 * The real ArduinoHttpClient parses it identically to a network response.
 * Written bytes (the HTTP request) are captured for assertion.
 */
#ifndef FAKE_CLIENT_H
#define FAKE_CLIENT_H

#include <Arduino.h>
#include <Client.h>
#include <string>
#include <vector>

class FakeClient : public Client {
public:
    /**
     * Pre-load a complete HTTP response. Call before HttpClient.get()/post().
     *
     * Example:
     *   fake.preloadResponse("HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nOK");
     */
    void preloadResponse(const std::string& response) {
        responseData_.assign(response.begin(), response.end());
        readPos_ = 0;
    }

    /** Returns all bytes written by HttpClient (the HTTP request). */
    std::string getWrittenData() const {
        return std::string(writtenData_.begin(), writtenData_.end());
    }

    // ========== Client interface ==========

    int connect(IPAddress, uint16_t) override {
        connected_ = true;
        return 1;
    }

    int connect(const char*, uint16_t) override {
        connected_ = true;
        return 1;
    }

    size_t write(uint8_t b) override {
        writtenData_.push_back(b);
        return 1;
    }

    size_t write(const uint8_t* buf, size_t size) override {
        writtenData_.insert(writtenData_.end(), buf, buf + size);
        return size;
    }

    int available() override {
        return static_cast<int>(responseData_.size() - readPos_);
    }

    int read() override {
        if (readPos_ < responseData_.size()) {
            return responseData_[readPos_++];
        }
        return -1;
    }

    int read(uint8_t* buf, size_t size) override {
        size_t bytesRead = 0;
        while (bytesRead < size && readPos_ < responseData_.size()) {
            buf[bytesRead++] = responseData_[readPos_++];
        }
        return static_cast<int>(bytesRead);
    }

    int peek() override {
        if (readPos_ < responseData_.size()) {
            return responseData_[readPos_];
        }
        return -1;
    }

    void flush() override {}

    void stop() override {
        connected_ = false;
    }

    uint8_t connected() override {
        return connected_ ? 1 : 0;
    }

    operator bool() override { return connected_; }

private:
    std::vector<uint8_t> responseData_;
    size_t readPos_ = 0;
    std::vector<uint8_t> writtenData_;
    bool connected_ = false;
};

#endif // FAKE_CLIENT_H
