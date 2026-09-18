#include <orbitra/brc.h>

#include <inttypes.h>
#include <stdio.h>
#include <time.h>

#define ORBITRA_BENCHMARK_ITERATIONS UINT32_C(10000000)

static volatile uint32_t benchmark_sink;

static double elapsed_seconds(clock_t start, clock_t end) {
    return (double)(end - start) / (double)CLOCKS_PER_SEC;
}

static void report(const char *name, clock_t start, clock_t end) {
    const double seconds = elapsed_seconds(start, end);
    const double ns_per_call = seconds * 1.0e9 / (double)ORBITRA_BENCHMARK_ITERATIONS;

    printf("%-20s %10.3f s  %10.2f ns/call\n", name, seconds, ns_per_call);
}

int main(void) {
    const uint32_t domain = UINT32_C(4093);
    orbitra_brc16_ctx_t ctx16;
    orbitra_brc32_ctx_t ctx32;
    uint16_t x16 = 0U;
    uint32_t x32 = 0U;
    uint32_t checksum = 0U;
    clock_t start;
    clock_t end;

    if (!orbitra_brc16_init(&ctx16, (uint16_t)domain, UINT32_C(0x12345678)) ||
        !orbitra_brc32_init(&ctx32, domain, UINT32_C(0x12345678))) {
        return 1;
    }

    start = clock();
    for (uint32_t i = 0U; i < ORBITRA_BENCHMARK_ITERATIONS; ++i) {
        x16 = orbitra_brc16_forward(x16, &ctx16);
        checksum ^= x16;
    }
    end = clock();
    benchmark_sink = checksum;
    report("BRC16 forward", start, end);

    checksum = 0U;
    start = clock();
    for (uint32_t i = 0U; i < ORBITRA_BENCHMARK_ITERATIONS; ++i) {
        x16 = orbitra_brc16_inverse(x16, &ctx16);
        checksum ^= x16;
    }
    end = clock();
    benchmark_sink = checksum;
    report("BRC16 inverse", start, end);

    checksum = 0U;
    start = clock();
    for (uint32_t i = 0U; i < ORBITRA_BENCHMARK_ITERATIONS; ++i) {
        x32 = orbitra_brc32_forward(x32, &ctx32);
        checksum ^= x32;
    }
    end = clock();
    benchmark_sink = checksum;
    report("BRC32 forward", start, end);

    checksum = 0U;
    start = clock();
    for (uint32_t i = 0U; i < ORBITRA_BENCHMARK_ITERATIONS; ++i) {
        x32 = orbitra_brc32_inverse(x32, &ctx32);
        checksum ^= x32;
    }
    end = clock();
    benchmark_sink = checksum;
    report("BRC32 inverse", start, end);

    checksum = 0U;
    start = clock();
    for (uint32_t i = 0U; i < ORBITRA_BENCHMARK_ITERATIONS; ++i) {
        x32 = x32 >= ctx32.threshold ? x32 - ctx32.threshold : x32 + ctx32.delta;
        checksum ^= x32;
    }
    end = clock();
    benchmark_sink = checksum;
    report("Plain rotation", start, end);

    printf("checksum=%" PRIu32 "\n", (uint32_t)benchmark_sink);
    return 0;
}
