# Leonida / GTA6 PC Port — Progress Log

**Last updated:** 2026-11-22  
**Current recorded state:** Windows x64 Release build succeeds; the latest CTest run passed 61 of 63 enabled tests. Real-game launch is not verified.

This log records tests performed in the local Leonida checkout used for this workbench. Results can vary with source revision, build options, Windows SDK, compiler, drivers and hardware.

## Latest local test run — 2026-11-22 (afternoon)

This is the newest recorded result and supersedes the earlier 28/28 result below.

- Rebuilt the Release configuration successfully with CMake:
  `cmake --build build --config Release --parallel 4`.
- Ran CTest against the current build:
  `ctest --test-dir build -C Release --output-on-failure`.
- **64 tests registered: 63 enabled, 1 disabled.**
- **61/63 enabled tests passed; 2 failed.**
- Failure 1 — `d3d12_vulkan_device`: D3D12_OPTIONS1 wave-lane capability mismatch. Vulkan reports a lane range of 8–32 while the native Intel D3D12 comparison reports 16–16; related capability fields also differ.
- Failure 2 — `d3d12_copy_xfmt`: several depth/color copy cases are rejected or mismatch, including D32_FLOAT ↔ R32_FLOAT and D16_UNORM ↔ R16_UNORM. A BC1/compressed CopyTextureRegion case also needs separate investigation.
- Source investigation narrowed the depth/color copy rejection to validation paths in `runtime/src/vulkan_replay.h`, including aspect-mask checks in `prepare_texture_copy()` and `prepare_resource_copy()`. No fix has been applied.
- A small test executable still passes the loader smoke test. This is not a GTA VI launch test.

### Current blockers

1. Reconcile the Vulkan and native D3D12 wave-lane capability reporting, with tests that prove the expected mapping.
2. Investigate depth/color texture-copy compatibility and the separate BC1 CopyTextureRegion case.
3. Re-run the affected tests after any change, then run the full enabled suite.
4. Keep all test results tied to the exact source revision, toolchain, GPU and driver.

## Earlier full test run — 2026-11-22

An earlier configuration reported **28 of 28 enabled CTest tests passing**, with one test disabled. That result is retained as historical evidence, but it is not the latest status. The later configuration registered 64 tests and had the two failures listed above; do not report the older result as the current suite status.

## Windows build environment recorded

- Visual Studio 2022, MSVC x64.
- MSVC 19.44.
- Windows SDK 10.0.26100.0.
- Configured and built the runtime using CMake.
- Release outputs included `gta6rt.exe` and `gta6rt_tls_host.dll`.
- SDL3 was disabled for the recorded build.
- Vulkan headers were not available in that configuration, so optional Vulkan-dependent tests/tools were not built.

The Windows runtime CMake project requires 64-bit compilation and Windows SDK 10.0.26100 or newer for its D3D12 root-signature declarations. See [the build guide](docs/BUILD_WINDOWS.md).

## Loader smoke test

A minimal Windows x64 test executable was compiled with MSVC:

```c
int main(void) { return 0; }
```

Observed results:

- `gta6rt.exe --dry-run leonida_test.exe` mapped one module.
- The report showed zero unimplemented imports.
- `gta6rt.exe --run leonida_test.exe` initialised one module, ran its entry point and exited with code `0`.

This verifies a basic PE loading/initialisation/execution path for a trivial test program only. It does **not** prove compatibility with the actual game.

## Xbox package investigation notes

The inspected extracted folder contained game resource archives (`.rpf`), many `.ttpf` resource files and a `gameos.xvd` file.

- `MicrosoftGame.Config` and `appxmanifest.xml` declare the executable name `game_scarlett_master.exe` for the Scarlett device family.
- That declared executable was not found in the inspected folder listing.
- No `.xvc` or `.msixvc` package was found in that inspected folder.
- The `gameos.xvd` file was 348,069,888 bytes.
- Its header contained the ASCII signature `msft-xvd` at offset `0x200`.

The initial exploratory XVD metadata values have not been independently validated against a complete format specification. No attempt was made to bypass package encryption or access controls.

## Real-game status

**GTA VI has not been launched.** The real game's executable has not been located or loaded by this workbench. Graphics, gameplay and real-game compatibility remain unverified. The upstream Leonida project README should be checked for its latest official status.

No game binaries, proprietary resource archives, keys or decrypted game content are included in this repository.

### Reproducibility metadata for the latest recorded run

- Upstream source commit SHA: **not captured for this historical run**. Do not infer or substitute the current upstream HEAD; the exact revision used locally cannot be reconstructed from the available log.
- OS: Windows 11 x64.
- CPU: Intel Core 3 N355 (recorded workstation/laptop context).
- GPU: Intel integrated graphics; exact device identifier and driver version were not captured in this test entry.
- Toolchain: Visual Studio 2022, MSVC 19.44, Windows SDK 10.0.26100.0.
- CMake: 4.4.4.
- Build: Release, x64; SDL3 disabled; Vulkan headers unavailable in this configuration.
- Test commands: the CMake build and CTest commands shown above.
- Future runs must capture the upstream SHA and complete hardware/driver/toolchain block before results are recorded. Use [the test-run template](docs/TEST_RUN_TEMPLATE.md).
