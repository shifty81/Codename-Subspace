#!/usr/bin/env python3
"""Transactional R23-R32 Studio Construct cutover.

This tranche deliberately absorbs the deferred R13-R22 source migration into
one mandatory, fail-closed transaction for certified head 2069763.  It then
adds the next normalization layer: context-neutral transform-space wording,
a single-row DEV workspace strip, responsive Asset Browser actions, and a
source state that ProjectOps static certification can prove is actually live.
"""
from __future__ import annotations

import argparse
from datetime import datetime, timezone
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

MIGRATION_ID = "subspace-studio-r23-r32-normalization-v1"
EXPECTED_HEAD = "206976365c3bc7364f06315a086d9bf31753b50f"
TARGET_FILES = (
    "engine/src/ship_editor/ShipyardBuilderSystem.cpp",
    "engine/src/ship_editor/ShipyardProfessionalVisibleCutover.cpp",
    "engine/src/ship_editor/ShipyardWorkspaceSystem.cpp",
    "engine/src/modeling/ShipyardModelingSystem.cpp",
)

HERE = Path(__file__).resolve().parent
R22_PATH = HERE / "apply_studio_r13_r22_normalization.py"
if not R22_PATH.is_file():
    raise SystemExit("R23-R32 requires the certified R22 migration helper")
spec = importlib.util.spec_from_file_location("subspace_r22_migration", R22_PATH)
r22 = importlib.util.module_from_spec(spec)
assert spec and spec.loader
spec.loader.exec_module(r22)


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def sha256_path(path: Path) -> str:
    return sha256_bytes(path.read_bytes())


def git_head(root: Path) -> str | None:
    try:
        return subprocess.check_output(
            ["git", "-C", str(root), "rev-parse", "HEAD"],
            text=True, stderr=subprocess.DEVNULL).strip()
    except Exception:
        return None


def replace_once(text: str, old: str, new: str, label: str) -> tuple[str, str]:
    old_count = text.count(old)
    new_count = text.count(new)
    if old_count == 1 and new_count == 0:
        return text.replace(old, new, 1), "APPLY"
    if old_count == 0 and new_count == 1:
        return text, "PRESENT"
    raise RuntimeError(
        f"{label}: expected one old anchor or one already-applied anchor "
        f"(old={old_count}, new={new_count})")


def replace_all_known(text: str, old: str, new: str, expected: int, label: str) -> tuple[str, str]:
    old_count = text.count(old)
    new_count = text.count(new)
    if old_count == expected and new_count == 0:
        return text.replace(old, new), "APPLY"
    if old_count == 0 and new_count == expected:
        return text, "PRESENT"
    raise RuntimeError(
        f"{label}: expected {expected} old anchors or {expected} applied anchors "
        f"(old={old_count}, new={new_count})")


def extra_builder(text: str) -> tuple[str, list[dict]]:
    log: list[dict] = []

    old = 'model.transformSpace==ShipyardTransformSpace::View?"CAMERA":(model.transformSpace==ShipyardTransformSpace::Ship?"SHIP":"LOCAL")'
    new = 'model.transformSpace==ShipyardTransformSpace::View?"VIEW":(model.transformSpace==ShipyardTransformSpace::Ship?"PARENT":"OBJECT")'
    text, state = replace_all_known(text, old, new, 2, "generic transform-space controls")
    log.append({"edit": "generic transform-space controls", "state": state})

    old = 'model_.transformSpace==ShipyardTransformSpace::View?"TRANSFORM SPACE: CAMERA":(model_.transformSpace==ShipyardTransformSpace::Ship?"TRANSFORM SPACE: SHIP":"TRANSFORM SPACE: LOCAL")'
    new = 'model_.transformSpace==ShipyardTransformSpace::View?"TRANSFORM SPACE: VIEW":(model_.transformSpace==ShipyardTransformSpace::Ship?"TRANSFORM SPACE: PARENT":"TRANSFORM SPACE: OBJECT")'
    text, state = replace_once(text, old, new, "generic transform-space status")
    log.append({"edit": "generic transform-space status", "state": state})
    return text, log


