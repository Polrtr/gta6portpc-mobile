# HORIZON test-run template

Copy this template into a test report. Fill in only facts that were actually observed.

## Run — YYYY-MM-DD

### Source
- Repository:
- Commit SHA (`git rev-parse HEAD`):
- Working tree status:

### Environment
- OS/version:
- Architecture:
- CPU:
- GPU/driver (if graphics tests are involved):
- Compiler/version:
- CMake/version:
- Generator:

### Commands
```cmd
cmake -S . -B build
cmake --build build --config Release --parallel 4
ctest --test-dir build -C Release --output-on-failure
```

Replace the examples with the exact commands actually used.

### Results
- Configure: pass/fail/not run
- Build: pass/fail/not run
- Tests passed:
- Tests failed:
- Failure details:
- Inspector sample type: synthetic / authorised public sample / not run
- Any limitations:

### Interpretation

Separate confirmed observations from hypotheses. Parser tests do not demonstrate that Xbox software runs on Windows, and no result should be described as GTA VI boot or gameplay unless that has been lawfully and reproducibly demonstrated.
