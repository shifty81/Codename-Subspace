# Studio G01 — operational identity and debug evidence audit

**Implementation scope:** a real, read-only operational census delivered as a single PCC `forge.patch.v1` root-drop. G01 is NOT the G02 close-path fix, G03 visible modeling fix, GUI redesign, complete source audit, or a Windows GREEN receipt. Preserve existing sources and approved patches.

## Why first

The 2026-09-19 11:38 debug archive documents a Studio `run-studio` session with a verified blueprint-only recovery, an explicit warning that model/interior/socket drafts were not recovered, exit code 7, and subsequent PCC failure/debug packaging. The title-bar modal selection is user reported but not recorded as a structured close receipt. The published `StudioApplication::Run` returns code 7 whenever unsupported unsaved drafts remain, even after the close guard permits acknowledged partial recovery. Repair G02 must use explicit close intent/outcome, not unconditional `return 0` or blanket PCC suppression.

GitHub source baseline examined: `14a5ca8b6039868b7968b3b00018307c6306e3ca`. The installed local checkout and actual running executable are not accessible from this handoff. G01 captures their fingerprints for safe G02/G03 source application.

## Usage from the repository root

```bat
tools\studio\StudioReadinessAudit.cmd --debug-bundle "C:\path\to\Subspace_DebugBundle_20260919-113826.zip" --binary "C:\path\to\subspace_studio.exe"
```

If the binary path is unknown, omit `--binary`. If the debug archive is unavailable, omit `--debug-bundle`. Output defaults to `.subspace/reports/studio/studio_readiness_latest.json`. `--stdout` produces JSON without writing files. `--out <path>` overrides output location. The tool never extracts a debug ZIP; it reads selected summary/log entries only and stores only signal flags and file hashes. It does not store raw logs or home-directory paths in the JSON output, invoke build tools, update source, change Git, apply patches, promote a gate, or independently classify a UI click as proven.

**Windows prerequisites:** Python 3 accessible as `py -3` or `python`; `git` optional (unavailable Git is marked unknown). This operation can be invoked from Forge/Cortex as a read-only project tool after provider registration, but it does not install that adapter or a Studio menu button.

## Acceptance

1. A wrong repo root fails before creating reports.
2. Local HEAD, branch and tracked changes are reported separately from the immutable source-file hashes.
3. `project.control.json` is parsed and Studio command keys enumerated; no command executed.
4. Actual executable path is supplied by user; its SHA256 is returned without guessing it was the running instance.
5. The Sep19 debug archive yields the known exit-7 signature, *not* a fabricated crash stack or proven modal choice.
6. A missing/malformed debug archive is reported without extraction or hidden exception.
7. All nine portable unit tests pass; Windows PowerShell/CMD invocation and original PCC intake remain to be tested locally.

## Next source passes

**G02:** introduce an explicit close decision/outcome/receipt in the Studio controller and share it with PCC. Intentional confirmed discard can exit 0 but must log exactly which lanes were discarded. Real recovery failure must remain nonzero and veto close. Test X, Alt-F4, File > Exit, Cancel, Save, discard, recovery, reentry, and game isolation.

**G03/G04:** introduce complete versioned document/session and visible Model scene bridge; a model-only BOX must render, select, edit, persist and reopen with zero ship modules. Ensure full autosave/recovery independent of blueprint-only recovery. Both are prerequisites to a certified GUI-first G10 milestone.

**Cumulative delivery:** G01 is one bounded incremental root package with verified manifest hashes; later source patches build on actual local source and receipts. A final cumulative patch rolls together *implemented* code and test results after milestones, not planning documents. Do not reapply older passed packages or silently push.
