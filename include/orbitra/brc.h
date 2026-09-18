#ifndef ORBITRA_BRC_H
#define ORBITRA_BRC_H

#include <stdbool.h>
#include <stdint.h>

#define ORBITRA_VERSION_MAJOR 0
#define ORBITRA_VERSION_MINOR 1
#define ORBITRA_VERSION_PATCH 0

/* Version of the mapping semantics implemented by this library. */
#define ORBITRA_BRC_ALGORITHM_VERSION 1

#ifdef __cplusplus
extern "C" {
#endif

/* Context members are implementation state. Initialize once, then treat as immutable. */
typedef struct {
    uint32_t pivot;
    uint32_t salt;
} orbitra_brc_round_t;

typedef struct {
    uint16_t domain;
    uint16_t delta;
    uint16_t threshold;
    orbitra_brc_round_t round[2];
} orbitra_brc16_ctx_t;

typedef struct {
    uint32_t domain;
    uint32_t delta;
    uint32_t threshold;
    orbitra_brc_round_t round[2];
} orbitra_brc32_ctx_t;

/**
 * Initialize a deterministic full-cycle BRC16-V1 context.
 *
 * @param ctx Context to initialize; must not be NULL.
 * @param domain Domain size N, in [1, UINT16_MAX].
 * @param seed Deterministic 32-bit seed; it is not a key or entropy source.
 * @return true on success. For N == 1, the mapping is 0 -> 0.
 *
 * On failure the context is not modified. A successful context must remain
 * immutable while it is used by forward/inverse calls.
 */
bool orbitra_brc16_init(orbitra_brc16_ctx_t *ctx, uint16_t domain, uint32_t seed);

/**
 * Initialize BRC16-V1 using an explicit full-cycle increment.
 *
 * @param ctx Context to initialize; must not be NULL.
 * @param domain Domain size N, in [1, UINT16_MAX].
 * @param delta For N == 1, must be 0. Otherwise 1 <= delta < N and gcd(delta,N) == 1.
 * @param seed Deterministic seed used for the two Q rounds.
 * @return true on success; false for invalid arguments. On failure ctx is unchanged.
 */
bool orbitra_brc16_init_with_delta(orbitra_brc16_ctx_t *ctx, uint16_t domain, uint16_t delta,
                                   uint32_t seed);

/**
 * Map x forward in an initialized BRC16-V1 context.
 *
 * @param x Input in [0, ctx->domain); ctx must be non-NULL and initialized.
 * @param ctx Immutable initialized context.
 * @return The permuted value in [0, ctx->domain). For domain 1, returns 0.
 *
 * The call has fixed work and performs no allocation. Concurrent calls are
 * safe when the context is not modified and callers synchronize their own data.
 */
uint16_t orbitra_brc16_forward(uint16_t x, const orbitra_brc16_ctx_t *ctx);

/**
 * Map x backward in an initialized BRC16-V1 context.
 *
 * @param x Input in [0, ctx->domain); ctx must be non-NULL and initialized.
 * @param ctx Immutable initialized context.
 * @return The inverse-permuted value in [0, ctx->domain). For domain 1, returns 0.
 *
 * The call has fixed work and performs no allocation. Concurrent calls are
 * safe when the context is not modified and callers synchronize their own data.
 */
uint16_t orbitra_brc16_inverse(uint16_t x, const orbitra_brc16_ctx_t *ctx);

/**
 * Initialize a deterministic full-cycle BRC32-V1 context.
 *
 * @param ctx Context to initialize; must not be NULL.
 * @param domain Domain size N, in [1, UINT32_MAX].
 * @param seed Deterministic 32-bit seed; it is not a key or entropy source.
 * @return true on success. For N == 1, the mapping is 0 -> 0.
 *
 * On failure the context is not modified. A successful context must remain
 * immutable while it is used by forward/inverse calls.
 */
bool orbitra_brc32_init(orbitra_brc32_ctx_t *ctx, uint32_t domain, uint32_t seed);

/**
 * Initialize BRC32-V1 using an explicit full-cycle increment.
 *
 * @param ctx Context to initialize; must not be NULL.
 * @param domain Domain size N, in [1, UINT32_MAX].
 * @param delta For N == 1, must be 0. Otherwise 1 <= delta < N and gcd(delta,N) == 1.
 * @param seed Deterministic seed used for the two Q rounds.
 * @return true on success; false for invalid arguments. On failure ctx is unchanged.
 */
bool orbitra_brc32_init_with_delta(orbitra_brc32_ctx_t *ctx, uint32_t domain, uint32_t delta,
                                   uint32_t seed);

/**
 * Map x forward in an initialized BRC32-V1 context.
 *
 * @param x Input in [0, ctx->domain); ctx must be non-NULL and initialized.
 * @param ctx Immutable initialized context.
 * @return The permuted value in [0, ctx->domain). For domain 1, returns 0.
 *
 * The call has fixed work and performs no allocation. Concurrent calls are
 * safe when the context is not modified and callers synchronize their own data.
 */
uint32_t orbitra_brc32_forward(uint32_t x, const orbitra_brc32_ctx_t *ctx);

/**
 * Map x backward in an initialized BRC32-V1 context.
 *
 * @param x Input in [0, ctx->domain); ctx must be non-NULL and initialized.
 * @param ctx Immutable initialized context.
 * @return The inverse-permuted value in [0, ctx->domain). For domain 1, returns 0.
 *
 * The call has fixed work and performs no allocation. Concurrent calls are
 * safe when the context is not modified and callers synchronize their own data.
 */
uint32_t orbitra_brc32_inverse(uint32_t x, const orbitra_brc32_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* ORBITRA_BRC_H */
