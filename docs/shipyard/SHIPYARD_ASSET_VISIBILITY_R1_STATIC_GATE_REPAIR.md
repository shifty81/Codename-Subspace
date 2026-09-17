# Shipyard Assets R1 — historical static-gate repair

Baseline: `cf0bf03aa553ec3ae00a4965572c70085f2664ba` with the already-applied `Subspace_Shipyard_Assets_Visibility_Recovery_cf0bf03.patch` overlay.

The Windows Full Quality Gate on 2026-09-17 at 17:41–17:42 built the native C++ project, then passed 108 of 109 tests. The sole failure was the historical `SubspaceProjectOpsStaticCertification` gate `pass1338_shipyard_blender_dcc_shell.cmake`, which searched the renderer for the non-interactive text `View   Select   Add   Object`. The Asset Browser recovery intentionally removed that text because it covered the new global ASSETS and RESET UI controls in the viewport header.

This repair updates *only the historical assertion*, not the renderer, gameplay, assets, docking model, or PCC intake implementation. It now verifies the 3D VIEW label, the real ASSETS and RESET UI control construction, their action handlers, and the real `OpenPanel` recovery call. Remaining original Pass1338 assertions stay intact. It does not certify that File/Edit/View/Help menus work, or that the visual GUI has been approved.

Local reproduction: old gate FAILED with the missing-token error; updated isolated gate PASS; a negative check removing the ASSETS control signature FAILED as expected. Full static certification cannot be replicated from the older archived source ZIP because its historical `docs/design/SHIPYARD_BLENDER_DCC_100_PASS_ROLLUP.md` is absent, although the live GitHub repository contains that document. Windows PCC Full Gate and visible Asset Browser acceptance remain authoritative.

Install this .patch alone, unextracted, beside SubspaceTools.cmd while the Assets recovery patch is already applied; approve in PCC, run Full Gate, then Run & Play to verify the ASSETS control actually reopens the Asset Browser after closing, collapsing, floating, maximizing, and restarting. Do not commit if it fails visual acceptance.
