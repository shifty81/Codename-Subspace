# Pass746R6R12 — Repository Publish Force-With-Lease Argument Repair

The authoritative Full GREEN `QG-20260908-085543-full-a02617fc` passed.

Repository option 9 then successfully:

- re-audited the old remote main;
- accepted exact `PUBLISH`;
- generated the normalized repository preview;
- revalidated the GREEN Git/source fingerprints;
- preserved the historical remote main on an archive branch.

The final guarded push failed because the force-with-lease option was emitted as
two native Git argv entries:

`--force-with-lease=refs/heads/main: <old-sha> origin main:main`

Git therefore interpreted the old SHA as the repository/remote and `origin` as
a refspec.

R6R12 precomputes the lease option as a single scalar using `String.Concat`:

`--force-with-lease=refs/heads/main:<old-sha>`

and pushes with the explicit argv order:

`push`, `<lease>`, `origin`, `refs/heads/main:refs/heads/main`

The remote SHA is also validated as a 40-character object id before any final
main replacement is attempted.

No game/runtime source changes.
No engine/CMakeLists.txt changes.
No root launcher overwrite.
