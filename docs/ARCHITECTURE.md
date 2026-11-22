# HORIZON architecture

HORIZON is intended to grow into a compatibility runtime. The current code is deliberately much smaller: a read-only PE metadata inspector.

## Current data flow

```text
authorised sample file
        |
        v
horizon-inspect
        |
        v
bounded header read
        |
        v
PE / COFF metadata parser
        |
        v
human-readable report
```

No image is mapped into memory as an executable, no imports are resolved, and no target code runs.

## Planned runtime layers

| Layer | Responsibility | Status |
|---|---|---|
| File identification | Identify documented file/header formats | Initial PE header support only |
| Package boundary | Recognise authorised package metadata | Not implemented |
| Executable loader | Memory layout, relocations, imports and lifecycle | Not implemented |
| Platform services | Documented Xbox-facing OS/GDK contracts with host equivalents | Not implemented |
| Graphics | D3D12 semantics and a host graphics backend | Not implemented |
| Other services | Input, audio, storage, networking, timing and synchronisation | Not implemented |
| Test harness | Synthetic test programs and repeatable result capture | Parser tests only |

## Design rules

- Keep format parsing separate from execution.
- Validate every offset and length before reading.
- Use explicit types for platform identifiers and status values.
- Do not infer API behaviour from names alone.
- Add a synthetic test before adding a compatibility implementation.
- Fail closed on unknown formats or unsupported operations.
- Keep host OS integration behind narrow interfaces.
- Record the exact specification/SDK version for every platform contract.

## PE subsystem note

Microsoft's PE/COFF documentation defines `IMAGE_SUBSYSTEM_XBOX` as value 14. HORIZON reports that field when it appears in a valid PE header. It is only a metadata signal; it does not identify a specific console generation, validate a package, or establish that Windows can execute the image.

Reference: [Microsoft PE format — Windows subsystems](https://learn.microsoft.com/windows/win32/debug/pe-format).

## Future module boundaries

As real code is added, the intended separation is:

- `include/horizon/`: public module contracts.
- `src/format/`: file-format inspection.
- `src/loader/`: executable image loading, only after a lawful and documented design exists.
- `src/platform/`: host services and platform compatibility interfaces.
- `src/graphics/`: graphics command/state translation.
- `tests/`: synthetic fixtures and unit tests.

These are design targets, not current capabilities.
