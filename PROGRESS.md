# Leonida / GTA6 PC Port — Progress

**Last updated:** 2026-11-22  
**Status:** Full configured Release build completed; all 28 enabled CTest tests pass. Real game launch is not verified.

## Verified progress

### Windows build
- Configured the runtime with CMake using Visual Studio 2022 (x64).
- Detected MSVC 19.44 and Windows SDK 10.0.26100.0.
- The SDK feature check for D3D12 root signature version 1.2 passed.
- Built the `gta6rt` Release target successfully.
- Produced `gta6rt.exe` and `gta6rt_tls_host.dll`.
- SDL3 was disabled for this build.
- Vulkan headers were not available, so optional Vulkan-dependent test/tools were not built.

### Loader smoke test
A minimal Windows x64 test executable was compiled with MSVC:

```c
int main(void) { return 0; }
```

Observed results:

- `gta6rt.exe --dry-run leonida_test.exe` mapped one module.
- The report showed zero unimplemented imports.
- `gta6rt.exe --run leonida_test.exe` initialised one module, ran its entry point, and exited with code `0`.

This verifies a basic PE loading/initialisation/execution path for a trivial test program. It does **not** prove compatibility with the actual game.

## Xbox package investigation

The inspected extracted folder contains game resource archives (`.rpf`), many `.ttpf` resource files, and a `gameos.xvd` file.

- `MicrosoftGame.Config` and `appxmanifest.xml` declare the executable name `game_scarlett_master.exe` for the Scarlett device family.
- That declared executable was not found in the inspected folder listing.
- No `.xvc` or `.msixvc` package was found in the inspected game folder.
- The `gameos.xvd` file is 348,069,888 bytes.
- Its header contains the ASCII signature `msft-xvd` at offset `0x200`.

The XVD metadata values observed in an initial exploratory read have not yet been independently validated against a complete format specification. No attempt was made to bypass package encryption or access controls.

## Current blockers / next steps

1. Locate a legitimate, complete game package and the executable image declared by its manifest.
2. Verify the exact image/container format and whether it is accessible to the runtime.
3. Continue implementing and testing missing Xbox/Windows APIs and graphics functionality in Leonida.
4. Re-test only with authorised files and clearly distinguish smoke-test success from actual game compatibility.

## Important limitation

**GTA VI has not been launched.** The current success is limited to compiling Leonida and running a trivial test executable through its loader. The real game's executable has not been located or loaded, and graphics/gameplay compatibility remains untested.

No game binaries, proprietary resource archives, or decrypted game content are included in this repository update.


## Full test suite — 2026-11-22

- Built the full configured Release solution successfully with:
  `cmake --build build --config Release --parallel 4`.
- This generated the runtime, test executables, and test fixtures, including `tiny_game.exe`.
- Ran the complete CTest suite with:
  `ctest --test-dir build -C Release --output-on-failure`.
- **Result: 100% tests passed — 28 of 28 enabled tests passed.**
- Total CTest time: 42.89 seconds.
- The `gameinput` test (test 22 of 29) is disabled in this configuration and was not run; it is not a failure.
- The passing tests cover the configured loader/PE loader, TLS, runtime types, unimplemented-import policy, memory shims, D3D12-related structures/proxies, Bink audio/decode, FSR dispatch, BVH, Xbox Game Runtime, tiny-game execution, and command-line validation tests.

This is strong evidence that the currently enabled Leonida unit/integration tests pass on this Windows build. It still does **not** demonstrate that GTA VI itself launches or is compatible.



## Latest local test run — 2026-11-22 (afternoon)

This section supersedes the earlier 28-test result above for the newer local build/configuration.

- Rebuilt the Release configuration successfully with CMake (`--build build --config Release --parallel 4`).
- Ran CTest against the current build: 64 tests registered, 63 enabled, 1 disabled.
- **Result: 61/63 enabled tests passed; 2 failed.**
- Failure 1: `d3d12_vulkan_device` — D3D12_OPTIONS1 wave-lane capability mismatch. Vulkan reports a lane range of 8–32 while the native Intel D3D12 comparison reports 16–16; related capability fields also differ.
- Failure 2: `d3d12_copy_xfmt` — several depth/color copy cases are rejected or mismatch, including D32_FLOAT ↔ R32_FLOAT and D16_UNORM ↔ R16_UNORM. One BC1/compressed `CopyTextureRegion` case also needs separate investigation.
- Source investigation narrowed the depth/color copy rejection to validation paths in `runtime/src/vulkan_replay.h`, including aspect-mask checks in `prepare_texture_copy()` and `prepare_resource_copy()`. No fix has been applied yet.
- A small test executable still passes the loader smoke test, but this is not a GTA VI launch test.

**Current status:** the project builds, but the newest configured test run is not fully green. The two failures above remain open. GTA VI has not been launched; real-game compatibility is unverified.

