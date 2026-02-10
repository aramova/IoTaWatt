#pragma once
#include <deque>
#include <vector>
#include <string>
#include "Arduino.h" // For String

class xbuf {
public:
    std::deque<uint8_t> buffer;

    xbuf(int size = 128) {}

    size_t available() {
        return buffer.size();
    }

    void write(const uint8_t* data, size_t len) {
        for(size_t i=0; i<len; i++) {
            buffer.push_back(data[i]);
        }
    }

    size_t read(uint8_t* data, size_t len) {
        size_t count = 0;
        for(size_t i=0; i<len && !buffer.empty(); i++) {
            data[i] = buffer.front();
            buffer.pop_front();
            count++;
        }
        return count;
    }

    String readString(size_t len) {
        std::string s;
        for(size_t i=0; i<len && !buffer.empty(); i++) {
            s += (char)buffer.front();
            buffer.pop_front();
        }
        return String(s);
    }

    String peekString() {
        std::string s;
        for(auto c : buffer) s += (char)c;
        return String(s);
    }
};
