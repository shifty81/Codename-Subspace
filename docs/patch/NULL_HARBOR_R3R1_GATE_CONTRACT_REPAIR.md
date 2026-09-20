# Null Harbor R3R1 — certification contract correction

Scope: fix the failed `SubspaceProjectOpsStaticCertification` caused by the R3 package requiring a shader-source marker it did not deliver. This patch touches only the two R3 verification scripts; it does not change shader, widget, game, Studio, project identity, or user data.

R3's source gate incorrectly required `vObjectPos = gl_Vertex.xyz;` in `SpaceMaterialSystem.cpp`, although shader source was absent from the R3 payload. The Python companion imposed the same misplaced requirement. The corrected scripts verify the scope R3 actually owns: Studio calls the overlay program guard, the gizmo projection/Socket conditional remains active, and the guard both suspends and restores the GL program. It is not a bypass of an observed shader regression: shader source is not altered, and shader compilation/visual checks remain independent gates and hands-on acceptance.

Prerequisite: R3 `null-harbor-foundation-widget-shader-guard-r3-20260920` applied, with the original R3 static gate and Python test hashes unchanged. Drop this archive at project root and explicitly approve via project PCC. Run `python tools/studio/tests/test_studio_foundation_r3.py --root .` and PCC option 1 Full Quality Gate. No forced apply, no manual extraction. If green, still test real widget visibility/picking at multiple camera angles and shader/overlay appearance before commit and push.

No Null Harbor identity migration or broad renderer/GUI refactor is included.
