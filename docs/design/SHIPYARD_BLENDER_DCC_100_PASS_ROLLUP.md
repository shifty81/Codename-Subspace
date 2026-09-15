# Shipyard Blender DCC — Pass1340–1439 cumulative rollup

This ledger is the authoritative 100-pass continuation after Pass1338/1339. Each pass is intentionally small enough to audit while the cumulative tranche is delivered as one fresh-folder source authority.


## DCC state/layout
- **Pass1340** — Introduce authoritative DCC UI state.
- **Pass1341** — Persist Asset Browser state.
- **Pass1342** — Add panel visibility state.
- **Pass1343** — Add viewport maximize state.
- **Pass1344** — Add grid overlay state.
- **Pass1345** — Add gizmo visibility state.
- **Pass1346** — Add socket overlay state.
- **Pass1347** — Add statistics overlay state.
- **Pass1348** — Add shield preview state.
- **Pass1349** — Add command palette state.
- **Pass1350** — Add Outliner display modes.
- **Pass1351** — Add viewport shading modes.
- **Pass1352** — Add Asset Browser density modes.
- **Pass1353** — Preserve favorites across layout reset.
- **Pass1354** — Promote Blender-derived workspace cycle.

## Asset Browser
- **Pass1355** — Route visible catalog through Asset Browser query.
- **Pass1356** — Respect construction-domain filtering.
- **Pass1357** — Respect ship-class fit filtering.
- **Pass1358** — Add selected-parent compatibility context.
- **Pass1359** — Add ALL asset preset.
- **Pass1360** — Add certified-structure preset.
- **Pass1361** — Add generator-ready preset.
- **Pass1362** — Add compatible-only preset.
- **Pass1363** — Add favorites preset.
- **Pass1364** — Add asset density cycle.
- **Pass1365** — Add thumbnail zoom in.
- **Pass1366** — Add thumbnail zoom out.
- **Pass1367** — Add selected-module favorites toggle.
- **Pass1368** — Add filter clear action.
- **Pass1369** — Retain available-module authority after DCC filtering.

## Outliner and Properties
- **Pass1370** — Build hierarchy-aware Outliner rows.
- **Pass1371** — Derive attachment depth from recipe edges.
- **Pass1372** — Add functional-role Outliner mode.
- **Pass1373** — Add module-class Outliner mode.
- **Pass1374** — Synchronize selected module into Outliner.
- **Pass1375** — Use canonical friendly module labels.
- **Pass1376** — Keep raw IDs out of primary Outliner presentation.
- **Pass1377** — Add compact Properties context rail.
- **Pass1378** — Promote Transform properties context.
- **Pass1379** — Promote Assembly/System properties context.
- **Pass1380** — Promote Socket properties context.
- **Pass1381** — Promote Authoring/DesignDNA properties context.
- **Pass1382** — Promote Material/Appearance properties context.
- **Pass1383** — Add Properties previous navigation.
- **Pass1384** — Add Properties next navigation.

## Hotkeys and workspaces
- **Pass1385** — Adopt G as Shipyard move hotkey.
- **Pass1386** — Adopt R as Shipyard rotate hotkey.
- **Pass1387** — Adopt S as Shipyard scale hotkey.
- **Pass1388** — Adopt T as toolbar toggle.
- **Pass1389** — Adopt N as Properties sidebar toggle.
- **Pass1390** — Adopt Ctrl+Space as viewport maximize.
- **Pass1391** — Adopt F3 as command search.
- **Pass1392** — Adopt Shift+F as asset-filter cycle.
- **Pass1393** — Add previous workspace hotkey.
- **Pass1394** — Add next workspace hotkey.
- **Pass1395** — Preserve flight action compatibility.
- **Pass1396** — Preserve historical editor action indices.
- **Pass1397** — Promote Assembly primary workspace.
- **Pass1398** — Promote Systems/Paint/Interior/Test primary workspaces.
- **Pass1399** — Retain advanced developer workspace shelf.

## Renderer overlays and shading
- **Pass1400** — Make grid respect DCC visibility.
- **Pass1401** — Make grid bounds respect hidden sidebars.
- **Pass1402** — Make grid bounds respect maximized viewport.
- **Pass1403** — Expose active workspace in 3D View header.
- **Pass1404** — Expose active shading in 3D View header.
- **Pass1405** — Make Asset Browser panel visibility authoritative.
- **Pass1406** — Make Tool Rail panel visibility authoritative.
- **Pass1407** — Make Sidebar panel visibility authoritative.
- **Pass1408** — Add live viewport statistics overlay.
- **Pass1409** — Add F3 command-palette overlay.
- **Pass1410** — Switch staged hint to G/R/S vocabulary.
- **Pass1411** — Add real wireframe polygon mode.
- **Pass1412** — Make socket markers obey socket overlay toggle.
- **Pass1413** — Make conformal shield preview toggleable.
- **Pass1414** — Keep build identity visible in status area.

## Responsive layout and compatibility
- **Pass1415** — Tighten Asset Browser width for viewport priority.
- **Pass1416** — Tighten Properties width for viewport priority.
- **Pass1417** — Use compact vertical Tool Rail.
- **Pass1418** — Use compact workspace strip.
- **Pass1419** — Scale thumbnail cards by density.
- **Pass1420** — Scale thumbnail cards by zoom.
- **Pass1421** — Calculate visible asset page dynamically.
- **Pass1422** — Use compact Outliner page sizing.
- **Pass1423** — Preserve legacy commands off-screen.
- **Pass1424** — Keep legacy automation command-discoverable.
- **Pass1425** — Reconcile compact click-target tests.
- **Pass1426** — Reconcile workspace tab-height tests.
- **Pass1427** — Reconcile 4K pane-width expectations.
- **Pass1428** — Reconcile compact Socket tab expectations.
- **Pass1429** — Fix compact 1280x768 panel overlap.

## Certification and rollup
- **Pass1430** — Reconcile Blender source gate to split Outliner/Properties.
- **Pass1431** — Add machine-readable DCC authority contract.
- **Pass1432** — Add Pass1340-1439 static source gate.
- **Pass1433** — Require build-identity tokens in certification.
- **Pass1434** — Require G/R/S/T/N/F3/Ctrl+Space input tokens.
- **Pass1435** — Require Asset Browser/Outliner/Properties renderer tokens.
- **Pass1436** — Require wireframe and shield toggles in certification.
- **Pass1437** — Add cumulative rollup/start-here authority.
- **Pass1438** — Run complete historical compatibility matrix.
- **Pass1439** — Seal BLENDER-DCC-100 PASS1439 cumulative authority.

## Acceptance

- Visible identity: `BLENDER-DCC-100 | PASS1439`.
- Portable engine and game targets must compile/link.
- Historical CTest and ProjectOps static gates must remain green after intentional Blender-DCC reconciliations.
- Windows PCC remains authoritative for Win32/WGL rendering and `--shipyard-smoke`.
