# Pass1202R1 — PCC Commit/Push Idempotency Repair

This repair addresses the false failure observed after the Pass1163-1202 Full Gate.

## Root cause

Option 2 created the correct certified commit, then a later guard compared the new HEAD to the pre-commit GREEN marker and interpreted the expected HEAD transition as post-GREEN corruption. The commit subsequently reached GitHub, but the PCC reported FAIL.

## Corrected behavior

- Governed source and certifiable worktree bytes must still match the GREEN gate exactly.
- Before the first commit, the GREEN gate's Git HEAD remains a strict lineage precondition.
- If option 2 is repeated after the certified commit was already created, the PCC recognizes the one direct certified commit over the GREEN parent when no certifiable changes remain.
- Push and remote verification are separate reported stages.
- `git ls-remote --heads origin <branch>` verifies that the remote branch equals local HEAD.
- A non-zero push process result is not treated as failure when remote verification proves origin already contains the exact local HEAD.
- No force push is introduced.

The Full Quality Gate remains the only promotion authority.
