# Leonida / GTA6 PC Port — Progress

**Last updated:** 2026-11-22  
**Status:** Experimental runtime build and smoke tests completed; real game launch is not verified.

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
