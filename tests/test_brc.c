#include "../src/brc_internal.h"
#include <inttypes.h>
#include <orbitra/brc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
#define ORBITRA_VECTOR_SCAN fscanf_s
#define ORBITRA_VARIANT_SIZE , (unsigned)sizeof variant
#else
#define ORBITRA_VECTOR_SCAN fscanf
#define ORBITRA_VARIANT_SIZE
#endif

static uint32_t mix_ref(uint32_t x) {
    x ^= x >> 16;
    x *= UINT32_C(0x85ebca6b);
    x ^= x >> 13;
    x *= UINT32_C(0xc2b2ae35);
    x ^= x >> 16;
    return x;
}
static uint32_t partner(uint32_t x, uint32_t k, uint32_t n) {
    return k >= x ? k - x : n - (x - k);
}
static uint32_t rr(uint32_t x, uint32_t n, uint32_t k, uint32_t s) {
    uint32_t p = partner(x, k, n), c = x < p ? x : p;
    return (mix_ref(c ^ s) & 1U) != 0U ? p : x;
}
static uint32_t rot(uint32_t x, uint32_t n, uint32_t d) {
    return (uint32_t)(((uint64_t)x + d) % n);
}
static uint32_t roti(uint32_t x, uint32_t n, uint32_t d) {
    return (uint32_t)(((uint64_t)x + n - d) % n);
}
static uint32_t rf(uint32_t x, const orbitra_brc32_ctx_t *c) {
    x = rr(x, c->domain, c->round[0].pivot, c->round[0].salt);
    x = rr(x, c->domain, c->round[1].pivot, c->round[1].salt);
    x = rot(x, c->domain, c->delta);
    x = rr(x, c->domain, c->round[1].pivot, c->round[1].salt);
    return rr(x, c->domain, c->round[0].pivot, c->round[0].salt);
}
static uint32_t ri(uint32_t x, const orbitra_brc32_ctx_t *c) {
    x = rr(x, c->domain, c->round[0].pivot, c->round[0].salt);
    x = rr(x, c->domain, c->round[1].pivot, c->round[1].salt);
    x = roti(x, c->domain, c->delta);
    x = rr(x, c->domain, c->round[1].pivot, c->round[1].salt);
    return rr(x, c->domain, c->round[0].pivot, c->round[0].salt);
}
static uint32_t gcd_ref(uint32_t a, uint32_t b) {
    while (b != 0U) {
        uint32_t r = a % b;
        a = b;
        b = r;
    }
    return a;
}
static int fail(const char *s, uint32_t n, uint32_t x) {
    fprintf(stderr, "%s N=%" PRIu32 " x=%" PRIu32 "\n", s, n, x);
    return 1;
}
static int test_init_cycle_delta(orbitra_brc32_ctx_t *ctx, uint32_t n, uint32_t delta,
                                 uint32_t seed) {
    if (n == 0U || delta >= n || !orbitra_brc32_init_with_delta(ctx, n, 1U, seed))
        return 0;
    ctx->delta = delta;
    ctx->threshold = n - delta;
    return 1;
}

