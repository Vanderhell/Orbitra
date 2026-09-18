#include <orbitra/brc.h>

#include <inttypes.h>
#include <stdio.h>

int main(void) {
    orbitra_brc32_ctx_t ctx;
    const uint32_t block_count = UINT32_C(64);

    if (!orbitra_brc32_init(&ctx, block_count, UINT32_C(1234))) {
        return 1;
    }

    /* This example orders block indices only; it is not a RAM test. */
    for (uint32_t logical = 0U; logical < block_count; ++logical) {
        const uint32_t physical = orbitra_brc32_forward(logical, &ctx);
        printf("logical=%" PRIu32 " physical=%" PRIu32 "\n", logical, physical);
    }

    return 0;
}
