#include "semantic_version.h"
#include <stdlib.h>

int32_t parseSemanticVersion(const char * ver){
    if(! ver){
        return -1;
    }
    char * ptr;
    long major = strtol(ver, &ptr, 10);
    if (major < 0 || major > 255) {
        return -1;
    }
    int32_t result = major << 16;

    if(*ptr == '.' || *ptr == '_'){
        long minor = strtol(++ptr, &ptr, 10);
        if (minor < 0 || minor > 255) {
            return -1;
        }
        result += minor << 8;
        if(*ptr == '.' || *ptr == '_'){
            long patch = strtol(++ptr, &ptr, 10);
            if (patch < 0 || patch > 255) {
                return -1;
            }
            result += patch;
        }
    }
    return result;
}
