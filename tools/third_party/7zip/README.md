# Project-local 7-Zip bootstrap

Codename Subspace may populate this directory with `7zr.exe` when a governed third-party `.7z` asset archive must be extracted and no compatible 7-Zip command is already installed.

- The executable is **not bundled** in Subspace source handoffs.
- It is downloaded on demand from the official `ip7z/7zip` release repository over HTTPS.
- The intake script records the downloaded executable's SHA-256 and source URL in `PROVENANCE.generated.json`.
- This cache exists only to make approved content intake reproducible and project-local; it is not a game runtime dependency.
