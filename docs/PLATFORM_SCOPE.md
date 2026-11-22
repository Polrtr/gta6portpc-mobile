# Platform scope and terminology

## What HORIZON is researching

HORIZON is an experimental compatibility-runtime project. The intended direction is to study how documented target-platform contracts could be represented by safe host-side interfaces and tested with original programs.

A compatibility runtime is not a file converter. Changing a header or renaming a binary does not translate operating-system calls, graphics commands, memory behaviour, scheduling, or device services.

## PE subsystem marker

Microsoft's PE/COFF specification lists `IMAGE_SUBSYSTEM_XBOX` as value 14 in the optional header's Subsystem field. HORIZON can report that field if it finds a structurally valid PE image.

That marker alone does not prove:
- which Xbox generation or device family produced the image;
- that the file is a complete game executable;
- that all required package data is present;
- that its imports and runtime contracts are available on Windows;
- that the image is signed, trusted or legally runnable on the host.

Reference: [Microsoft PE format](https://learn.microsoft.com/windows/win32/debug/pe-format).

## Packages are not the same as executable images

A game package/container and the executable image declared or contained within it are different layers. HORIZON's first milestone does not parse, unpack, decrypt or validate any commercial package format.

Any future package-format work must rely on official public specifications or explicit authorisation and must not bypass encryption, signature checks, DRM or access controls.

## Windows host vs console runtime

Even when both environments use x64 processors and related graphics concepts, a console-targeted program may rely on platform-specific APIs, libraries, services, memory rules and hardware behaviour. Those dependencies must be identified and implemented individually; they cannot be assumed to match desktop Windows.

## HORIZON's current boundary

Implemented: read-only PE header inspection and synthetic tests.

Not implemented: package handling, Xbox API compatibility, executable loading, process emulation, graphics translation, audio, input, storage, networking or game support.
