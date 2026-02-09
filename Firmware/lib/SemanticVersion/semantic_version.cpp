#include "semantic_version.h"
#include <stdlib.h>

int32_t parseSemanticVersion(const char * ver){
    if(! ver){
        return -1;
    }
    char * ptr;
    long result = strtol(ver, &ptr, 10) << 16;
    if(*ptr == '.' || *ptr == '_'){
        long node = strtol(++ptr, &ptr, 10);
        result += node << 8;
        if(*ptr == '.' || *ptr == '_'){
            long node = strtol(++ptr, &ptr, 10);
            result += node;
        }
    }
    return result;
}
