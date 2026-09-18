#include <orbitra/brc.h>

int main(void) {
    orbitra_brc16_ctx_t small;
    orbitra_brc32_ctx_t large;
    if (!orbitra_brc16_init(&small, 11U, 9U) || !orbitra_brc32_init(&large, 100003U, 9U))
        return 1;
    return orbitra_brc16_inverse(orbitra_brc16_forward(3U, &small), &small) == 3U &&
                   orbitra_brc32_inverse(orbitra_brc32_forward(70000U, &large), &large) == 70000U
               ? 0
               : 2;
}
