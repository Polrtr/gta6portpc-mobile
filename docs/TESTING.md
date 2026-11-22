# Testing and reporting results

## Run the configured suite

From the root of the upstream Leonida checkout:

```cmd
cmake --build build --config Release --parallel 4
ctest --test-dir build -C Release --output-on-failure
```

The first command compiles the selected Release targets. The second runs the tests registered in that build directory. Use a clean build directory if you change generator, architecture or major toolchain settings.

## Latest recorded baseline

Date: **2026-11-22 (afternoon)**

| Measure | Result |
|---|---:|
| Tests registered | 64 |
| Enabled | 63 |
| Disabled | 1 |
| Passed | 61 |
| Failed | 2 |

The current local test record is not fully green. A previous 28/28 result is historical and was superseded by this later run. Different source revisions and CMake configurations may register different tests.

## Known failures

### 1. `d3d12_vulkan_device`

The D3D12_OPTIONS1 wave-lane capability comparison differs: Vulkan reports 8–32, while the native Intel D3D12 comparison reports 16–16. Related capability fields also differ.

**Next investigation:** verify the intended D3D12 semantics and Vulkan subgroup capabilities, then add focused tests for the mapping. Do not hardcode a device-specific value without a documented compatibility rule.

### 2. `d3d12_copy_xfmt`

Several depth/color copy cases are rejected or mismatch, including D32_FLOAT ↔ R32_FLOAT and D16_UNORM ↔ R16_UNORM. A BC1/compressed CopyTextureRegion case also needs separate analysis.

The investigation has narrowed the depth/color rejection to aspect-mask validation paths in `runtime/src/vulkan_replay.h`, including `prepare_texture_copy()` and `prepare_resource_copy()`. No fix has been applied.

**Next investigation:** separate depth/color reinterpretation from compressed-format compatibility; add minimal tests for each supported case; verify the resource aspects and copy regions before changing validation.

## Loader smoke test

The recorded minimal executable test showed one module mapped, zero unimplemented imports in the dry run, and exit code 0 when the trivial program was run. This checks only a narrow PE loader path. It is not a game launch.

## What to include in a bug report

1. Exact test name and full command.
2. Full relevant CTest output, including the failure message.
3. Leonida commit SHA (`git rev-parse HEAD`).
4. Windows version, Visual Studio/MSVC version, CMake version and Windows SDK.
5. GPU model and driver version for graphics-related failures.
6. Whether the build folder was fresh or reused.
7. What changed since the last passing run.

## Data handling

Do not upload game files, proprietary archives, executable images, keys, extracted textures/models/audio, shader outputs derived from game content, saves or private logs containing such data. A small synthetic test or a minimal source-level reproduction is preferable.
