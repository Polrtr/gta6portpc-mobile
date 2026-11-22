# Changelog

## 2026-11-22 — PE layout parser (v0.3.0)

- Added `horizon/pe_layout.hpp`: optional-header fields, data directories, section table, bounds-checked RVA→file-offset translation and imported DLL names.
- Added `horizon-inspect --layout` (reads up to 256 MiB; read-only).
- Added `horizon-layout-tests` (14 synthetic tests, including malformed-input cases).
- CI now also runs on pushes to `claude/**` branches.
- No existing code was removed. This is metadata parsing only: nothing is mapped, relocated or executed.

## 2026-11-22 — HORIZON bootstrap

- Reframed the repository from a Leonida documentation workbench into the independent Project HORIZON prototype.
- Preserved the previous workbench history on the `main` branch; this transition is isolated in `horizon/bootstrap`.
- Added a C++20, read-only PE metadata inspector.
- Added synthetic parser tests and cross-platform CI.
- Replaced the previous roadmap and architecture notes with HORIZON-specific scope, milestones and safety boundaries.

This is an initial scaffold, not a working Xbox compatibility layer.
