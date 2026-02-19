/**
 * Minimal Arduino String shim for desktop testing.
 *
 * Implements just enough of the Arduino String class to compile and test
 * the pure functions extracted into utils.h/utils.cpp. This is NOT a
 * complete Arduino compatibility layer -- only the subset used by utils.cpp.
 */
#ifndef ARDUINO_H_SHIM
#define ARDUINO_H_SHIM

#include <string>
#include <cstring>
#include <cctype>
#include <cstdio>
#include <sstream>

#define HEX 16

inline bool isAlphaNumeric(char c) {
    return std::isalnum(static_cast<unsigned char>(c));
}

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

    void reserve(unsigned int size) {
        data_.reserve(size);
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

#endif // ARDUINO_H_SHIM
