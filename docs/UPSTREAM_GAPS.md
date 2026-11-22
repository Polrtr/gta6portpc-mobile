# Platform compatibility research checklist

This file replaces the previous Leonida upstream-gap notes. HORIZON is now its own project; it does not inherit another project's implementation status or source code.

## Research questions

- [ ] What publicly documented Xbox application lifecycle contracts are relevant?
- [ ] Which interfaces are part of the public Windows GDK, and which are console-only?
- [ ] Which contracts have documented Windows equivalents?
- [ ] What runtime, device, memory and scheduling assumptions differ between the target and host?
- [ ] Which graphics operations can be tested with original synthetic D3D12 workloads?
- [ ] What cannot be implemented or validated without authorised platform documentation and development hardware?

## Evidence required

For every proposed compatibility interface, record:

1. Official documentation URL and SDK/API version.
2. Exact structure and function contract.
3. Ownership, lifetime, threading and error semantics.
4. Host equivalent, if one exists.
5. Unsupported cases.
6. A synthetic test that demonstrates the behaviour.

Do not infer a contract from a symbol name, binary offset or guesswork.

## Current status

No Xbox system API, loader, package reader or graphics translator is implemented. The only executable-related code is a static PE-header inspector.

## External reference starting points

- [Microsoft PE format](https://learn.microsoft.com/windows/win32/debug/pe-format)
- [Microsoft Direct3D 12 documentation](https://learn.microsoft.com/windows/win32/direct3d12/direct3d-12-graphics)
- [Microsoft Game Development Kit documentation](https://learn.microsoft.com/gaming/gdk)

Always check the current official documentation and the applicable licence/SDK terms before implementation.
