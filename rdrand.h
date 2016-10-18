#include <assert.h>
#include <stdlib.h>
#include <nmmintrin.h>  /* import crc instrinsic */

#ifndef RDRAND_H_GUARD
#define RDRAND_H_GUARD

#ifndef RDRAND_FAIL_EXIT_CODE
#define RDRAND_FAIL_EXIT_CODE 32
#endif

static inline uint64_t rdrand64() {
    uint64_t r;
    int ok;
    asm volatile("1: .byte 0x48,0x0f,0xc7,0xf0\n\t"
    "jc 2f\n\t"
    "dec %0\n\t"
    "jnz 1b\n\t"
    "2:"
    : "=r" (ok), "=a" (r)
    : "0" (10));
    if (!ok) {
        assert(0);
        exit(RDRAND_FAIL_EXIT_CODE);
    }
    return r;
}

static inline uint32_t rdrand32() {
    static __thread char buf[8];
    static __thread int sec = 0;
    if (!sec) {
        *((uint64_t*)buf) = rdrand64();
        sec = 1;
        return *((uint32_t*)buf);
    } else {
        sec = 0;
        return *((uint32_t*)(buf+4));
    }
}

static inline uint32_t real_rdrand32() {
    uint32_t r;
    int ok;
    asm volatile("1: .byte 0x0f,0xc7,0xf0\n\t"
        "jc 2f\n\t"
        "dec %0\n\t"
        "jnz 1b\n\t"
        "2:"
    : "=r" (ok), "=a" (r)
    : "0" (10));

    if (!ok) {
        assert(0);
        exit(RDRAND_FAIL_EXIT_CODE);
    }
    return r;
}


uint32_t crc32_rand()
{
    static __thread uint32_t randnr_crc = 0;
    uint32_t crc = 42;
    if (randnr_crc == 0) {
        randnr_crc = real_rdrand32();
    }

    asm volatile(
        ".byte 0xF2, 0x0F, 0x38, 0xF1, 0xC2"
        : "+a" (randnr_crc)
        : "d" (crc));

    return randnr_crc;
}

#endif // RDRAND_H_GUARD
