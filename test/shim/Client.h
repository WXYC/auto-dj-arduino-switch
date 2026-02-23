/**
 * Arduino Client abstract class for desktop testing.
 *
 * Mirrors the Arduino core Client class: extends Stream with connection
 * management and buffered write. FakeClient (test/fake_client.h) provides
 * a concrete implementation for injecting pre-loaded HTTP responses.
 */
#ifndef CLIENT_H_SHIM
#define CLIENT_H_SHIM

#include <Arduino.h>
#include "IPAddress.h"

class Client : public Stream {
public:
    virtual ~Client() {}
    virtual int connect(IPAddress ip, uint16_t port) = 0;
    virtual int connect(const char* host, uint16_t port) = 0;
    virtual size_t write(uint8_t b) = 0;
    virtual size_t write(const uint8_t* buf, size_t size) = 0;
    virtual int available() = 0;
    virtual int read() = 0;
    virtual int read(uint8_t* buf, size_t size) = 0;
    virtual int peek() = 0;
    virtual void flush() = 0;
    virtual void stop() = 0;
    virtual uint8_t connected() = 0;
    virtual operator bool() = 0;
};

#endif // CLIENT_H_SHIM
