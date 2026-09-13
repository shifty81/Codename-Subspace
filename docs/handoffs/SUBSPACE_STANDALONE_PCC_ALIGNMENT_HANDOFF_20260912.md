# Subspace Standalone PCC + Alignment Handoff — 2026-09-12

Target baseline: GitHub `main` commit `1f714de6ea5c86ae45d38e01cd6167ca3dc6e5d3`.

This cumulative handoff changes only the project control **front door**, adds a standalone `.patch` transaction engine, and promotes the September 12 alignment decisions into repository documentation. The existing `SubspaceTools.ps1`, current Full Gate, ProjectOps, source authority, native runtime, tests and current game source are intentionally not replaced by this handoff.

### Test order
1. Apply this source update/patch to current `1f714de` source.
2. Launch `SubspaceTools.cmd` with no arguments.
3. Confirm the simple internal PCC appears and no Forge process is required.
4. For patch-path testing, place the cumulative `.patch` in root; startup should prompt; because the update is already present it should classify it as `ALREADY-APPLIED` and archive it without modifying source.
5. Run option 1 and provide the resulting Full Gate/debug evidence.
6. Test the game.
7. Only after satisfactory testing, use option 2 to commit + push current GREEN.

Direct `SubspaceTools.cmd -Action ...` calls still route to the original utility for automation compatibility.
