#include <stdio.h>
#include <stdint.h>
#include "rdrand.h"

int main () {
    uint32_t i = 0;
    printf("iteration,type,value\n");
    for (; i <= 10000; i++) {
        uint32_t r32;
        uint64_t r64;
        rdrand32(&r32);
        printf("%u,%s,%u\n", i, "rdrand32", r32);
        rdrand64(&r64);
        printf("%u,%s,%lu\n", i, "rdrand64", r64);
    }
    return 0;
}
