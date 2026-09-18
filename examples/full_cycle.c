#include <orbitra/brc.h>

#include <inttypes.h>
#include <stdio.h>

int main(void) {
    orbitra_brc16_ctx_t ctx;
    const uint16_t start = UINT16_C(0);
    uint16_t x = start;

    if (!orbitra_brc16_init(&ctx, UINT16_C(13), UINT32_C(7))) {
        return 1;
    }

    for (uint32_t step = 0U; step < ctx.domain; ++step) {
        printf("step=%" PRIu32 " value=%" PRIu16 "\n", step, x);
        x = orbitra_brc16_forward(x, &ctx);
        if (step + 1U < ctx.domain && x == start) {
            return 2;
        }
    }

    return x == start ? 0 : 3;
}