def extra_visible(text: str) -> tuple[str, list[dict]]:
    log: list[dict] = []

    include_old = '#include "editor/EditorDccShellLayoutSystem.h"'
    include_new = include_old + '\n#include "editor/ConstructionUiLayoutPolicy.h"'
    text, state = replace_once(text, include_old, include_new, "responsive layout policy include")
    log.append({"edit": "responsive layout policy include", "state": state})

    start_marker = "    // First-class authoring flow: Construct owns assembly + geometry submodes,"
    end_marker = "\n\n    if(maximized){"
    applied_marker = "    // R25-R27: one authoritative workspace strip; DEV replaces the row instead of stacking over the viewport."
    if start_marker in text:
        start = text.index(start_marker)
        end = text.index(end_marker, start)
        replacement = '''    // R25-R27: one authoritative workspace strip; DEV replaces the row instead of stacking over the viewport.
    const float stripX=8.0f*s;
    const float stripW=std::max(1.0f,static_cast<float>(w)-16.0f*s);
    const float stripGap=2.0f*s;
    if(!model.developerWorkspacesVisible){
        struct PrimaryTab { ShipyardBuilderCommand command; const char* label; bool active; bool enabled; };
        const PrimaryTab tabs[]={
            {ShipyardBuilderCommand::WorkspaceBuild,"CONSTRUCT",!model.testWorkspaceActive&&(model.workspaceMode==ShipyardWorkspaceMode::Build||model.workspaceMode==ShipyardWorkspaceMode::Model),true},
            {ShipyardBuilderCommand::WorkspaceInterior,"INTERIOR",!model.testWorkspaceActive&&model.workspaceMode==ShipyardWorkspaceMode::Interior,model.capabilities.interior},
            {ShipyardBuilderCommand::WorkspaceSystems,"SYSTEMS",!model.testWorkspaceActive&&model.workspaceMode==ShipyardWorkspaceMode::Systems,true},
            {ShipyardBuilderCommand::WorkspaceAppearance,"APPEARANCE",!model.testWorkspaceActive&&model.workspaceMode==ShipyardWorkspaceMode::Appearance,true},
            {ShipyardBuilderCommand::WorkspaceDevWorld,"TEST",model.testWorkspaceActive,true}
        };
        const std::size_t count=sizeof(tabs)/sizeof(tabs[0]);
        const float devW=64.0f*s;
        const float primaryW=std::max(1.0f,stripW-devW-stripGap);
        const float tabW=ConstructionUiLayoutPolicy::TabWidth(primaryW,count,stripGap);
        float x=stripX;
        for(const auto& t:tabs){add(t.command,0,x,l.workspaceBarY,tabW,l.workspaceBarHeight,t.label,t.active,t.enabled);x+=tabW+stripGap;}
        add(ShipyardBuilderCommand::WorkspaceAuthoring,-1268,stripX+primaryW+stripGap,l.workspaceBarY,devW,l.workspaceBarHeight,"DEV",false,true);
    }else{
        struct DevTab { ShipyardBuilderCommand command; const char* label; bool active; bool enabled; int value; };
        const DevTab tabs[]={
            {ShipyardBuilderCommand::WorkspaceAuthoring,"BACK",true,true,-1268},
            {ShipyardBuilderCommand::InspectorSockets,"SOCKETS",model.inspectorTab==ShipyardInspectorTab::Sockets,model.capabilities.sockets,0},
            {ShipyardBuilderCommand::WorkspacePcg,"PCG",model.workspaceMode==ShipyardWorkspaceMode::Pcg,model.capabilities.pcgStudio,0},
            {ShipyardBuilderCommand::WorkspaceWorld,"WORLD",model.workspaceMode==ShipyardWorkspaceMode::World,model.capabilities.world,0},
            {ShipyardBuilderCommand::WorkspaceCharacter,"CHAR",model.workspaceMode==ShipyardWorkspaceMode::Character,model.capabilities.character,0},
            {ShipyardBuilderCommand::WorkspaceDevWorld,"DEV WORLD",model.workspaceMode==ShipyardWorkspaceMode::DevWorld,model.capabilities.devWorld,0},
            {ShipyardBuilderCommand::WorkspaceProjectTools,"PROJECT",model.workspaceMode==ShipyardWorkspaceMode::ProjectTools,true,0},
            {ShipyardBuilderCommand::WorkspaceAuthoring,"AUTHOR",model.workspaceMode==ShipyardWorkspaceMode::Authoring,model.capabilities.rawAuthoring,0}
        };
        const std::size_t count=sizeof(tabs)/sizeof(tabs[0]);
        const float tabW=ConstructionUiLayoutPolicy::TabWidth(stripW,count,stripGap);
        float x=stripX;
        for(const auto& t:tabs){add(t.command,t.value,x,l.workspaceBarY,tabW,l.workspaceBarHeight,t.label,t.active,t.enabled);x+=tabW+stripGap;}
    }'''
        text = text[:start] + replacement + text[end:]
        log.append({"edit": "single-row primary/dev workspace strip", "state": "APPLY"})
    elif applied_marker in text:
        log.append({"edit": "single-row primary/dev workspace strip", "state": "PRESENT"})
    else:
        raise RuntimeError("single-row primary/dev workspace strip: expected R22 Construct strip anchor")

    old_actions = '''        add(ShipyardBuilderCommand::DccPreviousAssetPreset,0,ax+72*s,actionY,24*s,24*s,"<",false,true);
        add(ShipyardBuilderCommand::DccNextAssetPreset,0,ax+99*s,actionY,144*s,24*s,ShipyardDccUiSystem::AssetPresetName(model.dcc.assetPreset),true,true);
        add(ShipyardBuilderCommand::DccCycleAssetDensity,0,ax+246*s,actionY,82*s,24*s,ShipyardDccUiSystem::AssetDensityName(model.dcc.assetBrowser.density),false,true);
        add(ShipyardBuilderCommand::DccToggleFavoriteSelected,0,ax+aw-270*s,actionY,52*s,24*s,"FAV",false,!model.catalog.empty());
        add(ShipyardBuilderCommand::DccClearAssetFilters,0,ax+aw-215*s,actionY,54*s,24*s,"CLEAR",false,true);
        add(ShipyardBuilderCommand::AddModule,0,ax+aw-158*s,actionY,72*s,24*s,"PLACE",false,!model.catalog.empty());
        add(ShipyardBuilderCommand::Validate,0,ax+aw-83*s,actionY,76*s,24*s,"CHECK",false,true);'''
    new_actions = '''        add(ShipyardBuilderCommand::DccPreviousAssetPreset,0,ax+72*s,actionY,24*s,24*s,"<",false,true);
        add(ShipyardBuilderCommand::DccNextAssetPreset,0,ax+99*s,actionY,144*s,24*s,ShipyardDccUiSystem::AssetPresetName(model.dcc.assetPreset),true,true);
        if(ConstructionUiLayoutPolicy::ShowAssetDensity(aw,s))
            add(ShipyardBuilderCommand::DccCycleAssetDensity,0,ax+246*s,actionY,82*s,24*s,ShipyardDccUiSystem::AssetDensityName(model.dcc.assetBrowser.density),false,true);
        if(ConstructionUiLayoutPolicy::ShowAssetSecondaryActions(aw,s)){
            add(ShipyardBuilderCommand::DccToggleFavoriteSelected,0,ax+aw-270*s,actionY,52*s,24*s,"FAV",false,!model.catalog.empty());
            add(ShipyardBuilderCommand::DccClearAssetFilters,0,ax+aw-215*s,actionY,54*s,24*s,"CLEAR",false,true);
        }
        add(ShipyardBuilderCommand::AddModule,0,ax+aw-158*s,actionY,72*s,24*s,"PLACE",false,!model.catalog.empty());
        add(ShipyardBuilderCommand::Validate,0,ax+aw-83*s,actionY,76*s,24*s,"CHECK",false,true);'''
    text, state = replace_once(text, old_actions, new_actions, "responsive asset action row")
    log.append({"edit": "responsive asset action row", "state": state})
    return text, log


