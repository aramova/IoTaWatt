#include <unity.h>
#include "HashName.h"
#include <iostream>

void setUp(void) {}
void tearDown(void) {}

void test_base64encode_std(void) {
    // Known test vectors for standard base64
    // "Man" -> "TWFu"
    const char* data1 = "Man";
    std::string res1 = base64encode_std((const uint8_t*)data1, 3);
    TEST_ASSERT_EQUAL_STRING("TWFu", res1.c_str());

    // "Ma" -> "TWE="
    const char* data2 = "Ma";
    std::string res2 = base64encode_std((const uint8_t*)data2, 2);
    TEST_ASSERT_EQUAL_STRING("TWE=", res2.c_str());

    // "M" -> "TQ=="
    const char* data3 = "M";
    std::string res3 = base64encode_std((const uint8_t*)data3, 1);
    TEST_ASSERT_EQUAL_STRING("TQ==", res3.c_str());

    // Check Base64URL vs Base64 standard:
    // Standard: + /
    // URL-safe: - _
    // Let's test chars that result in 62/63 index.
    // Index 62 is 111110. Index 63 is 111111.
    // Input 3 bytes: FB FF BF -> 11111011 11111111 10111111
    // Bits: 111110 111111 111110 111111
    // Indices: 62 63 62 63
    // Chars: - _ - _
    uint8_t data4[] = {0xFB, 0xFF, 0xBF};
    std::string res4 = base64encode_std(data4, 3);
    // Standard Base64 would be "+/+/".
    // Our implementation uses "-_".
    TEST_ASSERT_EQUAL_STRING("-_-_", res4.c_str());
}

void test_hashName_std(void) {
    // With Mock SHA256:
    // reset() clears checksum.
    // update("test", 4) -> checksum = 't'+'e'+'s'+'t' = 116+101+115+116 = 448 = 0x1C0.
    // checksum (uint8_t) = 0xC0 (192).
    // finalize(hash, 6) fills hash with:
    // h[0] = (192 + 0) = 192 (0xC0)
    // h[1] = (192 + 1) = 193 (0xC1)
    // h[2] = (192 + 2) = 194 (0xC2)
    // h[3] = (192 + 3) = 195 (0xC3)
    // h[4] = (192 + 4) = 196 (0xC4)
    // h[5] = (192 + 5) = 197 (0xC5)

    // Input hash: C0 C1 C2 C3 C4 C5
    // Binary:
    // C0 = 11000000
    // C1 = 11000001
    // C2 = 11000010
    // C3 = 11000011
    // C4 = 11000100
    // C5 = 11000101

    // Chunk 1 (C0 C1 C2):
    // 110000 001100 000111 000010
    // 48     12     7      2
    // 'w'    'M'    'H'    'C'
    // Result: "wMHC"

    // Chunk 2 (C3 C4 C5):
    // 110000 111100 010011 000101
    // 48     60     19     5
    // 'w'    '8'    'T'    'F'
    // Result: "w8TF"

    // Total: "wMHCw8TF"

    std::string res = hashName_std("test");
    TEST_ASSERT_EQUAL_STRING("wMHCw8TF", res.c_str());

    // Another test: "a"
    // checksum 'a' = 97 (0x61).
    // hash: 61 62 63 64 65 66
    // 61 = 01100001
    // 62 = 01100010
    // 63 = 01100011
    // Chunk 1 (61 62 63):
    // 011000 010110 001001 100011
    // 24     22     9      35
    // 'Y'    'W'    'J'    'j'
    // Chunk 2 (64 65 66):
    // 64 = 01100100
    // 65 = 01100101
    // 66 = 01100110
    // 011001 000110 010101 100110
    // 25     6      21     38
    // 'Z'    'G'    'V'    'm'
    // Total: "YWJjZGVm"

    res = hashName_std("a");
    TEST_ASSERT_EQUAL_STRING("YWJjZGVm", res.c_str());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_base64encode_std);
    RUN_TEST(test_hashName_std);
    UNITY_END();
    return 0;
}
