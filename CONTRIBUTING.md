# Contributing to HORIZON

Thanks for helping with HORIZON. The project is an early compatibility-runtime research prototype.

## Before opening a change

- Read [Safety and scope](docs/SAFETY.md).
- Read [Architecture](docs/ARCHITECTURE.md) and [Roadmap](docs/ROADMAP.md).
- Search existing issues before starting duplicate work.
- Keep each pull request focused on one testable change.

## Good first contributions

- Add synthetic PE parser tests.
- Improve bounds checking and error messages.
- Document a field using an official specification.
- Add a reproducible Windows/Linux CI test.
- Prototype a host-service interface using a self-written test application.

## Development workflow

```cmd
git switch -c feature/short-description
cmake -S . -B build
cmake --build build --config Release --parallel 4
ctest --test-dir build -C Release --output-on-failure
```

Before submitting, include the exact commands and results. Do not claim a build or test passed unless it was actually run.

## Required evidence for platform work

- Link the public specification or SDK documentation.
- Identify the exact interface, version and host behaviour.
- Include a synthetic test that does not require proprietary game files.
- Explain unsupported cases and uncertainty.
- Avoid guessed ABI layouts, undocumented offsets or unverified assumptions.

## Do not contribute

- Game executables, game packages, keys, decrypted content or proprietary assets.
- Code that bypasses encryption, DRM, signature validation, licensing or access controls.
- Extracted textures, models, audio, video or game-derived shader data.
- Malware, credential theft, stealth or anti-cheat evasion.

## Pull-request checklist

- [ ] The change is small and independently reviewable.
- [ ] Tests cover success and malformed input where relevant.
- [ ] Windows CI passes.
- [ ] Linux CI passes, or the platform-specific limitation is explained.
- [ ] Documentation describes what is implemented and what remains unsupported.
- [ ] No proprietary game data or secrets are included.
