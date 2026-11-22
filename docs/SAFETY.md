# Safety, authorisation and data handling

HORIZON is for compatibility research using software, documentation and test cases that the developer is authorised to examine.

## Allowed project material

- Original HORIZON source code.
- Public platform documentation and links.
- Synthetic executables and fixtures created for testing.
- Minimal diagnostic output that does not reveal proprietary content or secrets.
- Test results and reproducible build information.

## Do not add to this repository

- Commercial game executables or packages.
- Product keys, signing keys, credentials, tokens or account data.
- Decrypted or unpacked protected game content.
- Proprietary textures, models, maps, audio, video or game-derived shader data.
- Saves or logs containing personal data or proprietary file content.

Keep authorised private test files outside the repository. Do not publish hashes or metadata if doing so would disclose private or restricted information.

## Security boundaries

HORIZON must not be used to bypass encryption, DRM, signature validation, licensing, account checks, anti-cheat or other access controls. Do not disable security checks or conceal execution from a platform or service.

## Safe engineering

- Inspect files read-only by default.
- Validate offsets, lengths and integer arithmetic before reading.
- Use synthetic fixtures for malformed input.
- Do not automatically execute files passed to inspection tools.
- Do not treat metadata as proof of authenticity or trust.
- Keep unknown formats opaque and fail closed.

## Compatibility claims

A unit test validates only the tested code path. A parser recognising a PE header does not show that a console application can run on Windows. A graphics demo does not show that a commercial game is supported.
