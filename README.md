# Project HORIZON

**An experimental Xbox-to-Windows compatibility runtime**

HORIZON is an independent research project exploring how authorised Xbox-targeted software might communicate with a Windows host through a separately implemented compatibility runtime.

> **IMPORTANT — A title-specific build requires the authorised Xbox executable and its required companion files. Without those inputs, HORIZON cannot prepare or test that title. The files must remain local and must never be uploaded to this GitHub repository.**
>
> **Also important: the current HORIZON prototype is not yet able to execute an Xbox game, even when an executable is supplied.** The executable loader, Xbox service adapters and graphics translation are not implemented. The current code is a PE metadata inspector and a generic internal call dispatcher; neither is a game runtime.

This project is not affiliated with Microsoft, Xbox, Rockstar Games or Take-Two Interactive.

## Intended architecture

```text
Authorised local game files
          |
          v
Input validation and manifest
          |
          v
HORIZON launcher / builder
          |
          v
Xbox-targeted executable loader
          |
          v
Guest API and call translation
          |
          v
HORIZON call bridge
     /      |       \
 graphics  system   audio/input/storage
     \      |       /
          v
    Windows host APIs
          |
          v
    Application window
```

The aim is for HORIZON to sit between the target program and Windows: it should route target requests to explicit host-side implementations and return compatible responses. Every interface must be mapped and tested individually. Xbox and Windows APIs are not automatically interchangeable.

## What exists today

- A portable C++20 command-line tool, `horizon-inspect`, for read-only PE metadata inspection.
- Bounds-checked reporting of machine type, PE32/PE32+ magic, subsystem value and section count.
- A read-only PE layout parser (`--layout`): optional-header fields, data directories, sections and imported DLL names. It distinguishes the certificate table's file offset from RVA-based directories, rejects ambiguous RVA mappings and reports suspicious section overlap. Nothing is mapped or executed.
- A bounded internal call dispatcher with request/response payload limits and capped diagnostic text.
- A generic internal call dispatcher with service registration, request validation, payload limits and explicit unsupported-service errors.
- Synthetic unit tests for the PE inspector and call dispatcher.
- Windows GitHub Actions build and test workflow.

The call dispatcher is only an internal HORIZON primitive. It is not connected to an Xbox executable or Xbox ABI.

## What does not exist yet

- Xbox package/container handling.
- Xbox executable loading, guest ABI adaptation or process lifecycle.
- Xbox OS/GDK service implementations.
- D3D12 command translation or a Vulkan backend.
- Audio, input, storage, networking or other console-service adapters.
- A graphical builder/launcher that creates a working game application.
- GTA VI support, boot, rendering or gameplay.

A PE Xbox subsystem marker (value 14) is only metadata. It does not establish that an image is complete, trusted or executable on Windows.

## Required game inputs — local only

For a future title-specific build, the user must provide the exact authorised executable and any companion files identified by the input manifest. Until the real package layout and required filenames have been verified, HORIZON deliberately does **not** invent fixed filenames.

When preparing a lawful local test, use the ignored `private-input/` folder in your own checkout. Keep all game files and generated title output outside Git tracking. Do not commit, upload or distribute commercial game executables, packages, keys or proprietary assets.

See [Game input requirements](docs/GAME_INPUTS.md).

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
build\Release\horizon-inspect.exe --layout path\to\your\authorised-sample.exe
```

## Project layout

| Path | Purpose |
|---|---|
| `include/horizon/` | Public interfaces for HORIZON modules |
| `src/` | PE inspection, PE layout parsing and internal call dispatch |
| `tests/` | Synthetic tests; no proprietary game data |
| `docs/ARCHITECTURE.md` | Runtime layers and current boundaries |
| `docs/GAME_INPUTS.md` | Required local inputs and file-handling rules |
| `docs/ROADMAP.md` | Phased implementation plan |
| `docs/SAFETY.md` | Safety and data-handling boundaries |
| `.github/workflows/ci.yml` | Automated Windows build and test checks |

## Development principles

1. Implement one small, independently testable component at a time.
2. Prefer official documentation and synthetic test cases.
3. Treat unknown platform behaviour as unknown; do not guess ABI, vtable layouts or calling conventions.
4. Keep executable inspection read-only until a lawful, documented runtime path exists.
5. Record the exact commit, toolchain, host and test output for each result.
6. Never present a parser or dispatcher test as proof that a game boots or runs.

## Legal and data-handling boundaries

- Use only software and files you are legally authorised to examine.
- Do not bypass encryption, DRM, signature checks, licensing, account checks or access controls.
- Do not commit or distribute game executables, packages, keys, decrypted files, proprietary assets, extracted textures/models/audio/video, or generated data derived from those assets.
- Keep private test files outside this repository. Publish only synthetic fixtures and safe, minimal diagnostic results.
- Respect the licences and notices of all third-party dependencies.

See [Safety and scope](docs/SAFETY.md), [Getting started](docs/GETTING_STARTED.md), [Architecture](docs/ARCHITECTURE.md) and [Roadmap](docs/ROADMAP.md).
