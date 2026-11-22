# Project HORIZON

**An experimental Xbox-to-Windows compatibility runtime**

HORIZON is an independent research project exploring the engineering needed to run authorised Xbox-targeted software in a Windows host environment. The long-term research areas are executable metadata, documented platform interfaces, host-service compatibility and graphics API adaptation.

> **Current status: bootstrap.** HORIZON currently contains a read-only PE metadata inspector and synthetic tests. It is not an Xbox emulator, does not load Xbox games, does not translate D3D12 commands and has not launched GTA VI.

This project is not affiliated with Microsoft, Xbox, Rockstar Games or Take-Two Interactive.

## What exists today

- A portable C++20 command-line tool, `horizon-inspect`.
- Bounds-checked inspection of standard PE headers.
- Reports machine type, PE32/PE32+ magic, subsystem value and section count.
- Recognises the documented PE Xbox subsystem marker (value 14), without treating that marker as proof that a binary is runnable.
- Synthetic unit tests and Windows CI.

## What does not exist yet

- Xbox package/container support.
- A package decryptor, signature bypass or security circumvention.
- Xbox system-call or GDK compatibility.
- A Windows loader for Xbox-targeted executables.
- D3D12-to-Vulkan translation.
- Audio, input, storage, networking or other console-service translation.
- GTA VI support, boot, rendering or gameplay.

A metadata match is only a clue. It does not make a console executable compatible with Windows.

## Build on Windows

Requirements: Windows 10/11 x64, Visual Studio 2022 with the Desktop development with C++ workload, and CMake 3.20 or newer.

Open **Command Prompt** in the repository directory:

```cmd
cmake -S . -B build
cmake --build build --config Release --parallel 4
ctest --test-dir build -C Release --output-on-failure
```

Run the read-only inspector:

```cmd
build\Release\horizon-inspect.exe path\to\your\authorised-sample.exe
```

For a single-configuration generator, the executable may instead be at `build/horizon-inspect.exe`.

## Project layout

| Path | Purpose |
|---|---|
| `include/horizon/` | Public interfaces for HORIZON modules |
| `src/` | Current implementation; presently the PE metadata inspector |
| `tests/` | Synthetic tests; no proprietary game data |
| `docs/ARCHITECTURE.md` | Proposed runtime architecture and current boundaries |
| `docs/ROADMAP.md` | Phased implementation plan |
| `docs/PLATFORM_SCOPE.md` | Xbox/Windows research scope and terminology |
| `.github/workflows/ci.yml` | Automated build and test checks |

Future directories such as `platform/`, `loader/`, `graphics/` and `services/` will be added only when a real, testable module exists.

## Development principles

1. Implement one small, independently testable component at a time.
2. Prefer official documentation and synthetic test cases.
3. Treat unknown platform behaviour as unknown; do not guess ABI, vtable layouts or calling conventions.
4. Keep executable inspection read-only until a lawful, documented runtime path exists.
5. Record the exact commit, toolchain, host and test output for each result.
6. Never present a parser test as proof that a game boots or runs.

## Legal and data-handling boundaries

- Use only software and files you are legally authorised to examine.
- Do not bypass encryption, DRM, signature checks, licensing, account checks or access controls.
- Do not commit or distribute game executables, packages, keys, decrypted files, proprietary assets, extracted textures/models/audio/video, or generated data derived from those assets.
- Keep private test files outside this repository. Publish only synthetic fixtures and safe, minimal diagnostic results.
- Respect the licences and notices of all third-party dependencies.

See [Safety and scope](docs/SAFETY.md), [Getting started](docs/GETTING_STARTED.md), [Architecture](docs/ARCHITECTURE.md) and [Roadmap](docs/ROADMAP.md).
