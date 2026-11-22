# Test run record template

Copy this block into `PROGRESS.md` for every new local test run. Do not leave the source revision implicit.

## Test run — YYYY-MM-DD

### Source
- Upstream repository: https://github.com/ItsNotRexed/Leonida
- Upstream commit SHA: `paste output of git rev-parse HEAD`
- Working tree changes: clean / list modified files
- Local workbench commit (if applicable): `paste commit SHA`

### Environment
- OS and version:
- CPU:
- GPU:
- GPU driver:
- Visual Studio / MSVC:
- Windows SDK:
- CMake:
- Vulkan SDK / runtime:
- Build configuration and options:

### Commands
```powershell
git -C .\Leonida rev-parse HEAD
git -C .\Leonida status --short
cmake --build .\Leonida\build --config Release --parallel 4
ctest --test-dir .\Leonida\build -C Release --output-on-failure
```

Adjust paths to the actual checkout and build directory. Record the exact commands actually used; do not imply that the example commands were run if they were not.

### Results
- Tests registered:
- Enabled:
- Passed:
- Failed:
- Disabled:
- Failed test names and concise error:
- Loader smoke test:
- Real-game milestone reached: not tested / loader / graphics init / first frame / gameplay
- Logs or evidence (safe text only; never attach game files, keys, proprietary assets or extracted content):

### Interpretation
Separate confirmed observations from hypotheses. A passing unit test or trivial loader smoke test is not evidence that the actual game boots or is playable.
