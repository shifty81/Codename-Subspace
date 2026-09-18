# Shipyard Studio G3 R1 — compositor certification repair

Input: certified Git 9c4689a7419e93223089c58275555baa327e1e8a with the **rebased**
`Subspace_Studio_G3_04_Rebased_9c4689a_Seamless_Direction.patch` already
applied, as confirmed by the 2026-09-17 21:15:33 update receipt and the native
build and G3 tests in `Subspace_DebugBundle_20260917-211536.zip`.

The Full Gate failed 2 of 115 CTests. The ProjectOps umbrella gate and its
standalone Pass1508R5 gate are duplicate executions of the same stale source
contract: the old hit-test expression rejected every control not belonging to
the uppermost floating dock. G3 added an application popup rendered after the
floating docks, so its hit-test deliberately allows only the `studio_menu`
control family above the topmost dock. The older literal expectation no longer
represented the actual drawing order.

This repair modifies only the Pass1508R5 **static source gate**. It now requires:
- normal topmost floating panel still occludes lower controls;
- the application popup is explicitly ranked above floating docks;
- only that popup is exempted in hit testing;
- the popup is actually rendered after floating panels.

Regression: the original gate reproduced the user's failure; corrected gate
passed. Removing the menu click exception, high rank, or final render call one
at a time makes the updated gate fail. Local umbrella static certification is
incomplete in the provided staged source because a historical rollup document
is missing there, so Windows Full Gate remains authoritative.

Scope: No renderer/gameplay/UI source is overwritten. This does not add planet
streaming, fleet terminal gameplay, or complete GUI/interior modeling. The G3
design direction already documents one seamless simulation and physical
fleet-command seats; these remain forward acceptance requirements.

Apply this incremental .patch through the project PCC on Git 9c4689a with G3
already applied. Do not reapply G3 or install the old cf0bf03 patch. Run Full
Quality Gate then perform visible File/View popup checks with a floating panel.
Do not commit until gate AND visual interaction pass.
