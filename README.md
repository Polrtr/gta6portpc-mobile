# GTA6 Port PC — Experimental Workbench

> **Unofficial, experimental project. Not affiliated with or endorsed by Rockstar Games or Take-Two Interactive.**

This repository documents experiments, build notes and compatibility research around running the Xbox Series version of Grand Theft Auto VI on Windows.

## Credits and upstream project

The runtime and core research used in our experiments come from **[Leonida](https://github.com/ItsNotRexed/Leonida)**, created and maintained by **ItsNotRexed** and its contributors.

- Upstream project: https://github.com/ItsNotRexed/Leonida
- Upstream license: [GNU General Public License v3.0](https://github.com/ItsNotRexed/Leonida/blob/main/LICENSE)
- Consult the upstream repository for its full contributor credits, notices, build instructions and current status.

**Leonida is the upstream project.** This repository is an independent collection of experiments and progress notes; it is not the official Leonida repository and should not be mistaken for an endorsed fork. All credit for Leonida's original code and engineering belongs to its original authors and contributors.

If code from Leonida is copied, modified or redistributed here in the future, its GPL-3.0 terms and applicable copyright notices must be preserved and followed. This README does not relicense upstream code.

## Current progress

See [PROGRESS.md](PROGRESS.md) for the detailed test log.

Verified locally so far:

- Configured the runtime with Visual Studio 2022, MSVC x64 and Windows SDK 10.0.26100.0.
- Built the Release target `gta6rt.exe` and `gta6rt_tls_host.dll`.
- `gta6rt.exe --dry-run leonida_test.exe` mapped a minimal test executable with zero unimplemented imports.
- `gta6rt.exe --run leonida_test.exe` initialised and ran the test program, which exited with code `0`.

These are **smoke tests of a trivial test executable only**. They do not demonstrate that GTA VI boots or is playable.

## Current limitations

- The real game executable declared by the inspected manifest (`game_scarlett_master.exe`) has not been located in the inspected folder.
- No `.xvc` or `.msixvc` package was found in that folder.
- The inspected `gameos.xvd` is a container, not a PE executable that can be passed directly to `gta6rt`.
- Real-game loading, graphics, gameplay and compatibility have not been tested.

## Legal and safety notes

- This repository does not provide GTA VI, game executables, keys, proprietary game assets or decrypted game data.
- Use only game files you are legally authorised to access. Do not upload game files, extracted assets, keys, saves or generated proprietary data to this repository.
- Do not attempt to bypass encryption or access controls.
- Grand Theft Auto and related trademarks belong to their respective owners. This is an independent technical research project.

## Contributing

When reporting a test, include the exact command, toolchain versions and relevant error output. Do not attach copyrighted game content, secrets, keys or generated game assets.

For Leonida source changes and upstream development, contribute directly to [ItsNotRexed/Leonida](https://github.com/ItsNotRexed/Leonida) and follow its contribution guidance.
