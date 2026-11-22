# GTA6 Port PC — Mobile / Research Workbench

> Unofficial research companion for experiments around **Leonida**, an independent Xbox Series-to-Windows compatibility project. This repository is not affiliated with Rockstar Games or Take-Two Interactive.

This repository is a place for our own progress logs, reproducible Windows test instructions, technical notes and contributor coordination. It is **not a standalone GTA VI port or a copy of the Leonida source code**.

## Start here

| I want to… | Go to |
|---|---|
| Understand the project and its current limits | [Project status](#current-status) |
| Set up the correct source repository | [Getting started](docs/GETTING_STARTED.md) |
| Compile the Windows runtime | [Windows build guide](docs/BUILD_WINDOWS.md) |
| Run and report tests | [Testing guide](docs/TESTING.md) |
| Understand the code layout | [Architecture notes](docs/ARCHITECTURE.md) |
| See what needs work next | [Roadmap](docs/ROADMAP.md) |
| See what Leonida still lacks | [Upstream gap analysis](docs/UPSTREAM_GAPS.md) |
| Help with documentation or research | [Contributing](CONTRIBUTING.md) |

## Important: there are two repositories

- **This repository:** [Polrtr/gta6portpc-mobile](https://github.com/Polrtr/gta6portpc-mobile) — our documentation and experiment log.
- **Upstream source:** [ItsNotRexed/Leonida](https://github.com/ItsNotRexed/Leonida) — the original runtime and implementation. Source-code work must respect its own license, contribution rules and project status.

When following the build guide, clone the upstream Leonida repository. Cloning this documentation repository alone will not give you the runtime source files.

## Current status — 22 November 2026

The latest local test record in this repository says:

- The configured Windows x64 Release runtime build completed successfully.
- CTest registered 64 tests: 63 enabled and 1 disabled.
- **61 of the 63 enabled tests passed; 2 failed.**
- The open failures are a D3D12/Vulkan wave-lane capability comparison and a group of texture-copy format cases.
- A trivial test executable passed the loader smoke test.

The previous 28/28 test result is kept in the history, but it is older and is superseded by the later 61/63 run. See [PROGRESS.md](PROGRESS.md) and [Testing](docs/TESTING.md).

**GTA VI has not been launched by this workbench.** A passing loader smoke test or runtime unit test is not proof that the real game starts, renders correctly or is playable. The upstream Leonida README also describes the project as early and says the game does not boot yet. Always check the upstream status before drawing conclusions.

## Scope

The current recorded experiments target **Windows x64** and the Leonida runtime. Despite the word “mobile” in this repository's name, Android and iOS builds are not implemented or claimed.

This project focuses on:
1. Reproducible runtime builds and tests.
2. Understanding loader, API-shim and graphics compatibility gaps.
3. Recording failures with enough detail for another developer to reproduce them.
4. Keeping legal, safe and clearly attributed research notes.

## Legal and safety

- This repository does not distribute GTA VI, game executables, proprietary assets, keys or decrypted game data.
- Only use files you are legally authorised to access.
- Do not bypass encryption, DRM or access controls.
- Never commit game archives, executable files, extracted assets, shaders, keys, saves or private logs containing game data.
- Leonida's original code and credits belong to its upstream authors. Any reuse or redistribution must follow the upstream license and notices.

## Project links

- [Leonida upstream source](https://github.com/ItsNotRexed/Leonida)
- [Upstream contribution guide](https://github.com/ItsNotRexed/Leonida/blob/main/CONTRIBUTING.md)
- [Our progress log](PROGRESS.md)
- [Report a documentation or reproducibility issue](https://github.com/Polrtr/gta6portpc-mobile/issues/new/choose)
