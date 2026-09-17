# Shipyard overlay-first implementation — Slice 1A

**Exact input Git revision:** `3cfe9676b80ba283682d99a7604c3403ce965fb2` (`QG-20260917-021024-full-e741d49a`). This patch is a source change, not Windows runtime certification. Do not apply over later local modifications to the files listed in the patch manifest.

## Delivered in this slice

* Shipyard's actual `SubspaceDockSystem::Materialize` path uses a full available viewport rectangle independently of any panel. The legacy split tree remains only as the existing tab/anchor ownership and serialization metadata, not for carving out renderer canvas space. Moving, floating or closing panels cannot shrink the materialized viewport.
* Tool rail is a normal floatable/closable tool. It has a small grip at the top, leaving toolbar buttons accessible underneath. It can drag out and dock back at the left anchor. The Asset Browser, Outliner, Properties and other registered panels use independent canvas-edge overlay anchors.
* Dock pointer drop target lookup uses the same compositor/materialized rectangles as paint and hit testing, rather than recursively interpreting a separate split-tree geometry. Edges remain available as explicit, small default anchors; dropping over a visible panel joins its existing tab leaf. Other panels cannot be dropped into the compact tool-rail anchor.
* Docked and floating tool bodies block picking through to the 3D canvas. The existing R5 per-panel opaque painting and frontmost float semantics are preserved. The top application/menu/workspace bars and the compact bottom status bar remain separate fixed chrome.
* User-only overlay layout is loaded after the shipbuilder initializes and saved after panel commands and completed drag/resize operations. Corrupt or incompatible persisted layouts are rejected without touching the ship document; the default layout remains usable. Reset layout is saved too. Location: `dist/user/shipyard_overlay_layout_v1.txt` relative to the process working directory.
* Native `SubspaceShipyardOverlayFoundationTests` and a scoped static guard are integrated into the existing CMake/CTest/PCC pipeline. The historical R3 test was updated for the new canvas contract instead of forcing an obsolete reserved sidebar.

## Important limits — do not advertise as finished

* This is **Slice 1A**, not full GUI normalization. It provides compact existing-leaf docking and edge anchors; it does not yet implement a visible drag ghost, selectable split previews, arbitrary split-node creation, new native OS windows, DPI migration or monitor-aware rebinding.
* The existing 3D camera/projection, ray-picking and screen-world conversion code still contain whole-window coordinate paths. The canvas is now full **workspace width** behind tools, but the fixed top/status chrome creates a vertical coordinate-transform problem to handle as one dedicated **Slice 1B** change. Do not claim that viewport projection/picking has been completely normalized.
* Legacy model-less shell-layout fallback remains for older compatibility tests. The active Shipyard runtime must pass the actual model through `ShipyardBuilderSystem::Layout(model,w,h)`; do not use the fallback for render/pointer authority.
* Blank registered modeling/interior/systems panels, complete blueprint persistence, real FPS ship interiors and a fully wired TEST action are outside this slice. They are next integration work after the GUI foundation and document authority.
* Linux game link and focused unit/source-gate checks do not prove that the Windows OpenGL editor looks correct. The project-owned Windows PCC and on-screen visual tests are required before certification/publish.

## Windows acceptance checklist

1. Confirm HEAD and patch receipt. Run PCC **option 1**. Do not commit if any test fails.
2. Run Shipyard at maximized and then smaller supported size. Record the viewport grid and ship framing. Float Asset Browser, Outliner, Properties and Tools in turn. The canvas background and viewport boundaries must not change; moved windows must be opaque and not show underlying text.
3. Drag the Tools grip without triggering a tool button, move it, drop on the left edge and confirm it is usable again. Hide and reopen it using a workspace control. Move the Asset Browser up/down: Tools and canvas boundaries must remain stationary.
4. Move three overlapping panels, bring an older one to the front, click its empty body, then click the visible ship. UI clicks must never select the ship through a tool; visible canvas clicks must still select objects.
5. Close and reopen the executable. Window placements should restore; Reset Layout should revert only panels, never alter the ship. A malformed layout file should fail safely and restore defaults.
6. Confirm camera MMB pan directions and RMB near-pole behavior remain as in certified PASS1508R5. Spot-check asset selection, placement and undo. Save an original screenshot and a screenshot with all three panels floated.

**Stop conditions:** no Windows GREEN; unexpected change in local HEAD; user-authored edits to any overwritten file; missing patch receipt; layout corrupted; shifted world picking. Report a debug bundle and screenshot rather than committing a visually broken pass.
