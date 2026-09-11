# Pass746R6R14 — Historical Guided Repository-Flow Migration

R6R13R1 intentionally replaced the old flat first-publication guidance with the
operator-controlled `Repository / GitHub lifecycle` submenu.

The next Full Gate passed 79/80 tests. The only failure was historical Pass746R4,
which still required the retired literal:

`First publication flow: 2 Prepare -> Full Quality Gate -> 9 Publish`

R6R14 updates that historical gate to certify the new guided Root Utility
contract instead. Audit, repair/adopt, Full GREEN, commit, push/verify and
first-time publish remain separate operator-selected actions; nothing
automatically chains into the next lifecycle stage.

No native/game source changes.
No `engine/CMakeLists.txt` changes.
No `SubspaceTools.cmd` changes.
