# Codename Subspace Repository Authority

## Canonical remote

- Provider: GitHub
- Repository: `shifty81/Codename-Subspace`
- Canonical branch: `main`
- Runtime authority: native C++ only

Normalized `main` represents the current buildable project, not every historical prototype that has occupied the repository.

## Historical preservation

Before the first normalized publication, the current remote `main` commit is preserved under:

`archive/pre-native-normalization-<timestamp>`

That archive is historical evidence and may retain legacy C#, obsolete root files, old asset layouts, and other donor material.

## Current-main inclusion policy

Normalized `main` is constructed from explicit authority roots: the native `engine/`, `GameData/`, `content/`, `scripts/`, `tools/`, `docs/`, project contract, root Control Center, and current root documentation/configuration.

## Explicit non-authority

- `AvorionLike/`
- legacy `.sln` files
- loose root intake notes
- consumed patch/hash handoff debris
- build trees
- logs/debug bundles
- `.subspace/`
- update transaction history
- generated/derived/imported/cache content
- ungoverned binary/third-party payloads

Legacy C# may remain locally as an ignored donor/reference folder. It must never be built or treated as fallback runtime.

## Promotion rule

Remote publication is permitted only when:

1. repository authority has been prepared;
2. a Full Quality Gate completed with `PASS`;
3. current certifiable Git fingerprint exactly matches that GREEN gate;
4. the normalization preview succeeds;
5. the operator explicitly types `PUBLISH`;
6. remote `main` still matches the SHA used by `--force-with-lease`.

This makes repository replacement fail-closed while preserving old GitHub history.