def extra_workspace(text: str) -> tuple[str, list[dict]]:
    log: list[dict] = []
    old = 'ShipyardWorkspaceSystem::DeveloperWorkspaces(){return {ShipyardWorkspaceMode::Model,ShipyardWorkspaceMode::Character,ShipyardWorkspaceMode::Pcg,ShipyardWorkspaceMode::World,ShipyardWorkspaceMode::DevWorld,ShipyardWorkspaceMode::ProjectTools,ShipyardWorkspaceMode::Authoring};}'
    new = 'ShipyardWorkspaceSystem::DeveloperWorkspaces(){return {ShipyardWorkspaceMode::Character,ShipyardWorkspaceMode::Pcg,ShipyardWorkspaceMode::World,ShipyardWorkspaceMode::DevWorld,ShipyardWorkspaceMode::ProjectTools,ShipyardWorkspaceMode::Authoring};}'
    text, state = replace_once(text, old, new, "geometry removed from developer workspace list")
    log.append({"edit": "geometry removed from developer workspace list", "state": state})

    old = '"View-space movement is default; Ship/Local are explicit alternatives."'
    new = '"View-space movement is default; Parent/Object are explicit alternatives."'
    text, state = replace_once(text, old, new, "generic transform-space help")
    log.append({"edit": "generic transform-space help", "state": state})
    return text, log


def extra_modeling(text: str) -> tuple[str, list[dict]]:
    # R22 already fixes the concrete +Y-forward wedge. R23-R32 intentionally
    # preserve that geometry and add no second primitive authority.
    return text, [{"edit": "preserve +Y-forward modeling authority", "state": "PRESENT"}]


EXTRA = {
    TARGET_FILES[0]: extra_builder,
    TARGET_FILES[1]: extra_visible,
    TARGET_FILES[2]: extra_workspace,
    TARGET_FILES[3]: extra_modeling,
}


