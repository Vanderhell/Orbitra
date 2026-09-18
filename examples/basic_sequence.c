#include <orbitra/brc.h>

#include <inttypes.h>
#include <stdio.h>

int main(void) {
    orbitra_brc32_ctx_t ctx;

    if (!orbitra_brc32_init(&ctx, UINT32_C(17), UINT32_C(42))) {
        return 1;
    }

    for (uint32_t x = 0U; x < ctx.domain; ++x) {
        printf("%" PRIu32 " -> %" PRIu32 "\n", x, orbitra_brc32_forward(x, &ctx));
    }

    return 0;
}
