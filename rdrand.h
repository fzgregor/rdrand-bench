#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#ifndef RDRAND_H_GUARD
#define RDRAND_H_GUARD

#ifndef RDRAND_FAIL_EXIT_CODE
#define RDRAND_FAIL_EXIT_CODE 32
#endif

static inline void rdrand64(uint64_t *v) {
    int ok;
    asm volatile("1: .byte 0x48,0x0f,0xc7,0xf0\n\t"
    "jc 2f\n\t"
    "dec %0\n\t"
    "jnz 1b\n\t"
    "2:"
    : "=r" (ok), "=a" (*v)
    : "0" (10));
    if (!ok) {
        assert(false);
        exit(RDRAND_FAIL_EXIT_CODE);
    }
}

static inline void rdrand32(uint32_t *v) {
    static __thread char buf[8];
    static __thread bool sec = false;
    if (!sec) {
        rdrand64((uint64_t*)buf);
        sec = true;
        *v = *((uint32_t*)buf);
    } else {
        sec = false;
        *v = *((uint32_t*)(buf+4));
    }
    return;
}

static inline void real_rdrand32(uint32_t *v) {
    int ok;
    asm volatile("1: .byte 0x0f,0xc7,0xf0\n\t"
    "jc 2f\n\t"
    "dec %0\n\t"
    "jnz 1b\n\t"
    "2:"
    : "=r" (ok), "=a" (*v)
    : "0" (10));
    if (!ok) {
        assert(false);
        exit(RDRAND_FAIL_EXIT_CODE);
    }
}

#endif // RDRAND_H_GUARD