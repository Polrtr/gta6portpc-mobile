# Build HORIZON on Windows

## Requirements

- Windows 10/11 x64.
- Visual Studio 2022 with the **Desktop development with C++** workload.
- Windows SDK installed through Visual Studio Installer.
- CMake 3.20 or newer.

## Build from Command Prompt

Open CMD in the repository root:

```cmd
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --parallel 4
ctest --test-dir build -C Release --output-on-failure
```

If you already configured `build` with a different generator, choose a fresh directory such as `build-vs2022`. Do not mix generators in one CMake build directory.

## Run the inspector

```cmd
build\Release\horizon-inspect.exe path\to\sample.exe
```

The current inspector is read-only and does not execute the inspected file.

## Common issues

### CMake is not recognised

Install CMake and ensure its `bin` directory is on PATH, or call cmake.exe with its full path.

### No C++ compiler is found

Open Visual Studio Installer and add **Desktop development with C++** and a Windows SDK.

### Tests fail

Keep the complete CTest output. A compilation success and a test success are different results. Report the exact commit, generator and compiler version when opening an issue.

## Scope reminder

This build produces the HORIZON inspector and synthetic tests. It does not produce an Xbox emulator, a GTA VI executable or a playable PC port.
