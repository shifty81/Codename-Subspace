# Foundation Convergence Authority

Foundation Convergence is the architecture milestone immediately following the certified P921 editor/client parity baseline.

Current locks:

- world and physics authority are full 3D;
- tactical planar control/view modes may exist, but may not erase 3D world truth;
- `VectorTravelSystem` is the in-system travel foundation;
- the legacy charged sector-jump `NavigationSystem` is migration-only and is not registered in production runtime;
- interstellar travel authority is a future physical `JumpGateTransitSystem` with destination-ready handoff;
- persistent identities survive representation, streaming, docking, landing, jump transit and save/reload;
- `SpatialFrameSystem` reparenting must preserve world point and world velocity through rotated/moving parent frames;
- solid planets are valid landing targets by capability; gas giants use orbital/atmospheric infrastructure;
- `HomeSystemSaveGame.cpp` is retained only as migration/reference source and is excluded from the production static library;
- P921 is the performance-reference baseline and convergence work must preserve or improve perceived responsiveness.

This milestone intentionally avoids renderer/editor rewrites while world authority is being corrected underneath them.
