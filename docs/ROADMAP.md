# Roadmap

This roadmap tracks the workbench's research priorities. It is not an upstream Leonida schedule, and it does not promise that GTA VI will become playable.

## Phase 1 — Reproducible baseline

- [x] Record the local Windows x64 Release build process.
- [x] Document the CTest command and latest result.
- [x] Separate the documentation repository from the upstream source repository.
- [x] Define a mandatory source-SHA capture process for every future test run (historical 2026-11-22 run has no recoverable SHA; documented explicitly).
- [x] Add an environment block to the latest progress entry and a reusable test-run template for future entries.

## Phase 2 — Resolve the two recorded test failures

- [ ] Investigate the D3D12_OPTIONS1 wave-lane capability mismatch in `d3d12_vulkan_device`.
- [ ] Add focused coverage for wave-lane capability reporting.
- [ ] Investigate D32_FLOAT/R32_FLOAT and D16_UNORM/R16_UNORM copy behavior in `d3d12_copy_xfmt`.
- [ ] Investigate the BC1 CopyTextureRegion case independently.
- [ ] Re-run the full enabled suite after each fix and publish the exact before/after results.

## Phase 3 — Improve graphics test coverage

- [ ] Add minimal synthetic tests for each supported copy/resolve path.
- [ ] Test across more than one GPU vendor where hardware is available.
- [ ] Record driver versions and distinguish hardware failures from validation/test setup errors.
- [ ] Keep unsupported operations explicitly documented rather than silently treating them as successful.

## Phase 4 — End-to-end compatibility research

- [ ] Follow upstream implementation and status updates.
- [ ] Only test with files the tester is legally authorised to access.
- [ ] Record whether a test reaches loader startup, graphics initialisation, first frame or actual gameplay; these are different milestones.
- [ ] Never claim that the game boots or is playable without a reproducible real-game test.

## Out of scope for now

- Distributing the game, game files, keys or proprietary assets.
- Bypassing encryption, DRM or access controls.
- Claiming Android/iOS support; the recorded work is Windows x64.
- Presenting a unit-test pass as proof of real-game compatibility.

## Phase 5 — Upstream feature gaps

The detailed, date-stamped list is maintained in [UPSTREAM_GAPS.md](UPSTREAM_GAPS.md). These are upstream-reported gaps, not features already implemented in this workbench.

- [ ] Verify upstream HEAD and identify whether buffer views, root signatures, PSOs, draw commands, placed resources, 1D textures and the game's presentation path remain open.
- [ ] Design focused synthetic tests before attempting D3D12/Vulkan implementation work.
- [ ] Investigate the remaining VS/PS/GS wave64 cross-half operations on 32-wide subgroups.
- [ ] Investigate the unverified vertex-shader input-layout assumption.
- [ ] Finish model-layout coverage and BCPack-to-rpf9dump integration only after checking upstream status.
- [ ] Record exact source commits and hardware/driver details for every result.
- [ ] Submit useful, narrowly scoped fixes upstream when verified.
