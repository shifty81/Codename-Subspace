# Codename Subspace Repository Authority

## Canonical remote

- GitHub: `shifty81/Codename-Subspace`
- Canonical branch: `main`
- Runtime authority: native C++ only
- Current operator authority: internal standalone PCC

## Normal promotion rule

1. Drop/approve the intended root `.patch` through the internal PCC.
2. Run **1 — FULL QUALITY GATE / CERTIFY GREEN**.
3. Test/play the resulting certified build.
4. Use **2 — COMMIT + PUSH CURRENT GREEN** only while the working/source fingerprint still matches the accepted GREEN record.

Patch application is never equivalent to acceptance. Publication must fail closed when the source changed after certification.

## Included authority roots

`engine/`, `GameData/`, `content/`, `scripts/`, `tools/`, `docs/`, `project.control.json`, and root project-control/documentation files.

## Explicit non-authority

Historical C#/AvorionLike source, old pass artifacts, build trees, logs/debug bundles, `.subspace/`, update transaction history, generated caches, and ungoverned third-party binaries are not shipping source authority.

The older first-time normalized-main force-with-lease workflow is historical migration machinery, not the normal day-to-day publication path.
