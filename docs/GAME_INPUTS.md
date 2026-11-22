# Game input requirements

> **A title-specific build needs the authorised Xbox executable and the exact companion files required by its verified input manifest. Without those files, HORIZON cannot prepare a test for that title.**

## Where files belong

Keep any authorised private test inputs in the local `private-input/` directory at the root of your HORIZON checkout. That directory is ignored by Git. Never upload game executables, packages, assets, keys or extracted data to this repository, an issue, a pull request or a public file host.

Example local-only layout (illustrative, not a verified GTA VI package layout):

```text
HORIZON/
  private-input/
    <authorised-executable-and-required-companions>
  game-output/
  build/
```

## Filenames are not guessed

HORIZON does not currently define a fixed GTA VI executable filename or a complete list of companion files. A package manifest or a marketing claim is not enough to prove the file layout. The builder must be updated only after the authorised file set and formats have been independently verified.

Do not rename files to make them appear compatible. Do not edit headers, remove signatures, decrypt protected content or bypass access controls.

## Important runtime limitation

Having the required files is necessary for a future title-specific build, but it is not sufficient to run a game today. The current HORIZON prototype has no Xbox executable loader, no guest ABI adaptation, no Xbox OS/GDK service implementation and no D3D12 translation path. The current inspector only reads PE metadata; it never executes the file.

## Safe validation sequence

When a lawful input manifest is eventually available, the builder should:

1. Check that each required path exists locally.
2. Check basic file type/size constraints without altering the files.
3. Report missing, unknown or unsupported entries.
4. Refuse to create a runnable-game claim while required runtime components are unavailable.
5. Keep inputs and generated title output outside version control.
