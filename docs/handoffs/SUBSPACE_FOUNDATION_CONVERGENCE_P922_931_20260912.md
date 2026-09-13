# Subspace Foundation Convergence — Pass922–931

Pass922–931 begins the architecture convergence required by the Two-System Reference Slice while preserving the certified P921 performance-reference baseline.

Implemented:

- full-3D world/physics authority is now the default controller contract; tactical planar constraint is explicit opt-in only;
- solid generated planets expose landing capability while gas giants remain non-surface targets;
- legacy charged sector-jump `NavigationSystem` remains migration code but is no longer registered in the production Engine;
- `HomeSystemSaveGame.cpp` is excluded from the production static library because it calls detached legacy home/expedition implementation symbols;
- `SpatialFrameSystem` reparenting now preserves world point and world velocity through rotated and moving parent frames;
- deterministic representation-independent persistent IDs are introduced for worlds, systems, planets, cells, ships, characters, settlements, stations, vehicles, survey records and module instances;
- persistence schema v2 contracts establish ship/cell/settlement/survey record identity surfaces without replacing the current save manager yet;
- P921 is recorded as `SUBSPACE-P921-PERFORMANCE-REFERENCE-BASELINE`; future hard budgets require measured telemetry rather than invented numbers;
- commit-GREEN fingerprint mismatch now prints certified/current fingerprints and changed paths before refusing publication.

Validation in the portable environment:

- full configured build: PASS
- CTest: **84/84 PASS**
- Pass922–931 focused assertions: **13/13 PASS**
- Pass912–921 regression: PASS
- Pass892–901 dock regression: PASS
- ProjectOps static certification, including the new convergence gate: PASS

Windows internal PCC Full Gate remains required before this pass is GREEN.
