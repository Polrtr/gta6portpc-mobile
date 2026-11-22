# Architecture notes

These notes describe the upstream Leonida checkout. Paths below are relative to the root of **ItsNotRexed/Leonida**, not this documentation-only repository.

## Main areas

| Path | Responsibility |
|---|---|
| `runtime/src/` | Runtime entry point, PE loading and lower-level runtime support. |
| `runtime/shims/` | Host-side implementations and compatibility shims for APIs expected by the target environment. |
| `runtime/test/` | Runtime-focused test programs and fixtures. |
| `runtime/CMakeLists.txt` | CMake project for the Windows runtime and its tests. |
| `third_party/` | Third-party dependencies used by the upstream project. |
| `tools/`, `scripts/` | Supporting tools and build/test helpers. |
| `make_game.py`, `game/` | Upstream game-package checking and build pipeline components; not part of this repository's documentation files. |
| `docs/` | Upstream technical notes for individual systems. |

## Runtime flow (high level)

1. The command-line program selects a mode and validates its arguments.
2. The PE loader maps a Windows executable image and resolves imports.
3. Compatibility shims provide selected host implementations for APIs the program expects.
4. The graphics path translates or forwards graphics work through the runtime's graphics backend.
5. Tests exercise individual components using synthetic programs and fixtures where possible.

This is a conceptual map, not a promise that every API or graphics operation is implemented.

## Files involved in the currently tracked investigations

- `runtime/src/vulkan_replay.h`: investigated validation and preparation paths for texture/resource copies, including `prepare_texture_copy()` and `prepare_resource_copy()`.
- `runtime/shims/d3d12_vulkan_device.cpp`: D3D12/Vulkan device and format mapping code inspected during the copy-format investigation.
- `runtime/test/`: location of runtime test sources, including the format-copy and Vulkan-device test work discussed in the progress log.

The two unresolved issues are described in [Testing](TESTING.md). No patch is claimed to have fixed them.

## Source attribution

Leonida's runtime and original architecture belong to the upstream authors and contributors. This page is an independently written navigation aid; it does not replace upstream documentation or contribution rules.
