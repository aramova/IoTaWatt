#include <unity.h>
#include <semantic_version.h>

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

void test_parse_null(void) {
    TEST_ASSERT_EQUAL_INT32(-1, parseSemanticVersion(NULL));
}

void test_parse_simple_version(void) {
    TEST_ASSERT_EQUAL_INT32(0x010203, parseSemanticVersion("1.2.3"));
}

void test_parse_version_with_underscores(void) {
    TEST_ASSERT_EQUAL_INT32(0x010203, parseSemanticVersion("1_2_3"));
}

void test_parse_zero_version(void) {
    TEST_ASSERT_EQUAL_INT32(0x000000, parseSemanticVersion("0.0.0"));
}

void test_parse_max_byte_version(void) {
    TEST_ASSERT_EQUAL_INT32(0xFFFFFF, parseSemanticVersion("255.255.255"));
}

void test_parse_partial_major_minor(void) {
    // "1.2" -> 1.2.0
    TEST_ASSERT_EQUAL_INT32(0x010200, parseSemanticVersion("1.2"));
}

void test_parse_partial_major_only(void) {
    // "1" -> 1.0.0
    TEST_ASSERT_EQUAL_INT32(0x010000, parseSemanticVersion("1"));
}

void test_parse_version_ignores_extra_components(void) {
    // "1.2.3.4" should be parsed as "1.2.3"
    TEST_ASSERT_EQUAL_INT32(0x010203, parseSemanticVersion("1.2.3.4"));
}

void test_parse_invalid_characters(void) {
    // "a.b.c" -> 0.0.0
    TEST_ASSERT_EQUAL_INT32(0x000000, parseSemanticVersion("a.b.c"));
}

void test_parse_overflow_behavior(void) {
    // "0.256.0" -> 0x00010000 (1.0.0) due to overflow addition
    // This documents current behavior, even if potentially undesirable.
    TEST_ASSERT_EQUAL_INT32(0x010000, parseSemanticVersion("0.256.0"));
}

void test_parse_negative_major(void) {
    // "-1.0.0" -> result is negative
    int32_t res = parseSemanticVersion("-1.0.0");
    TEST_ASSERT_TRUE(res < 0);
}

void test_parse_negative_minor(void) {
    // "0.-1.0" -> result is negative
    int32_t res = parseSemanticVersion("0.-1.0");
    TEST_ASSERT_TRUE(res < 0);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_parse_null);
    RUN_TEST(test_parse_simple_version);
    RUN_TEST(test_parse_version_with_underscores);
    RUN_TEST(test_parse_zero_version);
    RUN_TEST(test_parse_max_byte_version);
    RUN_TEST(test_parse_partial_major_minor);
    RUN_TEST(test_parse_partial_major_only);
    RUN_TEST(test_parse_version_ignores_extra_components);
    RUN_TEST(test_parse_invalid_characters);
    RUN_TEST(test_parse_overflow_behavior);
    RUN_TEST(test_parse_negative_major);
    RUN_TEST(test_parse_negative_minor);
    return UNITY_END();
}
