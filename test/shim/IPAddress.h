/**
 * Minimal IPAddress stub for desktop testing.
 *
 * Only needed because Client.h references IPAddress in the connect() signature.
 * Our code never connects by IP address.
 */
#ifndef IPADDRESS_H_SHIM
#define IPADDRESS_H_SHIM

#include <cstdint>

class IPAddress {
public:
    IPAddress() : _address{0, 0, 0, 0} {}
    IPAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d) : _address{a, b, c, d} {}
private:
    uint8_t _address[4];
};

#endif // IPADDRESS_H_SHIM
