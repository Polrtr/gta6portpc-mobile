# Getting started

This repository contains the HORIZON prototype itself. The initial milestone is a read-only PE metadata inspector.

## Requirements

- Windows 10/11 x64 or a recent Linux development environment.
- CMake 3.20 or newer.
- A C++20 compiler (Visual Studio 2022/MSVC, Clang or GCC).
- Git.

## Clone

```cmd
git clone https://github.com/Polrtr/gta6portpc-mobile.git
cd gta6portpc-mobile
```

The current transition is being developed on the `horizon/bootstrap` branch. Switch to it to test this version:

```cmd
git switch horizon/bootstrap
```

## Configure, build and test

```cmd
cmake -S . -B build
cmake --build build --config Release --parallel 4
ctest --test-dir build -C Release --output-on-failure
```

With Visual Studio's multi-configuration generator, the executable is normally `build\Release\horizon-inspect.exe`.

## Inspect an authorised PE sample

```cmd
build\Release\horizon-inspect.exe path\to\sample.exe
```

The tool reads at most the first 1 MiB and reports basic PE metadata. It does not run, modify, unpack or decrypt the input.

## What to do next

Read [Architecture](ARCHITECTURE.md), [Platform scope](PLATFORM_SCOPE.md), [Safety](SAFETY.md) and [Roadmap](ROADMAP.md). Do not use a game package as a test fixture. Use the synthetic fixtures already in `tests/`.
