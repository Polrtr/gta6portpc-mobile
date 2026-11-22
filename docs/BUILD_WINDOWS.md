# Building the Windows runtime

These instructions build the **Leonida runtime and its tests**, not a ready-to-play copy of GTA VI.

## Requirements

- Windows 10/11 x64.
- Visual Studio 2022 with **Desktop development with C++**.
- Windows SDK 10.0.26100 or newer.
- CMake 3.24 or newer.
- Git; clone the upstream repository recursively so its submodules are present.

The upstream runtime uses C++20 and stops configuration if the build is not 64-bit or the required D3D12 root-signature 1.2 declarations are missing.

## 1. Clone upstream

Open a terminal:

```cmd
git clone --recursive https://github.com/ItsNotRexed/Leonida.git
cd Leonida
git submodule update --init --recursive
```

## 2. Configure for Visual Studio 2022 x64

From the Leonida repository root:

```cmd
cmake -S runtime -B build -G "Visual Studio 17 2022" -A x64
```

If you already have a build folder configured with another generator or SDK, use a new folder name (for example `build-vs2022`) rather than mixing configurations.

## 3. Compile Release

```cmd
cmake --build build --config Release --parallel 4
```

The recorded local build produced `gta6rt.exe` and `gta6rt_tls_host.dll` under the Release output directory. Exact outputs depend on the source revision and enabled optional dependencies.

## 4. Run the test suite

```cmd
ctest --test-dir build -C Release --output-on-failure
```

CTest returns a non-zero exit code when tests fail. Read the names and failure messages; do not treat a successful compile as a passing test suite.

## 5. Common configuration problems

### CMake is not found

Install CMake 3.24+ and ensure it is available in your terminal PATH, or call the full path to cmake.exe.

### Wrong architecture

The runtime requires x64. Configure using the Visual Studio generator with `-A x64`.

### Windows SDK is too old

Install Windows SDK 10.0.26100 or newer through Visual Studio Installer. Then delete the **build directory only** and run the configure command again. Do not delete your source checkout.

### Submodule or third-party files are missing

From the repository root, run:

```cmd
git submodule update --init --recursive
```

### Build passes, but tests fail

Build and test are separate checks. Record the exact failing test and output, then follow [Testing](TESTING.md). The failures currently tracked by this workbench are documented there.

## Important

These steps do not build or provide the game, game assets, keys or a complete playable port. Use only files you are legally authorised to access and never bypass encryption or access controls.
