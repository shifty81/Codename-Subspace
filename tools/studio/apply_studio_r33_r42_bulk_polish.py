#!/usr/bin/env python3
"""Guarded R33-R42 Studio source normalization.

Runs only after the R32 Construct migration. All edits are exact-preimage,
idempotent, transactional, and backed up under artifacts/gates/migrations.
"""
from __future__ import annotations
import argparse, hashlib, json, os, shutil, sys, tempfile
from datetime import datetime, timezone
from pathlib import Path
from studio_verified_normalized_state import verify_complete_r82r1


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def replace_exact(text: str, old: str, new: str, label: str) -> tuple[str, bool]:
    if new in text:
        return text, False
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly one approved preimage, found {count}")
    return text.replace(old, new, 1), True


def assert_contains(text: str, token: str, label: str) -> None:
    if token not in text:
        raise RuntimeError(f"{label}: required token missing after normalization: {token}")


def assert_absent(text: str, token: str, label: str) -> None:
    if token in text:
        raise RuntimeError(f"{label}: retired token remains after normalization: {token}")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", required=True)
    ap.add_argument("--apply", action="store_true")
    args = ap.parse_args()
    root = Path(args.root).resolve()
    # An R53+ migration extends R33 postimages; do not replay obsolete exact
    # preimages if the full R82R1 state is independently source-gate proven.
    if verify_complete_r82r1(root):
        print("r33_r42_bulk_polish: complete R82R1 normalized source verified; historical migration no-op.")
        return 0

    files = {
        "builder": root / "engine/src/ship_editor/ShipyardBuilderSystem.cpp",
        "app": root / "engine/src/studio/StudioApplication.cpp",
        "gizmo": root / "engine/include/studio/StudioAxisGizmo.h",
        "overlay": root / "engine/src/studio/StudioGizmoOverlay.cpp",
        "workspace": root / "engine/src/ship_editor/ShipyardWorkspaceSystem.cpp",
    }
    for key, path in files.items():
        if not path.is_file():
            raise RuntimeError(f"R33-R42 missing source {key}: {path}")

    original = {k: p.read_text(encoding="utf-8") for k, p in files.items()}
    # Fail closed if the preceding public-workspace normalization is absent.
    assert_contains(original["builder"], "ConstructionScalePivotSystem.h", "R32 precondition")
    assert_contains(original["workspace"], 'case ShipyardWorkspaceMode::Build:return"CONSTRUCT";', "R32 precondition")
    assert_contains(original["workspace"], 'case ShipyardWorkspaceMode::Model:return"GEOMETRY";', "R32 precondition")

    changed: dict[str, str] = dict(original)
    touched: list[str] = []

    def edit(key: str, old: str, new: str, label: str) -> None:
        nonlocal changed, touched
        next_text, did = replace_exact(changed[key], old, new, label)
        changed[key] = next_text
        if did and key not in touched:
            touched.append(key)

    # R33: adaptive readout placement. Keep the current render/pick snapshot
    # and only add presentation coordinates to it.
    edit("app",
         '#include "studio/StudioGuiInteractionPolicy.h"\n#include "studio/StudioGizmoProjectionPolicy.h"',
         '#include "studio/StudioGuiInteractionPolicy.h"\n#include "studio/StudioOverlayPlacementPolicy.h"\n#include "studio/StudioGizmoProjectionPolicy.h"',
         "Studio adaptive overlay include")
    edit("gizmo",
         '    bool readoutVisible=false;\n    StudioTransformReadout readout{};',
         '    bool readoutVisible=false;\n    float readoutLeft=0.0f,readoutTop=0.0f;\n    StudioTransformReadout readout{};',
         "Studio readout placement fields")
    edit("overlay",
         '    const float x=snapshot.viewportLeft+9.0f,y=snapshot.viewportTop+9.0f;',
         '    const float x=snapshot.readoutLeft,y=snapshot.readoutTop;',
         "Studio transform HUD origin")
    old_hud = '''        // The native HUD currently measures 435x106px. Check its ENTIRE area\n        // against real floating geometry: point sampling missed thin panels.\n        const SubspaceUiRect hud{gizmo.viewportLeft+9.0f,gizmo.viewportTop+9.0f,435.0f,106.0f};\n        gizmo.readoutVisible=StudioGuiInteractionPolicy::ClearOverlayArea(layers,hud);'''
    new_hud = '''        // Prefer a clear viewport corner instead of simply hiding the readout\n        // whenever a floating panel occupies the historical top-left slot.\n        const auto placement=StudioOverlayPlacementPolicy::Choose(layers,\n            gizmo.viewportLeft,gizmo.viewportTop,gizmo.viewportRight,gizmo.viewportBottom);\n        gizmo.readoutVisible=placement.visible;\n        gizmo.readoutLeft=placement.rect.x;gizmo.readoutTop=placement.rect.y;'''
    edit("app", old_hud, new_hud, "Studio adaptive transform HUD block")

    # R36/R37: preserve command identities and semantic ship vocabulary but
    # make visible manipulation controls use the same context-neutral ruler as
    # the gizmo/readout and R32 Construct contract.
    visible_replacements = [
        ('{ShipyardBuilderCommand::NudgePort,"PORT",false,hasTransformSubject}',
         '{ShipyardBuilderCommand::NudgePort,"X -",false,hasTransformSubject}', "nudge -X"),
        ('{ShipyardBuilderCommand::NudgeStarboard,"STARBOARD",false,hasTransformSubject}',
         '{ShipyardBuilderCommand::NudgeStarboard,"X +",false,hasTransformSubject}', "nudge +X"),
        ('{ShipyardBuilderCommand::NudgeForward,"FORWARD",false,hasTransformSubject}',
         '{ShipyardBuilderCommand::NudgeForward,"Y +",false,hasTransformSubject}', "nudge +Y"),
        ('{ShipyardBuilderCommand::NudgeAft,"AFT",false,hasTransformSubject}',
         '{ShipyardBuilderCommand::NudgeAft,"Y -",false,hasTransformSubject}', "nudge -Y"),
        ('{ShipyardBuilderCommand::NudgeDorsal,"UP",false,hasTransformSubject}',
         '{ShipyardBuilderCommand::NudgeDorsal,"Z +",false,hasTransformSubject}', "nudge +Z"),
        ('{ShipyardBuilderCommand::NudgeVentral,"DOWN",false,hasTransformSubject}',
         '{ShipyardBuilderCommand::NudgeVentral,"Z -",false,hasTransformSubject}', "nudge -Z"),
        ('{ShipyardBuilderCommand::SymmetryAxisPortStarboard,"PORT <-> STARBOARD",model.symmetryFrame.axis==ConstructionSymmetryAxis::PortStarboard,true}',
         '{ShipyardBuilderCommand::SymmetryAxisPortStarboard,"MIRROR X",model.symmetryFrame.axis==ConstructionSymmetryAxis::PortStarboard,true}', "mirror X"),
        ('{ShipyardBuilderCommand::SymmetryAxisForeAft,"FORE <-> AFT",model.symmetryFrame.axis==ConstructionSymmetryAxis::ForeAft,true}',
         '{ShipyardBuilderCommand::SymmetryAxisForeAft,"MIRROR Y",model.symmetryFrame.axis==ConstructionSymmetryAxis::ForeAft,true}', "mirror Y"),
        ('{ShipyardBuilderCommand::SymmetryAxisDorsalVentral,"DORSAL <-> VENTRAL",model.symmetryFrame.axis==ConstructionSymmetryAxis::DorsalVentral,true}',
         '{ShipyardBuilderCommand::SymmetryAxisDorsalVentral,"MIRROR Z",model.symmetryFrame.axis==ConstructionSymmetryAxis::DorsalVentral,true}', "mirror Z"),
        ('model_.status="Symmetry plane: PORT <-> STARBOARD";',
         'model_.status="Symmetry plane: MIRROR X / WIDTH";', "symmetry X status"),
        ('model_.status="Symmetry plane: FORE <-> AFT";',
         'model_.status="Symmetry plane: MIRROR Y / LENGTH";', "symmetry Y status"),
        ('model_.status="Symmetry plane: DORSAL <-> VENTRAL";',
         'model_.status="Symmetry plane: MIRROR Z / HEIGHT";', "symmetry Z status"),
        ('model_.status="Reflected selected module/subassembly in place across PORT <-> STARBOARD";',
         'model_.status="Reflected selected module/subassembly in place across MIRROR X / WIDTH";', "mirror-copy status"),
    ]
    for old, new, label in visible_replacements:
        edit("builder", old, new, label)

    # A generic Construct title matches the one-workspace presentation while
    # preserving all ship-specific project/runtime semantics underneath.
    edit("app", 'config.title="Null Harbor Studio - Ship Authoring";',
         'config.title="Null Harbor Studio - Construct";', "Studio Construct title")

    # Postconditions: no ambiguous visible controls, no overlay-through-panel
    # regression, and no accidental rollback of R32 authority.
    for token in ('"X -"','"X +"','"Y +"','"Y -"','"Z +"','"Z -"','"MIRROR X"','"MIRROR Y"','"MIRROR Z"'):
        assert_contains(changed["builder"], token, "generic manipulation labels")
    assert_absent(changed["builder"], 'ShipyardBuilderCommand::NudgeStarboard,"STARBOARD"', "generic manipulation labels")
    assert_absent(changed["builder"], 'ShipyardBuilderCommand::SymmetryAxisPortStarboard,"PORT <-> STARBOARD"', "generic symmetry labels")
    assert_contains(changed["app"], "StudioOverlayPlacementPolicy::Choose", "adaptive HUD")
    assert_contains(changed["gizmo"], "readoutLeft", "adaptive HUD")
    assert_contains(changed["overlay"], "snapshot.readoutLeft", "adaptive HUD")

    if not args.apply:
        print("R33-R42 DRY RUN PASS; files requiring normalization:", ", ".join(touched) if touched else "none")
        return 0
    if not touched:
        print("R33-R42 source normalization already present; no mutation required.")
        return 0

    stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    backup_root = root / "artifacts/gates/migrations/studio_r33_r42" / stamp
    backup_root.mkdir(parents=True, exist_ok=False)
    receipt = {"schema":"subspace.studio-r33-r42-migration.v1","timestampUtc":stamp,"files":[]}

    # Back up every to-be-written file before any replacement occurs.
    for key in touched:
        path = files[key]
        rel = path.relative_to(root)
        dst = backup_root / rel
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(path, dst)
        receipt["files"].append({"path":rel.as_posix(),"beforeSha256":sha256(path.read_bytes())})

    written: list[Path] = []
    try:
        for key in touched:
            path = files[key]
            tmp = path.with_name(path.name + ".r33r42.tmp")
            tmp.write_text(changed[key], encoding="utf-8", newline="\n")
            os.replace(tmp, path)
            written.append(path)
        for item in receipt["files"]:
            item["afterSha256"] = sha256((root / item["path"]).read_bytes())
        (backup_root / "receipt.json").write_text(json.dumps(receipt, indent=2)+"\n", encoding="utf-8")
    except Exception:
        # Fail closed and restore the backed-up preimage if any write fails.
        for item in receipt["files"]:
            rel = Path(item["path"])
            backup = backup_root / rel
            if backup.exists():
                shutil.copy2(backup, root / rel)
        raise

    print(f"R33-R42 source normalization applied transactionally to {len(touched)} file(s).")
    print(f"Backup/receipt: {backup_root}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:
        print(f"R33-R42 NORMALIZATION BLOCKED: {exc}", file=sys.stderr)
        raise SystemExit(2)
