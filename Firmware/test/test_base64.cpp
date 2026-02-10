#include <unity.h>
#include "base64.h"

// Mocks are included via build flags or separate compilation units in PlatformIO usually.
// base64codes_P is provided by base64.cpp now.

// Mock trace if not linked (Unity tests usually link against mocks)
// In native environment, we might need to implement it here or link a mock object.
// We'll declare it here to satisfy linker if compiled as single unit.
#ifdef __cplusplus
extern "C" {
#endif
void trace(const uint8_t module, const uint8_t id, const uint8_t det) {
    // no-op
}
#ifdef __cplusplus
}
#endif

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

void test_empty(void) {
    String res = base64encode((const uint8_t*)"", 0);
    TEST_ASSERT_EQUAL_STRING("", res.c_str());
}

void test_hello_world(void) {
    const char* input = "Hello World";
    String res = base64encode((const uint8_t*)input, strlen(input));
    TEST_ASSERT_EQUAL_STRING("SGVsbG8gV29ybGQ=", res.c_str());
}

void test_binary(void) {
    uint8_t input[] = {0xFF, 0x00, 0xFE};
    String res = base64encode(input, 3);
    TEST_ASSERT_EQUAL_STRING("_wD-", res.c_str());
}

void test_padding_1(void) {
    const char* input = "A";
    String res = base64encode((const uint8_t*)input, strlen(input));
    TEST_ASSERT_EQUAL_STRING("QQ==", res.c_str());
}

void test_padding_2(void) {
    const char* input = "AB";
    String res = base64encode((const uint8_t*)input, strlen(input));
    TEST_ASSERT_EQUAL_STRING("QUI=", res.c_str());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_empty);
    RUN_TEST(test_hello_world);
    RUN_TEST(test_binary);
    RUN_TEST(test_padding_1);
    RUN_TEST(test_padding_2);
    return UNITY_END();
}
