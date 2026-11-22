# HORIZON architecture

HORIZON is designed as an intermediate compatibility layer between an authorised target application and the Windows host. Its current implementation is an early foundation, not a complete runtime.

## Intended end-to-end flow

```text
Authorised local title files
          |
          v
Input manifest and validation
          |
          v
HORIZON launcher / builder
          |
          v
Target executable loader (not implemented)
          |
          v
Guest ABI and API adapters (not implemented)
          |
          v
HORIZON call bridge (generic dispatcher implemented)
       /       |       \
   graphics  system   audio/input/storage
       \       |       /
          v
      Windows host APIs
          |
          v
      Application output
```

Requests should be translated at explicit, documented boundaries. The host implementation returns results through the bridge to the guest-facing adapter. Unsupported calls must return an explicit error; they must never silently pretend to succeed.

## Current code

| Component | Responsibility | Status |
|---|---|---|
| PE inspector | Read standard PE header metadata without execution | Implemented |
| PE layout | Read optional-header fields, data directories, section table, RVA→file-offset translation and imported DLL names, read-only | Implemented (metadata only; no mapping or relocation) |
| Call bridge | Route HORIZON-defined service requests to registered handlers | Implemented as a generic in-process dispatcher |
| Input manifest | Validate the exact local file set required for a selected title | Not implemented |
| Launcher/builder | Validate inputs and start a configured runtime | Not implemented |
| Guest executable loader | Load and manage an authorised target image | Not implemented |
| Guest ABI adapters | Translate documented target calls and data structures | Not implemented |
| Runtime services | Provide host equivalents for documented service contracts | Not implemented |
| Graphics adapter | Translate target graphics semantics to a host backend | Not implemented |
| Audio/input/storage/network | Implement individually documented host service adapters | Not implemented |

## PE layout contract

`read_pe_layout` builds on `inspect_pe` and adds image base, entry-point RVA, alignments, data directories and the section table. `rva_to_file_offset` translates an RVA range only when every byte is backed by header or section raw data in the supplied buffer; it never maps, zero-fills or relocates anything. `list_import_dlls` returns DLL names only (no function names or thunks), bounded to 4096 descriptors and 256-byte names, and fails closed on any out-of-range pointer, missing terminator or non-printable name.

This module is a prerequisite for a future loader and for the Phase 2 interface inventory, but it is not a loader: nothing is mapped or executed. Imported DLL names are metadata and say nothing about whether the corresponding interfaces are implemented by HORIZON.

## Call bridge contract

The generic dispatcher uses HORIZON-owned service IDs. They are internal routing identifiers, not Xbox API ordinals, function numbers or ABI values. A handler is registered explicitly during initialisation. Unknown service IDs, invalid requests and oversized payloads fail with explicit statuses.

The dispatcher currently runs in-process and has no guest-code entry point. A later guest ABI adapter must validate and translate target structures before creating a HORIZON request. Exact calling conventions and data layouts must come from authoritative documentation and tests.

## Game input requirements

A title-specific build requires an authorised executable and whatever companion files are established by a verified input manifest. The exact game filenames and package layout are not guessed. Private files belong in a local ignored directory, not in Git.

Even with those files, the current code cannot execute the target title. The loader, target services and graphics path remain unimplemented.

## Design rules

- Keep file-format parsing separate from execution.
- Validate offsets, lengths, integer conversions and payload sizes.
- Use explicit types for platform identifiers and status values.
- Do not infer API behaviour from names alone.
- Add a synthetic test before adding a compatibility implementation.
- Fail closed on unknown formats or unsupported operations.
- Keep host OS integration behind narrow interfaces.
- Record the exact specification/SDK version for every platform contract.

## PE subsystem note

Microsoft's PE/COFF documentation defines `IMAGE_SUBSYSTEM_XBOX` as value 14. HORIZON reports that field when it appears in a valid PE header. It is only a metadata signal; it does not identify a specific console generation, validate a package or establish that Windows can execute the image.

Reference: [Microsoft PE format — Windows subsystems](https://learn.microsoft.com/windows/win32/debug/pe-format).
