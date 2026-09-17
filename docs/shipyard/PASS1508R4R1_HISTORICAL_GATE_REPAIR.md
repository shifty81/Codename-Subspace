# PASS1508R4R1 — Historical DCC source-gate reconciliation

**Input:** `Subspace_DebugBundle_20260917-011231.zip`; Full Gate failed 2 of 101 CTests after PASS1508R4. Native C++ compiled.

Two historical source gates still require the literal phrase `a neutral DCC canvas replaces the black in-game-space`. PASS1508R4 replaced the obsolete backdrop implementation, removing that comment. Both historical tests therefore fail despite the new R4 geometry tests passing.

This corrective package changes only the two stale **test scripts**. Both now inspect the real live-workspace layout call and the viewport-bounded backdrop drawing; the older staged-part, perspective-grid and other historical checks are retained. No renderer, gameplay, UI or PCC source is overwritten.

This fixes **the test mismatch only**. It does not prove that moving the Asset Browser is visually correct; a rendered, interactive follow-up is required.

Drop the `.patch` directly next to `SubspaceTools.cmd`, approve it in the PCC, then run the full gate. Run the editor and inspect the Asset Browser/rail behavior separately. Do not commit until both certification and visual inspection pass.
