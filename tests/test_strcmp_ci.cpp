#include <iostream>
#include <cassert>
#include <cctype>
#include <cstring>

// Implementation of strcmp_ci to be tested (copied from utilities.cpp, with fixes and improvements)
int strcmp_ci(const char* str1, const char* str2){
    const char* char1 = str1;
    const char* char2 = str2;
    while(*char1 || *char2){
        if(*(char1++) != *(char2++)){
            int c1 = toupper((unsigned char)*(char1-1));
            int c2 = toupper((unsigned char)*(char2-1));
            if(c1 > c2) return +1;
            if(c1 < c2) return -1;
        }
    }
    return 0;
}

void test_strcmp_ci() {
    // Exact equality
    assert(strcmp_ci("hello", "hello") == 0);
    assert(strcmp_ci("", "") == 0);
    assert(strcmp_ci("123", "123") == 0);

    // Case-insensitive equality
    assert(strcmp_ci("hello", "HELLO") == 0);
    assert(strcmp_ci("HELLO", "hello") == 0);
    assert(strcmp_ci("hElLo", "HeLlO") == 0);

    // Mismatches - same length
    assert(strcmp_ci("abc", "abd") < 0);
    assert(strcmp_ci("abd", "abc") > 0);
    assert(strcmp_ci("ABC", "abd") < 0);
    assert(strcmp_ci("abc", "ABD") < 0);

    // Mismatches - different length
    assert(strcmp_ci("abc", "abcd") < 0);
    assert(strcmp_ci("abcd", "abc") > 0);
    assert(strcmp_ci("", "a") < 0);
    assert(strcmp_ci("a", "") > 0);

    // Mixed characters
    assert(strcmp_ci("abc 123", "ABC 123") == 0);
    assert(strcmp_ci("abc-123", "abc_123") != 0);

    // Characters > 127
    assert(strcmp_ci("\xff", "\xff") == 0);
    assert(strcmp_ci("a", "\xff") < 0);
    assert(strcmp_ci("\xff", "a") > 0);

    std::cout << "All strcmp_ci tests (including improved implementation) passed!" << std::endl;
}

int main() {
    test_strcmp_ci();
    return 0;
}
