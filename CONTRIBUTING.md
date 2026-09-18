# Contributing

- BRC-V1 mappings are frozen. Changes to the mixer, tags, seed expansion, delta selection, Q construction, or mapping semantics require BRC-V2.
- Keep production code portable C11, heap-free, warning-clean, and free of mutable global state.
- Add tests for new behavior and run the full CTest suite, including the exhaustive validation and installed consumer.
- Never silently regenerate or edit `tests/vectors_v1.csv`. A future algorithm version needs its own vectors.
- Do not make cryptographic or security claims for Orbitra. A cryptographic design would need a separate specification and independent review.
