# Orbitra

Orbitra is a small C11 library for deterministic, reversible traversal of arbitrary bounded integer domains. BRC16 and BRC32 map `[0, N)` onto itself using constant memory. In full-cycle mode, repeated application visits every domain element exactly once before returning to the starting point.

For every valid initialized context, forward and inverse are exact permutations of the domain. A full-cycle context has one cycle of length `N`; for `N > 1`, it has no fixed points. `N = 1` maps `0` to `0`.

## Why Orbitra

Many embedded tasks need a repeatable traversal of a finite range without allocating a table proportional to that range. Orbitra provides a deterministic relabeling of a full-domain cycle for RAM block ordering, flash-page traversal ordering, device/channel enumeration, deterministic test scheduling, and logical-to-physical index permutation.

Plain modular rotation is substantially cheaper and is preferable when simple full-cycle traversal is sufficient. The two reflections provide a different deterministic labeling; measured dispersion is characterization only, not a guarantee for every seed and domain.

## Guarantees

For a valid context and `x` in `[0, N)`:

- `forward(x)` and `inverse(x)` both remain in `[0, N)`.
- `inverse(forward(x)) == x` and `forward(inverse(x)) == x`.
- Every value has exactly one predecessor and one successor.
- Full-cycle initialization chooses `delta` with `gcd(delta, N) == 1`.
- The resulting permutation has exactly one cycle of length `N` and no fixed points when `N > 1`.
- For `N == 1`, the only mapping is `0 -> 0`.

These are structural properties of the construction, not probabilistic claims.

## Quick start

```c
#include <orbitra/brc.h>

#include <stdint.h>

static void process_block(uint32_t block_index)
{
    /* Replace with application work. */
    (void)block_index;
}

int main(void)
{
    orbitra_brc32_ctx_t ctx;
    const uint32_t domain = UINT32_C(4093);
    uint32_t x = 0U;

    if (!orbitra_brc32_init(&ctx, domain, UINT32_C(12345))) {
        return 1;
    }

    for (uint32_t i = 0U; i < domain; ++i) {
        process_block(x);
        x = orbitra_brc32_forward(x, &ctx);
    }

    return x == 0U ? 0 : 2;
}
```

## How it works

BRC constructs a permutation by conjugating a domain rotation:

```text
P = Q^-1 o D o Q
D(x) = (x + delta) mod N
```

`Q` is exactly two deterministic conditional-reflection involutions, `R0` then `R1`. Its inverse applies `R1` then `R0`. The rotation increment is coprime to the domain for full-cycle initialization. Conjugation changes the labeling and order of the cycle without changing its cycle structure.

More generally, translation by `delta` has `gcd(delta, N)` cycles, each of length `N / gcd(delta, N)`. The V1 public initializers select only full-cycle parameters (with the one-element domain handled as `0 -> 0`).

## API

Include `<orbitra/brc.h>` and link the `orbitra` CMake target. The API provides seeded and explicit-increment initializers plus forward and inverse functions for BRC16 and BRC32. Context members are implementation state: initialize a context once and leave it immutable during use. Calls can read the same immutable context concurrently; synchronization of surrounding application data remains the caller's responsibility.

The public functions are `orbitra_brc16_init`, `orbitra_brc16_init_with_delta`, `orbitra_brc16_forward`, `orbitra_brc16_inverse`, and the corresponding `orbitra_brc32_*` functions.

Domains are `1..UINT16_MAX` for BRC16 and `1..UINT32_MAX` for BRC32. Explicit-increment initialization requires `delta == 0` for `N == 1`; otherwise `1 <= delta < N` and `gcd(delta, N) == 1`. Forward and inverse require a non-null initialized context and an input in `[0, N)`.

The hot path uses fixed work, no heap, no runtime division or modulo, and no retry loop. Initialization uses bounded candidate search and may use division/modulo.

## Full-cycle traversal

Given a starting value `x0`, apply `x = orbitra_brc32_forward(x, &ctx)` exactly `N` times. The values before each update visit the whole domain once; after the `N`th update, `x` returns to `x0`. See [`examples/full_cycle.c`](examples/full_cycle.c).

## Direct permutation use

The mapping can also permute indices directly: `physical = orbitra_brc32_forward(logical, &ctx)`. This is useful for deterministic ordering. [`examples/ram_block_order.c`](examples/ram_block_order.c) demonstrates index ordering only; it is not a RAM diagnostic.

## Embedded characteristics

The production core is freestanding-friendly C11 and uses only fixed-size context state. It has no heap use, mutable global state, runtime RNG, floating point, recursion, or lookup table proportional to the domain. It uses no division or modulo in forward/inverse.

