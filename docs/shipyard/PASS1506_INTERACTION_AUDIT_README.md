# PASS1506 — read-only Shipyard interaction inventory

**Scope:** first incremental foundation for PASS1506–1515. This is a real, runnable
source-inventory utility, **not** the completed PASS1506 runtime acceptance gate
and not a claim that any GUI control is functional. It adds only new files and
leaves PCC, renderer, camera, and ship/runtime implementation untouched.

From the project root, run:

```powershell
py -3 tools\control\ShipyardInteractionAudit.py --root .
py -3 -m unittest discover -s tools\control\tests -p test_shipyard_interaction_audit.py -v
```

If `py` isn't available, use `python` instead. Results are written to
`artifacts/reports/shipyard-interaction/interaction_audit.json` and
`interaction_audit.md`; they will not be added to the normal source package.
The audit uses only Python's standard library and does not load or modify the
project, require external assets, or make network calls. `--strict` returns
code 2 for undefined command references or commands with zero source usage,
but is **not** intended to replace Full Quality Gate.

## Interpretation

- `case_present_behavior_unverified`: native editor command case exists;
  actual action, side effects, visual result, click target, and undo are unknown.
- `manual_review_no_ship_editor_handler_case`: command references were found,
  but no ship-editor switch case; search other dispatch mechanisms before
  declaring it broken.
- `no_source_reference`: command is declared but not found in C++ source.
- `statusOnlyCandidates`: a narrow heuristic for one-line cases that only
  update status text and return true. These may still be implemented by the
  application layer; confirm by running the game.

This is deliberately **not** a string-token certification test. Results provide
file and line evidence and enumerate uncertain behavior for a human/runtime
verification pass.

## Next implementation boundary

Before completing PASS1506, instrument the actual displayed control list and
hit-test path in the real PASS1505R1 Windows build: for each control record
workspace, command, label, bounds, visible/enabled state, disabled reason,
hover, click dispatch, result, side effect, and undo receipt. Test 1280×768 and
1920×1080, assert non-overlapping interactive rectangles, and capture an
actual rendered screenshot plus structured telemetry. Only then promote
functional acceptance. Continue docking, guided workflow, camera, modeling,
pivot/articulation, interior, material and kitbash passes on that baseline.

**Compatibility:** this payload contains only three new paths. If any destination
already exists in the current live tree, do not overwrite it; compare first.
The September 12 reference source was used to exercise this scanner, not as
the source authority for live game changes. The current local Windows GREEN
remains PASS1505R1 until its own PCC certifies another state.
