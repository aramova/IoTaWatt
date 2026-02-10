#pragma once
#include <string>
#include <cstring>
#include <cstdint>
#include <iostream>

#define PROGMEM
#define PGM_P const char*
#define FPSTR(x) (x)

inline void strcpy_P(char* dest, const char* src) {
    strcpy(dest, src);
}

inline size_t strlen_P(const char* str) {
    return strlen(str);
}

inline uint8_t pgm_read_byte(const char* p) {
    return (uint8_t)*p;
}

class String : public std::string {
public:
    String() : std::string() {}
    String(const char* s) : std::string(s) {}
    String(const std::string& s) : std::string(s) {}
    String(char c) : std::string(1, c) {}

    // operator += is inherited/works for const char*
    String& operator+=(char c) {
        this->push_back(c);
        return *this;
    }

    // operator [] is inherited

    int indexOf(char c) const {
        size_t pos = this->find(c);
        if (pos == std::string::npos) return -1;
        return (int)pos;
    }

    const char* c_str() const {
        return std::string::c_str();
    }
};