## Build

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
cmake --install build --prefix install
```

An installed consumer can use `find_package(Orbitra CONFIG REQUIRED)` and link `Orbitra::orbitra`:

```cmake
find_package(Orbitra CONFIG REQUIRED)
target_link_libraries(firmware PRIVATE Orbitra::orbitra)
```

When Orbitra is added as a subdirectory, tests and examples default off so they do not add targets to a parent project unexpectedly.

Build the optional benchmark with `-DORBITRA_BUILD_BENCHMARKS=ON`. Its measurements are desktop/toolchain dependent and do not predict MCU timing.

## Tests

The default test suite checks the frozen vectors, invalid initialization, the public C and C++ headers, and an installed-package consumer. The BRC validation test also performs exhaustive domains 1 through 4096 over four seeds, larger BRC16 boundary domains, full-orbit checks, reference comparisons, two million generated BRC32 cases, and cycle-formula tests through domain 128.

Strict warnings are enabled for the library, examples, and tests. CI runs GCC and Clang builds plus ASan/UBSan test runs.

## Compatibility

`BRC16-V1` and `BRC32-V1` mapping semantics are frozen. V1 uses the 32-bit MurmurHash3 finalizer with multipliers `0x85ebca6b` and `0xc2b2ae35`; seed expansion is `mix(seed ^ mix(domain + tag))`. Delta tags are `0x44454c00 + attempt`; Q tags are `0x42524310` and `0x42524320`; each round salt is its tag XOR `0x9e3779b9`. Delta generation uses at most 32 candidates and falls back to 1. The two Q rounds and all these values are fixed. The 480 data rows in [`tests/vectors_v1.csv`](tests/vectors_v1.csv) are checked by regression tests; normal builds and tests never regenerate them. C struct layout is not a serialization format.

## Resource footprint

Context sizes measured with GCC are 24 bytes for BRC16 and 28 bytes for BRC32. Freestanding source-object sizes below were measured with arm-none-eabi-gcc 13.2.1 using `-Os`. The BRC16/BRC32/combined links use Cortex-M0, section garbage collection, and libgcc; the sizes include the division helper used only during initialization.

| Target/build | Text | Data | BSS |
| --- | ---: | ---: | ---: |
| Cortex-M0 combined object | 708 B | 0 B | 0 B |
| Cortex-M3 combined object | 678 B | 0 B | 0 B |
| Cortex-M4 combined object | 680 B | 0 B | 0 B |
| Cortex-M0 BRC16-only smoke link | 944 B | 0 B | 24 B |
| Cortex-M0 BRC32-only smoke link | 876 B | 0 B | 28 B |
| Cortex-M0 combined smoke link | 1012 B | 0 B | 52 B |
| RV32I combined object (Clang 18.1.3, `-Os`) | 2548 B | 0 B | 0 B |
| RV32IM combined object (Clang 18.1.3, `-Os`) | 1712 B | 0 B | 0 B |

GCC `-fstack-usage` reports per-function frames. Summing nested frames gives conservative library-only maxima of 72 B for Cortex-M0 forward calls and 120 B for its initialization chain; corresponding Cortex-M3/M4 estimates are 64 B and 96 B. These exclude the caller, interrupt handlers, RTOS context, and platform startup. Treat all sizes as toolchain/configuration-specific estimates, not timing measurements.

## Verified targets

Strict freestanding cross-compilation has been verified for Cortex-M0, Cortex-M3, Cortex-M4, RV32I, and RV32IM. Forward/inverse assembly has no divide/remainder instruction or division helper; Cortex-M0's `__aeabi_uidivmod` and RV32I's `__umodsi3` are confined to initialization. RV32I uses `__mulsi3` for V1 mixer multiplication; RV32IM has native multiply support. Physical MCU timing and hardware execution have not been measured.

## Limitations

Orbitra is not cryptographic, encryption, a secure PRP, random, a PRNG, or secure obfuscation. It offers no fault-coverage guarantee and is not a RAM test algorithm, wear leveling, or a flash endurance guarantee. The seed is deterministic input, not entropy or a key. Plain modular rotation is cheaper when simple full-cycle traversal is enough.

## Versioning

Orbitra library version `0.1.0` is independent of BRC algorithm version `V1`. The library can evolve without changing V1 mappings. Any incompatible mapping change requires a separately specified BRC-V2 algorithm and vectors.

## License

Orbitra is licensed under the Apache License, Version 2.0. See [`LICENSE`](LICENSE).
