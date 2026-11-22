# HORIZON progress

**Project stage:** Phase 0 — repository and inspector bootstrap  
**Last updated:** 2026-11-22

## Verified repository state

- [x] Created an isolated branch, `horizon/bootstrap`, for the HORIZON transition.
- [x] Preserved the previous workbench history on `main`.
- [x] Added the initial C++20 project structure.
- [x] Added a read-only PE metadata inspector.
- [x] Added synthetic parser tests.
- [x] Added Windows CI configuration.

## Current implementation boundary

The first module reads only the start of a file and inspects PE metadata. It does not execute the file, map it into memory, resolve imports, unpack console packages, decrypt content or access Xbox services.

## Not implemented

- Xbox package/container parser.
- Xbox OS/GDK API compatibility.
- Executable loading and process emulation.
- D3D12 command translation or Vulkan backend.
- GPU presentation, audio, input, storage, networking or game support.

## Next tasks

1. [ ] Run the first CI build and fix any compiler/test failures.
2. [ ] Add more malformed-input tests for the PE inspector.
3. [ ] Document the supported PE fields and known limitations.
4. [ ] Research official, publicly documented Xbox/Windows interfaces.
5. [ ] Design a synthetic host-service interface before implementing any compatibility shim.
6. [ ] Create a minimal graphics test application that contains no game code or assets.

## Reporting rule

Only mark a task complete after code and a reproducible test support it. A successful parser test is not a runtime boot. A successful graphics sample is not GTA VI compatibility.
