# Getting started

This page explains how to reproduce the runtime work recorded in this repository.

## 1. Know which repository to clone

This repository, `Polrtr/gta6portpc-mobile`, contains documentation and progress notes. It does not contain the Leonida runtime source.

For the actual source code, use the upstream project:

```cmd
git clone --recursive https://github.com/ItsNotRexed/Leonida.git
cd Leonida
git submodule update --init --recursive
```

Keep the upstream checkout separate from this documentation repository. Do not copy proprietary game files into either Git repository.

## 2. Install the Windows build tools

For the runtime build documented here, prepare:

- Windows 10 or 11, 64-bit.
- Visual Studio 2022 with the **Desktop development with C++** workload.
- Windows SDK **10.0.26100 or newer**.
- CMake **3.24 or newer**.
- Git, including submodule support.

The Windows SDK requirement is checked by the upstream runtime CMake project. If CMake reports that D3D12 root-signature 1.2 is unavailable, install the required SDK in Visual Studio Installer and configure into a fresh build directory.

## 3. Build and test the runtime

From the root of the upstream Leonida checkout, follow [Building on Windows](BUILD_WINDOWS.md).

The short version is:

```cmd
cmake -S runtime -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --parallel 4
ctest --test-dir build -C Release --output-on-failure
```

The test command may report failures. That does not necessarily mean the compiler build failed; inspect the names and failure output separately.

## 4. Understand what the result means

A successful CMake build means the configured targets compiled. Passing CTest cases mean those particular tests passed in that environment. A trivial PE loader smoke test only checks a limited loader path.

None of these results proves that GTA VI boots or is playable. At the time of the latest record in this repository, the test suite had two known failures and real-game launch had not been verified.

## 5. Before sharing a result

Include:
- Leonida source commit or version.
- Windows version, Visual Studio/MSVC version, CMake version and Windows SDK.
- GPU model and driver version, when graphics tests are involved.
- The exact command and complete relevant failure output.
- Whether the result is from a clean build or an existing build directory.

Never attach game archives, executable files, keys, extracted assets, shaders or saves. See [Testing](TESTING.md) and [Contributing](../CONTRIBUTING.md).
