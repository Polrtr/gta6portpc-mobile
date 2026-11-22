# HORIZON roadmap

This roadmap describes our own implementation. Percentages are not used: a milestone is complete only when code, tests and documentation support it.

## Phase 0 — Project foundation

- [x] Create an independent repository branch and project identity.
- [x] Add CMake and a C++20 build.
- [x] Add read-only PE metadata inspection.
- [x] Add synthetic tests and CI configuration.
- [ ] Verify the first Windows and Linux CI runs.

## Phase 1 — Format recognition

- [x] Expand PE metadata coverage using the official PE/COFF specification (optional-header fields, data directories, sections, import DLL names; further directories such as exports, relocations and TLS remain open).
- [ ] Add robust malformed-input and boundary tests (started for the layout parser; the base inspector still needs more cases, and fuzzing is not yet done).
- [ ] Document which public package metadata formats can be identified without unpacking protected content.
- [ ] Keep unknown or protected package data opaque; no decryption or access-control bypass.

## Phase 2 — Compatibility contracts

- [ ] Build a documented inventory of public Xbox-facing interfaces relevant to application lifecycle, timing, file access and input.
- [ ] Record version, calling convention, structure layout and source for every interface.
- [ ] Define host-side interfaces before implementing shims.
- [ ] Add self-written test programs for each contract.
- [ ] Mark unsupported calls explicitly.

## Phase 3 — Minimal runtime services

- [ ] Add a test-only host runtime with explicit startup/shutdown.
- [ ] Implement only documented, testable service behaviour.
- [ ] Add deterministic tests for handles, errors, clocks and input.
- [ ] Keep host resource access sandboxed and opt-in.

## Phase 4 — Graphics research

- [ ] Build a small original D3D12 sample that draws a triangle on Windows.
- [ ] Document the graphics semantics required by the sample.
- [ ] Prototype a separate backend adapter using public APIs and synthetic resources.
- [ ] Add image/output comparisons and GPU/driver metadata.
- [ ] Do not claim Xbox D3D12 equivalence from a Windows-only sample.

## Phase 5 — End-to-end research

- [ ] Demonstrate a complete path using HORIZON's own synthetic application.
- [ ] Publish reproducible build, test and first-frame evidence.
- [ ] Review legal authorisation, platform requirements and technical feasibility before any third-party title is considered.
- [ ] Keep actual title compatibility unclaimed until independently verified.

## Explicit non-goals

- Distributing commercial games or game files.
- Defeating encryption, DRM, signatures, activation, licensing or platform security.
- Extracting or publishing proprietary game assets.
- Claiming that HORIZON is a complete console emulator or that it can run GTA VI.
