/**
 * Arduino shim for desktop testing.
 *
 * Provides the subset of the Arduino API needed to compile and test
 * sketch modules (RelayMonitor, AzuraCastClient, FlowsheetClient) on
 * desktop using GoogleTest. Includes String, Print, Stream, Serial,
 * GPIO stubs, and timing functions.
 */
#ifndef ARDUINO_H_SHIM
#define ARDUINO_H_SHIM

#include <string>
#include <cstring>
#include <cctype>
#include <cstdio>
#include <cstdint>
#include <sstream>

// ========== Constants ==========

#define HEX 16
#define DEC 10
#define HIGH 1
#define LOW 0
#define INPUT_PULLUP 2
#define OUTPUT 1
#define LED_BUILTIN 13

typedef uint8_t byte;

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

// ========== Character helpers ==========

inline bool isAlphaNumeric(char c) {
    return std::isalnum(static_cast<unsigned char>(c));
}

inline bool isHexadecimalDigit(char c) {
    return std::isxdigit(static_cast<unsigned char>(c));
}

inline bool isSpace(char c) {
    return std::isspace(static_cast<unsigned char>(c));
}

// ========== String ==========

class String {
public:
    String() : data_() {}
    String(const char* s) : data_(s ? s : "") {}
    String(const String& other) : data_(other.data_) {}

    // Numeric constructors
    String(int val) {
        data_ = std::to_string(val);
    }
    String(unsigned long val) {
        data_ = std::to_string(val);
    }
    String(unsigned char val, int base) {
        if (base == HEX) {
            char buf[8];
            std::snprintf(buf, sizeof(buf), "%x", val);
            data_ = buf;
        } else {
            data_ = std::to_string(val);
        }
    }

    unsigned int length() const { return static_cast<unsigned int>(data_.size()); }
    char charAt(unsigned int index) const { return data_[index]; }
    char operator[](unsigned int index) const { return data_[index]; }
    char& operator[](unsigned int index) { return data_[index]; }

    int indexOf(const char* str) const {
        auto pos = data_.find(str);
        return pos == std::string::npos ? -1 : static_cast<int>(pos);
    }
    int indexOf(char c) const {
        auto pos = data_.find(c);
        return pos == std::string::npos ? -1 : static_cast<int>(pos);
    }

    String substring(unsigned int from) const {
        if (from >= data_.size()) return String("");
        return String(data_.substr(from).c_str());
    }
    String substring(unsigned int from, unsigned int to) const {
        if (from >= data_.size()) return String("");
        return String(data_.substr(from, to - from).c_str());
    }

    int toInt() const {
        try {
            return std::stoi(data_);
        } catch (...) {
            return 0;
        }
    }

    unsigned char reserve(unsigned int size) {
        data_.reserve(size);
        return 1;
    }

    bool concat(char c) {
        data_ += c;
        return true;
    }
    bool concat(const char* s) {
        if (s) data_ += s;
        return true;
    }
    bool concat(const String& s) {
        data_ += s.data_;
        return true;
    }

    bool equalsIgnoreCase(const String& other) const {
        if (data_.size() != other.data_.size()) return false;
        for (size_t i = 0; i < data_.size(); i++) {
            if (std::tolower(static_cast<unsigned char>(data_[i])) !=
                std::tolower(static_cast<unsigned char>(other.data_[i]))) {
                return false;
            }
        }
        return true;
    }

    String& operator+=(char c) {
        data_ += c;
        return *this;
    }
    String& operator+=(const char* s) {
        data_ += s;
        return *this;
    }
    String& operator+=(const String& s) {
        data_ += s.data_;
        return *this;
    }

    String operator+(const char* s) const {
        String result(*this);
        result.data_ += s;
        return result;
    }
    String operator+(const String& s) const {
        String result(*this);
        result.data_ += s.data_;
        return result;
    }

    friend String operator+(const char* lhs, const String& rhs) {
        String result(lhs);
        result.data_ += rhs.data_;
        return result;
    }

    bool operator==(const char* s) const { return data_ == s; }
    bool operator==(const String& s) const { return data_ == s.data_; }
    bool operator!=(const char* s) const { return data_ != s; }

    const char* c_str() const { return data_.c_str(); }

    // For test assertions
    const std::string& str() const { return data_; }

private:
    std::string data_;
};

// Allow gtest to print String values
inline std::ostream& operator<<(std::ostream& os, const String& s) {
    return os << s.c_str();
}

// ========== Printable ==========

class Print; // forward declaration

class Printable {
public:
    virtual ~Printable() {}
    virtual size_t printTo(Print& p) const = 0;
};

// ========== Print ==========

class Print {
public:
    virtual ~Print() {}
    virtual size_t write(uint8_t b) = 0;

    virtual size_t write(const uint8_t* buffer, size_t size) {
        size_t n = 0;
        while (size--) {
            if (write(*buffer++)) n++;
            else break;
        }
        return n;
    }

    size_t write(const char* str) {
        if (!str) return 0;
        return write(reinterpret_cast<const uint8_t*>(str), std::strlen(str));
    }

    size_t print(const char* s) { return write(s); }
    size_t print(char c) { return write(static_cast<uint8_t>(c)); }
    size_t print(int n, int = DEC) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%d", n);
        return write(buf);
    }
    size_t print(unsigned int n, int = DEC) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%u", n);
        return write(buf);
    }
    size_t print(unsigned long n, int = DEC) {
        char buf[24];
        std::snprintf(buf, sizeof(buf), "%lu", n);
        return write(buf);
    }
    size_t print(const String& s) { return write(s.c_str()); }

    size_t println() { return write(reinterpret_cast<const uint8_t*>("\r\n"), 2); }
    size_t println(const char* s) { size_t n = print(s); return n + println(); }
    size_t println(char c) { size_t n = print(c); return n + println(); }
    size_t println(int n, int base = DEC) { size_t n2 = print(n, base); return n2 + println(); }
    size_t println(unsigned long n, int base = DEC) { size_t n2 = print(n, base); return n2 + println(); }
    size_t println(const String& s) { size_t n = print(s); return n + println(); }
};

// ========== Stream ==========

class Stream : public Print {
public:
    virtual ~Stream() {}
    virtual int available() = 0;
    virtual int read() = 0;
    virtual int peek() = 0;

    void setTimeout(unsigned long timeout) { _timeout = timeout; }
    unsigned long getTimeout() const { return _timeout; }

    // Implemented in shim.cpp (depends on millis())
    int timedRead();
    String readString();

    size_t readBytes(char* buffer, size_t length) {
        size_t count = 0;
        while (count < length) {
            int c = timedRead();
            if (c < 0) break;
            *buffer++ = static_cast<char>(c);
            count++;
        }
        return count;
    }
    size_t readBytes(uint8_t* buffer, size_t length) {
        return readBytes(reinterpret_cast<char*>(buffer), length);
    }

protected:
    unsigned long _timeout = 1000;
};

// ========== Serial ==========

class NullSerial : public Stream {
public:
    void begin(unsigned long) {}
    size_t write(uint8_t) override { return 1; }
    int available() override { return 0; }
    int read() override { return -1; }
    int peek() override { return -1; }
    operator bool() const { return true; }
};

extern NullSerial Serial;

// ========== GPIO ==========

inline void pinMode(int, int) {}
inline int digitalRead(int) { return HIGH; }
inline void digitalWrite(int, int) {}

// ========== Timing ==========

unsigned long millis();
inline void delay(unsigned long) {}

#endif // ARDUINO_H_SHIM