int main(void) {
    uint32_t n, seed, x;
    orbitra_brc32_ctx_t c;
    if (orbitra_brc16_init(NULL, 1U, 0U) || orbitra_brc32_init(&c, 0U, 0U) ||
        orbitra_brc32_init_with_delta(&c, 7U, 7U, 0U) ||
        orbitra_brc32_init_with_delta(&c, 8U, 4U, 0U))
        return fail("invalid init", 0, 0);
    for (n = 1U; n <= 4096U; ++n)
        for (seed = 0U; seed < 4U; ++seed) {
            if (!orbitra_brc32_init(&c, n, seed))
                return fail("init", n, seed);
            for (x = 0U; x < n; ++x)
                for (uint32_t r = 0U; r < 2U; ++r) {
                    uint32_t y = orbitra_reflect(x, n, c.round[r].pivot, c.round[r].salt);
                    if (orbitra_reflect(y, n, c.round[r].pivot, c.round[r].salt) != x)
                        return fail("reflection involution", n, x);
                }
            for (x = 0U; x < n; ++x) {
                uint32_t y = orbitra_brc32_forward(x, &c), z = orbitra_brc32_inverse(x, &c);
                if (y >= n || z >= n)
                    return fail("range", n, x);
                if (orbitra_brc32_inverse(y, &c) != x || orbitra_brc32_forward(z, &c) != x)
                    return fail("inverse", n, x);
                if (y != rf(x, &c) || z != ri(x, &c))
                    return fail("reference", n, x);
                if (n > 1U && y == x)
                    return fail("fixed point", n, x);
            }
            if (n > 1U) {
                uint32_t y = 0U;
                for (x = 0U; x < n; ++x) {
                    y = orbitra_brc32_forward(y, &c);
                    if (y == 0U && x + 1U < n)
                        return fail("early cycle", n, x + 1U);
                }
                if (y != 0U)
                    return fail("cycle closure", n, y);
            }
        }
    for (n = 1U; n <= 4096U; ++n)
        for (seed = 0U; seed < 4U; ++seed) {
            orbitra_brc16_ctx_t s;
            orbitra_brc32_ctx_t ref;
            if (!orbitra_brc16_init(&s, (uint16_t)n, seed))
                return fail("init16", n, seed);
            ref.domain = s.domain;
            ref.delta = s.delta;
            ref.threshold = s.threshold;
            ref.round[0].pivot = s.round[0].pivot;
            ref.round[0].salt = s.round[0].salt;
            ref.round[1].pivot = s.round[1].pivot;
            ref.round[1].salt = s.round[1].salt;
            for (x = 0U; x < n; ++x) {
                uint16_t y = orbitra_brc16_forward((uint16_t)x, &s);
                if (orbitra_brc16_inverse(y, &s) != (uint16_t)x)
                    return fail("inverse16", n, x);
                if ((uint32_t)y != rf(x, &ref) ||
                    orbitra_brc16_inverse((uint16_t)x, &s) != ri(x, &ref))
                    return fail("reference16", n, x);
                if (n > 1U && y == (uint16_t)x)
                    return fail("fixed16", n, x);
            }
            if (n > 1U) {
                uint16_t y = 0U;
                for (x = 0U; x < n; ++x) {
                    y = orbitra_brc16_forward(y, &s);
                    if (y == 0U && x + 1U < n)
                        return fail("early cycle16", n, x + 1U);
                }
                if (y != 0U)
                    return fail("cycle closure16", n, y);
            }
        }
    {
        const uint16_t domains[] = {UINT16_C(65521), UINT16_C(65535)};
        for (size_t j = 0U; j < sizeof domains / sizeof domains[0]; ++j)
            for (seed = 0U; seed < 4U; ++seed) {
                orbitra_brc16_ctx_t s;
                uint32_t dn = domains[j];
                if (!orbitra_brc16_init(&s, domains[j], seed))
                    return fail("large init16", dn, seed);
                for (x = 0U; x < dn; ++x) {
                    uint16_t y = orbitra_brc16_forward((uint16_t)x, &s);
                    if (y >= dn || orbitra_brc16_inverse(y, &s) != (uint16_t)x ||
                        (dn > 1U && y == (uint16_t)x))
                        return fail("large domain16", dn, x);
                }
            }
    }
    {
        const uint32_t domains[] = {UINT32_MAX, UINT32_MAX - 1U,
                                    65535U,     65536U,
                                    65537U,     1000000U,
                                    4093U,      4096U,
                                    4097U,      257U,
                                    256U,       255U,
                                    33U,        32U,
                                    31U,        3U,
                                    2U,         1U};
        for (size_t j = 0U; j < sizeof domains / sizeof domains[0]; ++j) {
            n = domains[j];
            if (!orbitra_brc32_init(&c, n, 123U))
                return fail("boundary init", n, 0);
            for (x = 0U; x < 10000U; ++x) {
                uint32_t state = mix_ref(x + UINT32_C(0x1234567)), v = state % n;
                uint32_t y = orbitra_brc32_forward(v, &c);
                if (y != rf(v, &c) || orbitra_brc32_inverse(y, &c) != v || (n > 1U && y == v))
                    return fail("boundary", n, v);
            }
        }
    }
    {
        uint32_t state = UINT32_C(0x8f31a29d);
        for (uint32_t i = 0U; i < 2000000U; ++i) {
            state = mix_ref(state + i);
            n = state == 0U ? UINT32_MAX : state;
            state = mix_ref(state);
            seed = state;
            state = mix_ref(state);
            x = state % n;
            if (!orbitra_brc32_init(&c, n, seed))
                return fail("property init", n, i);
            uint32_t y = orbitra_brc32_forward(x, &c);
            if (y != rf(x, &c) || orbitra_brc32_inverse(y, &c) != x || (n > 1U && y == x))
                return fail("property", n, x);
        }
    }
    for (n = 2U; n <= 128U; ++n)
        for (uint32_t d = 0U; d < n; ++d) {
            orbitra_brc32_ctx_t t;
            uint32_t seen[128] = {0}, cycles = 0U;
            if (!test_init_cycle_delta(&t, n, d, 55U))
                return fail("cycle init", n, d);
            for (x = 0U; x < n; ++x)
                if (seen[x] == 0U) {
                    uint32_t y = x, len = 0U;
                    do {
                        seen[y] = 1U;
                        ++len;
                        y = orbitra_brc32_forward(y, &t);
                    } while (y != x && len <= n);
                    if (len != n / gcd_ref(d, n))
                        return fail("cycle length", n, d);
                    ++cycles;
                }
            if (cycles != gcd_ref(d, n))
                return fail("cycle count", n, d);
        }
    {
        FILE *f = fopen(ORBITRA_VECTOR_FILE, "r");
        char variant[16], header[128];
        uint32_t vn, vs, vd, vi, vf, vir;
        size_t rows = 0U;
        int trailing;
        if (f == NULL)
            return fail("vector file", 0, 0);
        if (fgets(header, sizeof header, f) == NULL ||
            strcmp(header, "variant,domain,seed,delta,input,forward,inverse\n") != 0)
            return fail("vector header", 0, 0);
        while (ORBITRA_VECTOR_SCAN(
                   f,
                   "%15[^,],%" SCNu32 ",%" SCNu32 ",%" SCNu32 ",%" SCNu32 ",%" SCNu32 ",%" SCNu32 "\n",
                   variant ORBITRA_VARIANT_SIZE, &vn, &vs, &vd, &vi, &vf, &vir) == 7) {
            if (strcmp(variant, "BRC16-V1") == 0) {
                orbitra_brc16_ctx_t q;
                if (vn > UINT16_MAX || !orbitra_brc16_init(&q, (uint16_t)vn, vs) || q.delta != vd ||
                    orbitra_brc16_forward((uint16_t)vi, &q) != vf ||
                    orbitra_brc16_inverse((uint16_t)vi, &q) != vir)
                    return fail("BRC16 vector", vn, vi);
                if (vn == 0U || vi >= vn)
                    return fail("vector range", vn, vi);
            } else if (strcmp(variant, "BRC32-V1") == 0) {
                orbitra_brc32_ctx_t q;
                if (!orbitra_brc32_init(&q, vn, vs) || q.delta != vd ||
                    orbitra_brc32_forward(vi, &q) != vf || orbitra_brc32_inverse(vi, &q) != vir)
                    return fail("BRC32 vector", vn, vi);
                if (vn == 0U || vi >= vn)
                    return fail("vector range", vn, vi);
            } else
                return fail("vector variant", vn, vi);
            ++rows;
        }
        trailing = fgetc(f);
        if (ferror(f) || trailing != EOF || rows != 480U)
            return fail("vector count", (uint32_t)rows, 480U);
        fclose(f);
    }
    printf("BRC validation passed; contexts: BRC16=%zu bytes BRC32=%zu bytes\n",
           sizeof(orbitra_brc16_ctx_t), sizeof(orbitra_brc32_ctx_t));
    return 0;
}
