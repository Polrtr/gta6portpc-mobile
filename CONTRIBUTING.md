# Contributing

Thanks for helping improve this research workbench.

## First, choose the correct repository

- Documentation, progress reports, reproducibility notes and contributor coordination belong here: [Polrtr/gta6portpc-mobile](https://github.com/Polrtr/gta6portpc-mobile).
- Runtime source-code changes belong in the upstream project: [ItsNotRexed/Leonida](https://github.com/ItsNotRexed/Leonida). Read and follow its [CONTRIBUTING.md](https://github.com/ItsNotRexed/Leonida/blob/main/CONTRIBUTING.md) before proposing code changes.

Do not open a runtime-code pull request here as though this were the official Leonida source repository.

## Good contributions here

- Clear build instructions that have been tested.
- Test logs with exact source revision and environment details.
- Explanations of a failure that distinguish evidence from guesses.
- Corrections to documentation or links.
- Roadmap updates backed by an actual test or upstream change.

## How to contribute

1. Open an issue or check existing ones before starting duplicate work.
2. Fork this repository on GitHub.
3. Clone your fork and create a topic branch:

```cmd
git clone https://github.com/YOUR-USERNAME/gta6portpc-mobile.git
cd gta6portpc-mobile
git switch -c docs/short-description
```

4. Make a focused change and check the Markdown/YAML formatting.
5. Commit and push:

```cmd
git add .
git commit -m "docs: explain a reproducible test"
git push -u origin docs/short-description
```

6. Open a pull request against this repository and explain what changed and how you verified it.

## Rules for test reports

Always include the exact command, upstream Leonida commit SHA, compiler/toolchain versions and relevant output. Include GPU/driver details for graphics investigations.

## Do not upload

- Game executables, archives, package files or proprietary assets.
- Keys, credentials, saves or personal information.
- Extracted game textures, models, audio, videos or shader data.
- Logs that contain proprietary content or private paths/secrets.

Use synthetic test cases and short diagnostic excerpts wherever possible. Do not bypass encryption, DRM or access controls.
