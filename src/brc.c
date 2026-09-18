#include "brc_internal.h"
#include <orbitra/brc.h>

#include <stddef.h>

static uint32_t expand(uint32_t seed, uint32_t domain, uint32_t tag) {
    return orbitra_mix32(seed ^ orbitra_mix32(domain + tag));
}

static uint32_t gcd32(uint32_t a, uint32_t b) {
    while (b != 0U) {
        const uint32_t r = a % b;
        a = b;
        b = r;
    }
    return a;
}

static bool init32(orbitra_brc32_ctx_t *ctx, uint32_t n, uint32_t delta, uint32_t seed) {
    uint32_t i;
    if (ctx == NULL || n == 0U || delta >= n || (n > 1U && (delta == 0U || gcd32(delta, n) != 1U)))
        return false;
    ctx->domain = n;
    ctx->delta = delta;
    ctx->threshold = n - delta;
    for (i = 0U; i < 2U; ++i) {
        const uint32_t tag = i == 0U ? UINT32_C(0x42524310) : UINT32_C(0x42524320);
        const uint32_t a = expand(seed, n, tag);
        const uint32_t b = expand(seed, n, tag ^ UINT32_C(0x9e3779b9));
        ctx->round[i].pivot = a % n;
        ctx->round[i].salt = b;
    }
    return true;
}

bool orbitra_brc32_init_with_delta(orbitra_brc32_ctx_t *ctx, uint32_t domain, uint32_t delta,
                                   uint32_t seed) {
    return init32(ctx, domain, delta, seed);
}

bool orbitra_brc32_init(orbitra_brc32_ctx_t *ctx, uint32_t domain, uint32_t seed) {
    uint32_t attempt;
    if (ctx == NULL || domain == 0U)
        return false;
    if (domain == 1U)
        return init32(ctx, domain, 0U, seed);
    for (attempt = 0U; attempt < 32U; ++attempt) {
        const uint32_t delta =
            1U + (expand(seed, domain, UINT32_C(0x44454c00) + attempt) % (domain - 1U));
        if (gcd32(delta, domain) == 1U)
            return init32(ctx, domain, delta, seed);
    }
    return init32(ctx, domain, 1U, seed);
}

static uint32_t rotate_forward(uint32_t x, uint32_t n, uint32_t d, uint32_t t) {
    (void)n;
    return x >= t ? x - t : x + d;
}

static uint32_t rotate_inverse(uint32_t x, uint32_t n, uint32_t d) {
    if (x >= d)
        return x - d;
    return n - (d - x);
}

static uint32_t forward_core(uint32_t x, uint32_t domain, uint32_t delta, uint32_t threshold,
                             const orbitra_brc_round_t round[2]) {
    x = orbitra_reflect(x, domain, round[0].pivot, round[0].salt);
    x = orbitra_reflect(x, domain, round[1].pivot, round[1].salt);
    x = rotate_forward(x, domain, delta, threshold);
    x = orbitra_reflect(x, domain, round[1].pivot, round[1].salt);
    return orbitra_reflect(x, domain, round[0].pivot, round[0].salt);
}

static uint32_t inverse_core(uint32_t x, uint32_t domain, uint32_t delta,
                             const orbitra_brc_round_t round[2]) {
    x = orbitra_reflect(x, domain, round[0].pivot, round[0].salt);
    x = orbitra_reflect(x, domain, round[1].pivot, round[1].salt);
    x = rotate_inverse(x, domain, delta);
    x = orbitra_reflect(x, domain, round[1].pivot, round[1].salt);
    return orbitra_reflect(x, domain, round[0].pivot, round[0].salt);
}

uint32_t orbitra_brc32_forward(uint32_t x, const orbitra_brc32_ctx_t *ctx) {
    return forward_core(x, ctx->domain, ctx->delta, ctx->threshold, ctx->round);
}

uint32_t orbitra_brc32_inverse(uint32_t x, const orbitra_brc32_ctx_t *ctx) {
    return inverse_core(x, ctx->domain, ctx->delta, ctx->round);
}

bool orbitra_brc16_init_with_delta(orbitra_brc16_ctx_t *ctx, uint16_t domain, uint16_t delta,
                                   uint32_t seed) {
    orbitra_brc32_ctx_t tmp;
    if (ctx == NULL || !init32(&tmp, domain, delta, seed))
        return false;
    ctx->domain = domain;
    ctx->delta = delta;
    ctx->threshold = (uint16_t)tmp.threshold;
    ctx->round[0].pivot = tmp.round[0].pivot;
    ctx->round[0].salt = tmp.round[0].salt;
    ctx->round[1].pivot = tmp.round[1].pivot;
    ctx->round[1].salt = tmp.round[1].salt;
    return true;
}

bool orbitra_brc16_init(orbitra_brc16_ctx_t *ctx, uint16_t domain, uint32_t seed) {
    orbitra_brc32_ctx_t tmp;
    if (ctx == NULL || domain == 0U)
        return false;
    if (!orbitra_brc32_init(&tmp, domain, seed))
        return false;
    ctx->domain = domain;
    ctx->delta = (uint16_t)tmp.delta;
    ctx->threshold = (uint16_t)tmp.threshold;
    ctx->round[0].pivot = tmp.round[0].pivot;
    ctx->round[0].salt = tmp.round[0].salt;
    ctx->round[1].pivot = tmp.round[1].pivot;
    ctx->round[1].salt = tmp.round[1].salt;
    return true;
}

uint16_t orbitra_brc16_forward(uint16_t x, const orbitra_brc16_ctx_t *ctx) {
    return (uint16_t)forward_core(x, ctx->domain, ctx->delta, ctx->threshold, ctx->round);
}

uint16_t orbitra_brc16_inverse(uint16_t x, const orbitra_brc16_ctx_t *ctx) {
    return (uint16_t)inverse_core(x, ctx->domain, ctx->delta, ctx->round);
}
