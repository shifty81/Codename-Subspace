PASS1507 Dock active-tab overlap repair — one-click patch generator
===================================================================
WHY GENERATOR: the ChatGPT container has only an OLD September 12 source ZIP.
This small generator reads your certified 31442f2 source directly from YOUR
clean repository, checks exact Git commit and the original docking-file Git
blob, and constructs a conventional PCC-native .patch there. It never writes
any existing source file. The internal PCC handles approval, backup, rollback,
application and GREEN certification.

STEPS:
  1. Extract this generator ZIP anywhere OUTSIDE the repository (Desktop).
  2. Drag C:\Users\Shifty\Desktop\CodenameSubspaceNullharbor onto
     Generate_PASS1507_DockFix.cmd, or invoke the .cmd with that quoted path.
  3. The script places ONE .patch in your project root after validating that
     Git HEAD is 31442f2 and the working tree is clean.
  4. Launch the project's SubspaceTools.cmd, approve the new patch at startup.
  5. Run Full Quality Gate, then launch Shipyard and verify right-hand tabs.

If the script says PRECONDITION FAIL, STOP. Do not disable the check or
reconstruct from an old source. It means the repo has moved or source differs.

WHAT IS FIXED: dock area materialization now returns the active eligible tab,
not every tab piled into the same rectangle; closing an active tab promotes
an available sibling. A separate native CMake/CTest regression is included.

NOT YET FIXED: drag-to-split, detachable OS windows, tooltip timing, every
Shipyard workflow and the rest of PASS1507-1515. GREEN still needs Windows PCC.
