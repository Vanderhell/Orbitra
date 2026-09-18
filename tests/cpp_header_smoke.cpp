#include <orbitra/brc.h>

int main() {
    orbitra_brc32_ctx_t ctx{};
    if (!orbitra_brc32_init(&ctx, 17U, 42U)) {
        return 1;
    }
    return orbitra_brc32_inverse(orbitra_brc32_forward(3U, &ctx), &ctx) == 3U ? 0 : 2;
}
