# Subspace Pass1202R2 Handoff — PCC Historical Gate Repair

Baseline GitHub main: `7e9fda2e19b436d7f9cde90175a1dcc69615fba5`.

Apply after Pass1202R1. The prior Full Gate reached 95/96 CTest targets and
failed only `SubspacePass746R6R4HistoricalRootAuditUnbornGitSourceGate`.

R2 removes the direct native HEAD probe from `Invoke-GitPush` and routes HEAD
lookup through the shared unborn-safe ProjectOps Git state authority.

After apply: run PCC option 1. Do not use option 2 until the Full Gate is GREEN.
