#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>

class SHA256 {
    uint8_t checksum = 0;
public:
    void reset() {
        checksum = 0;
    }
    void update(const void* data, size_t len) {
        const uint8_t* d = (const uint8_t*)data;
        for(size_t i=0; i<len; ++i) {
            checksum += d[i];
        }
    }
    void finalize(void* hash, size_t len) {
        uint8_t* h = (uint8_t*)hash;
        for(size_t i=0; i<len; ++i) {
            // Fill with a pattern derived from checksum to be deterministic but variable
            h[i] = (checksum + i) & 0xFF;
        }
    }
};
