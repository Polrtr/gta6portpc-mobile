# Upstream gap analysis — 22 November 2026

This document records implementation gaps visible in the public `ItsNotRexed/Leonida` README and D3D12/Vulkan plan checked on 2026-11-22. It is a research snapshot, not a claim that every item is still open at the current upstream HEAD. Before writing a patch, fetch the latest upstream commit and check the relevant source, issue and pull request.

## Important boundary

Leonida is the implementation project. `Polrtr/gta6portpc-mobile` is our independent research/workbench repository. We should add original tests, diagnostics, reproducible reports and patches here; we should not copy the upstream source tree or claim upstream features as ours.

## Gaps explicitly reported upstream

### A. Xbox D3D12 to Vulkan — high priority

The upstream status says these pieces are still missing or incomplete:

- [ ] Buffer views.
- [ ] Root signatures.
- [ ] Pipeline-state objects (PSOs) and graphics/compute pipeline creation.
- [ ] Draw commands and the remaining command execution needed to render frames.
- [ ] Placed resources.
- [ ] 1D textures and 1D texture arrays.
- [ ] The game's actual presentation path / swapchain integration. The D3D12 plan notes a window presenter exists, but the game's own DXGI or PresentX entry point is not implemented.
- [ ] Verify all operations against the actual game's call patterns, not only synthetic tests.

**First work for our repository:** create a small, isolated test matrix for each missing interface/operation, with expected D3D12 semantics, Vulkan equivalent, validation requirements and a minimal synthetic test. Do not implement guessed COM vtable slots: verify ABI and method order against the exact upstream headers and measured call sites first.

### B. Shader translation correctness — high priority

The upstream README reports all 26,503 shaders translated and SPIR-V-validated, but validation does not prove that every shader produces the correct output on every GPU.

Open areas explicitly identified upstream:

- [ ] Verify the vertex-shader input layout assumption (`v0` / `VertexIndex`) if the console uses NGG for vertex processing.
- [ ] Make wave64 cross-half operations exact for vertex, pixel and geometry shaders on GPUs with 32-wide subgroups (notably NVIDIA and Intel).
- [ ] Add output-comparison tests for translated game shaders when a lawful, reproducible test harness and expected results are available.

**Important:** compute and mesh wave64 emulation is already reported as exact; do not reimplement it. The remaining cross-half issue is specifically VS/PS/GS on 32-wide subgroups.

### C. Asset decoding — medium priority

- [ ] Finish decoding model layouts that the current exporter rejects.
- [ ] Integrate the clean-room BCPack texture decoder into the normal `rpf9dump` export path.
- [ ] Validate BC6H/BC7 cases that need local history tables without adding proprietary game data or generated assets to this repository.

Do not duplicate the already working OBJ model export, DDS texture export, or AWC audio export.

### D. Real executable / boot milestone — blocking dependency

- [ ] Establish a lawful, reproducible developer test path for the actual game executable when it is available to an authorised developer.
- [ ] Keep loader smoke tests, synthetic D3D12 tests, pipeline creation, first-frame rendering and actual gameplay as separate milestones.
- [ ] Do not claim that GTA VI boots or is playable until a real-game run is demonstrated and independently reproducible.

The upstream project states that the real executable is not present in the repository and that the game does not boot yet. This is not something our documentation alone can solve.

## Our existing local findings — historical, not yet confirmed against current upstream

The previous local Windows run recorded:

- 61/63 enabled CTest cases passing, with a wave-lane capability comparison failure and texture-copy format failures.
- A trivial stand-in executable passing a loader smoke test.

These results belong to the exact source checkout used for that run. They must not be described as current upstream defects until we record its commit SHA, sync to the current upstream revision, rebuild and rerun the tests. The upstream project may already have changed or fixed the relevant code.

## Contribution workflow

1. Record the exact upstream commit SHA and local GPU, driver, OS, compiler and Vulkan SDK versions.
2. Check the current upstream README, plan, source, open issues and pull requests for the target feature.
3. Write a minimal synthetic reproduction and expected behaviour.
4. Implement one narrowly scoped change in our research branch or a clean upstream fork.
5. Run focused tests, then the full relevant test suite; include before/after results.
6. Submit a small upstream pull request if the change is useful and complies with the upstream contribution guide.
7. Update this document only when the status is supported by a commit, test log or upstream reference.

## References

- [Leonida upstream repository](https://github.com/ItsNotRexed/Leonida)
- [Upstream README / status](https://github.com/ItsNotRexed/Leonida/blob/main/README.md)
- [Xbox D3D12 to Vulkan implementation plan](https://github.com/ItsNotRexed/Leonida/blob/main/docs/d3d12_vulkan.md)
- [Upstream contribution guide](https://github.com/ItsNotRexed/Leonida/blob/main/CONTRIBUTING.md)

This snapshot was checked on 2026-11-22. Always re-check upstream before starting implementation.
