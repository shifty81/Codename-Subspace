# G4 R2 — Hull visibility regression repair

Exact baseline: cf5107e4b36881ebfbcb46325a31558e84979597. This is a standard PCC root-drop `.patch` containing full verified source overlays. Do not extract or run a builder. Do not reapply G4/G5 foundation or R1.

Root cause: the INTERIOR workspace defaults to InteriorOnly, but leaving that workspace does not restore EXTERIOR. The renderer intentionally excludes hull geometry in InteriorOnly even if a placement is correctly present in the Outliner.

Fix: both clicked workspace commands and keyboard workspace cycling restore EXTERIOR when leaving InteriorOnly, while explicit Cutaway/X-Ray remain intact. Successful AddModule/ConfirmPlacement also reveals an exterior hull. Only the view policy and its existing front-end caller are changed; renderer, meshes, fleet, camera, PCC and gameplay are untouched.

Verify: Start blank Studio, enter INTERIOR, click ASSEMBLY, drag/place first hull and confirm ghost and geometry are visible and the Outliner agrees. Repeat via keyboard workspace cycling. Confirm Cutaway/X-Ray remain intentionally selected. Confirm in-game Shipyard/cockpit is unaffected. If EXTERIOR still fails, inspect runtime logs for missing certified Shipyard module assets. Run Windows PCC Full Gate and visual check; only commit/push if both pass.

Boundaries: this R2 document belongs to the superseding cumulative G4 R3 release. It does not implement true orthographic projection or fleet terminal interactions; focused view-policy tests are not Windows-render certification. The core BeginCatalogDrag implementation is still not intercepted by this patch; if a user manually selects Interior-only before dragging, the ghost can remain hidden until the view is cycled or placement is committed. Use VIEW > EXTERIOR as immediate recovery and capture a debug bundle if the issue persists.
