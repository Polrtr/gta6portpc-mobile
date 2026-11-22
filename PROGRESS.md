# HORIZON progress

**Project stage:** Phase 1 — PE metadata and internal call-bridge foundation  
**Last updated:** 2026-11-22

## Verified implementation

- [x] Independent C++20/CMake project.
- [x] Read-only PE metadata inspector.
- [x] Generic in-process call dispatcher.
- [x] HORIZON-owned service identifiers for runtime, graphics, audio, input, storage and network.
- [x] Request validation and a 16 MiB payload limit.
- [x] Explicit errors for unsupported services and handler failures.
- [x] Synthetic tests for the PE inspector and call dispatcher.
- [x] Windows CI build and test workflow.
- [x] Read-only PE layout parser: optional-header fields, data directories, section table, bounds-checked unique RVA translation and imported DLL names (`--layout` in `horizon-inspect`). Synthetic tests cover PE32/PE32+, import-directory bounds, file-offset semantics for the security directory, ambiguous mappings and section overlap.
- [x] Call bridge bounds request/response payloads to 16 MiB and diagnostic text to 4 KiB.

## Critical game-input status

A title-specific test requires the exact authorised Xbox executable and its required companion files. No such game files are stored in this repository. Keep authorised private inputs in the ignored local `private-input/` folder, never in Git.

Supplying an executable alone will **not** make the current prototype run the game. HORIZON still lacks the Xbox executable loader, guest ABI adaptation, Xbox service implementations and graphics translation. The exact input filenames must be established from an authorised, verified package; do not guess or rename files.

## Current implementation boundary

The call bridge is an internal routing primitive. It does not yet receive real Xbox ABI calls, load guest code, implement Xbox system APIs or translate graphics commands. The service IDs are HORIZON-owned identifiers, not Xbox ABI values.

## Not implemented

- Xbox package/container handling.
- Xbox OS/GDK API compatibility.
- Executable loading and guest process emulation.
- D3D12 command translation or Vulkan backend.
- GPU presentation, audio, input, storage and networking adapters.
- Graphical builder/launcher and game support.

## Next tasks

1. [ ] Run and record CI for the call-bridge increment.
2. [ ] Extend malformed-input coverage for the base PE inspector and add fuzzing (layout parser has 14 synthetic tests; the base inspector still has 5).
3. [ ] Specify a local-only input manifest with verified filenames, formats and required companions.
4. [ ] Research official, publicly documented Xbox/Windows interfaces.
5. [ ] Define the guest ABI boundary from authoritative documentation.
6. [ ] Build a self-written synthetic application that exercises a HORIZON service end to end.
7. [ ] Create an original D3D12 graphics sample and investigate a host backend.

## Verification status of the PE layout increment

- Built and tested on Linux (GCC 13.3, CMake): 3/3 CTest targets pass; the layout tests also pass under AddressSanitizer and UBSan.
- Cross-compiled for Windows x64 with MinGW-w64 (no warnings), but **not executed** on Windows and **not built with MSVC**. Windows CI results for this branch are still to be recorded.

## Reporting rule

Only mark a task complete after code and a reproducible test support it. A parser or dispatcher test is not a runtime boot. A synthetic graphics test is not proof of GTA VI compatibility.
