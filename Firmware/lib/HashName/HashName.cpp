#include "HashName.h"
#include <SHA256.h>
#include <cstring>

// Standard Base64 URL-safe alphabet as used in common.cpp
static const char base64codes[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

std::string base64encode_std(const uint8_t* in, size_t len) {
    if (len == 0) return "";

    std::string out;
    out.reserve(((len + 2) / 3) * 4);

    size_t i = 0;
    while (i + 2 < len) {
        uint32_t octet_a = in[i++];
        uint32_t octet_b = in[i++];
        uint32_t octet_c = in[i++];

        uint32_t triple = (octet_a << 16) | (octet_b << 8) | octet_c;

        out.push_back(base64codes[(triple >> 18) & 0x3F]);
        out.push_back(base64codes[(triple >> 12) & 0x3F]);
        out.push_back(base64codes[(triple >> 6) & 0x3F]);
        out.push_back(base64codes[triple & 0x3F]);
    }

    if (len - i == 1) {
        uint32_t octet_a = in[i];
        uint32_t triple = (octet_a << 16);

        out.push_back(base64codes[(triple >> 18) & 0x3F]);
        out.push_back(base64codes[(triple >> 12) & 0x3F]);
        out.push_back('=');
        out.push_back('=');
    } else if (len - i == 2) {
        uint32_t octet_a = in[i];
        uint32_t octet_b = in[i+1];
        uint32_t triple = (octet_a << 16) | (octet_b << 8);

        out.push_back(base64codes[(triple >> 18) & 0x3F]);
        out.push_back(base64codes[(triple >> 12) & 0x3F]);
        out.push_back(base64codes[(triple >> 6) & 0x3F]);
        out.push_back('=');
    }

    return out;
}

std::string hashName_std(const char* name) {
    SHA256 sha256;
    uint8_t hash[6];
    sha256.reset();
    sha256.update(name, strlen(name));
    sha256.finalize(hash, 6);
    return base64encode_std(hash, 6);
}
