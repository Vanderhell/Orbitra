#include <orbitra/brc.h>

int main(void) {
    orbitra_brc16_ctx_t ctx;

    if (!orbitra_brc16_init(&ctx, UINT16_C(13), UINT32_C(7))) {
        return 1;
    }

    return orbitra_brc16_inverse(orbitra_brc16_forward(UINT16_C(4), &ctx), &ctx) == 4U ? 0 : 2;
}