def prepare(root: Path):
    head = git_head(root)
    if head != EXPECTED_HEAD:
        raise RuntimeError(
            f"target commit mismatch; expected {EXPECTED_HEAD}, got {head or 'NO_GIT_HEAD'}. "
            "Refresh the patch for a newer certified baseline instead of forcing it.")

    base_prepared, base_edits = r22.prepare(root)
    prepared = {}
    edits = list(base_edits)
    for rel in TARGET_FILES:
        item = base_prepared[rel]
        text = item["after"].decode("utf-8")
        final_text, extra_log = EXTRA[rel](text)
        after = final_text.encode("utf-8")
        prepared[rel] = {
            "before": item["before"],
            "after": after,
            "changed": after != item["before"],
            "beforeSha256": sha256_bytes(item["before"]),
            "afterSha256": sha256_bytes(after),
        }
        edits.extend({"file": rel, **e} for e in extra_log)
    return prepared, edits


def write_atomic(path: Path, data: bytes):
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, tmp_name = tempfile.mkstemp(prefix=path.name+".", suffix=".r32.tmp", dir=str(path.parent))
    try:
        with os.fdopen(fd, "wb") as f:
            f.write(data); f.flush(); os.fsync(f.fileno())
        os.replace(tmp_name, path)
    finally:
        if os.path.exists(tmp_name):
            os.unlink(tmp_name)


def apply(root: Path) -> Path | None:
    prepared, edits = prepare(root)
    changed = [rel for rel, item in prepared.items() if item["changed"]]
    if not changed:
        print("R23-R32 Studio normalization already present; no writes required.")
        return None

    stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    recovery = root / ".subspace" / "recovery" / f"studio-r32-{stamp}"
    receipts = root / ".subspace" / "receipts"
    recovery.mkdir(parents=True, exist_ok=False)
    receipts.mkdir(parents=True, exist_ok=True)
    files = []
    try:
        for rel in changed:
            src = root / rel
            dst = recovery / rel
            dst.parent.mkdir(parents=True, exist_ok=True)
            dst.write_bytes(prepared[rel]["before"])
        for rel in changed:
            write_atomic(root / rel, prepared[rel]["after"])
        for rel in changed:
            actual = sha256_path(root / rel)
            if actual != prepared[rel]["afterSha256"]:
                raise RuntimeError(f"post-write hash mismatch: {rel}")
            files.append({
                "path": rel,
                "beforeSha256": prepared[rel]["beforeSha256"],
                "afterSha256": prepared[rel]["afterSha256"],
                "backup": str((recovery / rel).relative_to(root)).replace('\\','/'),
            })
    except Exception:
        for rel in changed:
            backup = recovery / rel
            if backup.is_file():
                write_atomic(root / rel, backup.read_bytes())
        raise

    receipt = {
        "schema": "subspace.studio-source-migration.v1",
        "migrationId": MIGRATION_ID,
        "createdUtc": datetime.now(timezone.utc).isoformat(),
        "gitHead": git_head(root),
        "recoveryRoot": str(recovery.relative_to(root)).replace('\\','/'),
        "files": files,
        "edits": edits,
        "status": "APPLIED",
    }
    receipt_path = receipts / f"studio-r32-{stamp}.json"
    write_atomic(receipt_path, (json.dumps(receipt, indent=2, sort_keys=True)+"\n").encode())
    print(f"R23-R32 source migration APPLIED: {len(files)} file(s)")
    print(f"Receipt: {receipt_path}")
    return receipt_path


def rollback(root: Path, receipt_path: Path):
    receipt = json.loads(receipt_path.read_text(encoding="utf-8"))
    if receipt.get("migrationId") != MIGRATION_ID:
        raise RuntimeError("receipt is not an R23-R32 migration receipt")
    for item in receipt.get("files", []):
        target = root / item["path"]
        backup = root / item["backup"]
        if not backup.is_file():
            raise RuntimeError(f"missing backup: {backup}")
        current = sha256_path(target) if target.is_file() else None
        if current != item["afterSha256"]:
            raise RuntimeError(f"rollback blocked by post-migration edits: {item['path']}")
    for item in receipt.get("files", []):
        write_atomic(root / item["path"], (root / item["backup"]).read_bytes())
    print("R23-R32 rollback complete. Recovery evidence retained.")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", type=Path, default=Path.cwd())
    group = ap.add_mutually_exclusive_group(required=True)
    group.add_argument("--check", action="store_true")
    group.add_argument("--apply", action="store_true")
    group.add_argument("--rollback", type=Path)
    args = ap.parse_args()
    root = args.root.resolve()
    try:
        if args.rollback:
            rollback(root, args.rollback.resolve()); return 0
        prepared, edits = prepare(root)
        pending = [k for k,v in prepared.items() if v["changed"]]
        if args.check:
            print(f"R23-R32 preflight PASS: {len(edits)} contract edit(s); {len(pending)} file(s) pending")
            for rel in pending: print(f"  PENDING {rel}")
            return 0
        apply(root); return 0
    except Exception as exc:
        print(f"R23-R32 migration BLOCKED: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
