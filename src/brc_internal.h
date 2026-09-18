#ifndef ORBITRA_BRC_INTERNAL_H
#define ORBITRA_BRC_INTERNAL_H

#include <stdint.h>

static uint32_t orbitra_mix32(uint32_t x) {
    x ^= x >> 16;
    x *= UINT32_C(0x85ebca6b);
    x ^= x >> 13;
    x *= UINT32_C(0xc2b2ae35);
    x ^= x >> 16;
    return x;
}

static uint32_t orbitra_reflect(uint32_t x, uint32_t n, uint32_t k, uint32_t salt) {
    const uint32_t p = k >= x ? k - x : n - (x - k);
    const uint32_t c = x < p ? x : p;
    return (orbitra_mix32(c ^ salt) & 1U) != 0U ? p : x;
}

#endif
