# Codename Subspace — Standalone Project Control Center Authority

**Operational authority:** `SUBSPACE-INTERNAL-PCC-20260912`

Until Forge is fully integrated into Cortex and explicitly promoted back into the workflow, Codename Subspace is operated as a self-contained project. ForgePY is optional/deferred and is not required for patching, building, testing, certifying, running, or publishing Subspace.

## Operator workflow

1. Drop a project `.patch` file unextracted into the repository root.
2. Launch `SubspaceTools.cmd` with no arguments.
3. The internal PCC scans root `.patch` transports and prompts **Apply this patch now? [Y/N]**.
4. Approved patches are manifest-validated, baseline-checked, backed up, applied transactionally, verified, receipted, and archived under `updates/`.
5. **Option 1 — FULL QUALITY GATE / CERTIFY GREEN** delegates to the existing authoritative `SubspaceTools.ps1 -Action full-gate` pipeline.
6. Test/play the resulting build.
7. **Option 2 — COMMIT + PUSH CURRENT GREEN** delegates to the existing guarded GREEN commit and Git push actions. It does not force-push.

Direct/machine calls such as `SubspaceTools.cmd -Action full-gate` bypass the interactive wrapper and continue to use the existing project control utility, preserving automation compatibility.

## Patch format

Normal standalone transports use extension `.patch` and are ZIP containers with `PATCH_MANIFEST.json` at package root. `schemaVersion` is `1`. Paths are repository-relative. Each file has SHA-256 and byte count. Optional removals are explicit. A package may declare `projectId=codename-subspace` and `baseline.gitCommit`.

The internal PCC intentionally refuses unknown/plain unified-diff `.patch` files on the automatic lane. This avoids the ambiguity that caused previous patch failures. Legacy ZIP patch intake remains owned by the existing project tooling for recovery compatibility.

## Safety rules

- `.git` may never be written by a patch.
- `updates/inbox` may never be written by a patch payload.
- Baseline mismatches fail before writes.
- Partial writes roll back from per-file backups.
- Every successful/duplicate application creates a transaction receipt.
- Pending root `.patch` files block option 1 so a GREEN gate cannot accidentally certify the wrong tree.
- Option 2 remains dependent on the existing project GREEN fingerprint checks.


## Windows PowerShell 5.1 compatibility

The internal PCC and both patch-intake lanes must run under Windows PowerShell 5.1 on the normal Windows development machine. Do not use .NET Core-only APIs such as `System.IO.Path.GetRelativePath()` in project-control scripts. Relative paths use the project-owned normalized containment helper instead. Patch failures are archived as evidence and return control to the PCC menu rather than terminating the entire PCC.
