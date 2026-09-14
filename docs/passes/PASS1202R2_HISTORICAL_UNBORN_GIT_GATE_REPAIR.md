# Pass1202R2 — Historical Unborn-Git Gate Repair

Pass1202R1 correctly repaired the false option-2 commit/push failure, but its
remote-verification path introduced one direct PowerShell-native HEAD probe.

The historical clean-clone certification gate intentionally forbids that probe
because an initialized repository with no first commit can emit ordinary Git
diagnostics into PowerShell's error stream.

## Repair

- `Invoke-GitPush` now reads branch/HEAD through `Get-ProjectGitRepositoryState`.
- The shared ProjectOps Git probe remains the authority for unborn-safe HEAD detection.
- Push behavior, remote-head verification, idempotent option 2 behavior, and no-force policy are unchanged.
- No game/runtime source is changed by this repair.

The Pass746R6R4 historical unborn-Git source gate is the regression authority.
