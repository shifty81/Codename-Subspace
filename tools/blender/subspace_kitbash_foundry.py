#!/usr/bin/env python3
"""Subspace Kitbash Foundry front-end.

This is the normalized Blender lane introduced by R21.  It does NOT import or
execute archived donor Python.  Donor inventory is read-only evidence used to
classify capabilities.  New geometry is described by a small deterministic
recipe and compiled into a standalone Blender worker script.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import re
import sys

RECIPE_SCHEMA = "subspace.kitbash-foundry.recipe.v1"
CAP_SCHEMA = "subspace.kitbash-foundry.capabilities.v1"
WORKER_SCHEMA = "subspace.kitbash-foundry.worker.v1"
ALLOWED_PRIMITIVES = {"BOX", "CYLINDER", "WEDGE", "HULL_SEGMENT"}


def atomic_write(path: Path, data: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    tmp = path.with_name(path.name + ".tmp")
    tmp.write_text(data, encoding="utf-8", newline="\n")
    tmp.replace(path)


def load_json(path: Path):
    return json.loads(path.read_text(encoding="utf-8"))


def vec3(value, name):
    if not isinstance(value, list) or len(value) != 3 or not all(isinstance(v, (int, float)) for v in value):
        raise ValueError(f"{name} must be a numeric vec3")
    return [float(v) for v in value]


def validate_recipe(doc: dict) -> dict:
    if not isinstance(doc, dict) or doc.get("schema") != RECIPE_SCHEMA:
        raise ValueError(f"expected {RECIPE_SCHEMA}")
    asset_id = doc.get("assetId")
    if not isinstance(asset_id, str) or not re.fullmatch(r"[A-Za-z0-9_.-]{1,96}", asset_id):
        raise ValueError("assetId must be a safe stable identifier")
    parts = doc.get("parts")
    if not isinstance(parts, list) or not 1 <= len(parts) <= 512:
        raise ValueError("parts must contain 1..512 entries")
    seen = set()
    normalized = []
    for i, part in enumerate(parts):
        if not isinstance(part, dict):
            raise ValueError(f"parts[{i}] must be an object")
        pid = part.get("id")
        if not isinstance(pid, str) or not re.fullmatch(r"[A-Za-z0-9_.-]{1,96}", pid) or pid in seen:
            raise ValueError(f"parts[{i}].id invalid or duplicate")
        seen.add(pid)
        primitive = str(part.get("primitive", "")).upper()
        if primitive not in ALLOWED_PRIMITIVES:
            raise ValueError(f"parts[{i}].primitive unsupported: {primitive}")
        size = vec3(part.get("size"), f"parts[{i}].size")
        if any(v <= 0 or v > 100000 for v in size):
            raise ValueError(f"parts[{i}].size out of range")
        normalized.append({
            "id": pid,
            "primitive": primitive,
            "size": size,
            "position": vec3(part.get("position", [0, 0, 0]), f"parts[{i}].position"),
            "rotationDeg": vec3(part.get("rotationDeg", [0, 0, 0]), f"parts[{i}].rotationDeg"),
            "role": str(part.get("role", "STRUCTURE"))[:64],
            "materialZone": str(part.get("materialZone", "HULL_PRIMARY"))[:64],
        })
    sockets = doc.get("sockets", [])
    if not isinstance(sockets, list) or len(sockets) > 2048:
        raise ValueError("sockets must be a list with <=2048 entries")
    nsockets = []
    socket_ids = set()
    for i, socket in enumerate(sockets):
        if not isinstance(socket, dict):
            raise ValueError(f"sockets[{i}] must be an object")
        sid, part_id = socket.get("id"), socket.get("partId")
        if not isinstance(sid, str) or not re.fullmatch(r"[A-Za-z0-9_.-]{1,96}", sid) or sid in socket_ids:
            raise ValueError(f"sockets[{i}].id invalid or duplicate")
        if part_id not in seen:
            raise ValueError(f"sockets[{i}].partId does not reference a known part")
        socket_ids.add(sid)
        nsockets.append({
            "id": sid,
            "partId": part_id,
            "position": vec3(socket.get("position", [0, 0, 0]), f"sockets[{i}].position"),
            "forward": vec3(socket.get("forward", [0, 1, 0]), f"sockets[{i}].forward"),
            "up": vec3(socket.get("up", [0, 0, 1]), f"sockets[{i}].up"),
            "kind": str(socket.get("kind", "STRUCTURAL"))[:64],
        })
    result = {
        "schema": RECIPE_SCHEMA,
        "assetId": asset_id,
        "parts": normalized,
        "sockets": nsockets,
        "metadata": doc.get("metadata", {}) if isinstance(doc.get("metadata", {}), dict) else {},
    }
    canonical = json.dumps(result, sort_keys=True, separators=(",", ":")).encode()
    result["recipeSha256"] = hashlib.sha256(canonical).hexdigest()
    return result


def classify_donor(path: str) -> list[str]:
    p = path.lower()
    capabilities = []
    rules = (
        ("shipfoundry", "SHIP_GENERATION"),
        ("shipyardpipeline", "ASSEMBLY_PIPELINE"),
        ("kitbash", "KITBASH_GEOMETRY"),
        ("lineage", "EXTERIOR_INTERIOR_LINEAGE"),
        ("interior", "INTERIOR_GENERATION"),
        ("socket", "SOCKET_AUTHORING"),
        ("material", "MATERIALS"),
        ("export", "EXPORT"),
        ("gltf", "EXPORT"),
        ("glb", "EXPORT"),
        ("pcg", "PROCEDURAL_RECIPE"),
        ("procedural", "PROCEDURAL_RECIPE"),
        ("rig", "CHARACTER_RIG"),
        ("animation", "ANIMATION"),
    )
    for needle, cap in rules:
        if needle in p and cap not in capabilities:
            capabilities.append(cap)
    if not capabilities:
        capabilities.append("UNCLASSIFIED")
    return capabilities


def capability_registry(inventory: dict) -> dict:
    donors = inventory.get("donors") if isinstance(inventory, dict) else None
    if not isinstance(donors, list):
        raise ValueError("donor inventory missing donors list")
    caps: dict[str, list[dict]] = {}
    for donor in donors:
        if not isinstance(donor, dict) or not isinstance(donor.get("sourcePath"), str):
            continue
        evidence = {
            "sourcePath": donor["sourcePath"],
            "sha256": donor.get("sha256"),
            "status": donor.get("status", "REVIEW_BEFORE_PORT"),
            "category": donor.get("category", "unknown"),
        }
        for cap in classify_donor(donor["sourcePath"]):
            caps.setdefault(cap, []).append(evidence)
    return {
        "schema": CAP_SCHEMA,
        "authority": "READ_ONLY_DONOR_EVIDENCE",
        "executionPolicy": "DONOR_CODE_NEVER_EXECUTED_DIRECTLY",
        "donorCount": len(donors),
        "capabilities": {k: {"count": len(v), "evidence": v} for k, v in sorted(caps.items())},
    }


def emit_worker(recipe: dict) -> str:
    r = validate_recipe(recipe)
    payload = json.dumps(r, sort_keys=True, separators=(",", ":"))
    # The emitted script deliberately owns all bpy calls.  Donor scripts are
    # never imported.  +Y is forward, +Z is up, +X is width/right.
    return f'''# Generated by Subspace Kitbash Foundry. DO NOT EDIT GENERATED OUTPUT.\n# schema: {WORKER_SCHEMA}\nimport bpy, json, math, os, sys\nfrom mathutils import Vector\nRECIPE = json.loads({payload!r})\n\ndef clear_scene():\n    bpy.ops.object.select_all(action='SELECT')\n    bpy.ops.object.delete(use_global=False)\n\ndef add_wedge(name, size):\n    x,y,z=[v*.5 for v in size]\n    verts=[(-x,y,0),(x,y,0),(x,-y,-z),(-x,-y,-z),(x,-y,z),(-x,-y,z)]\n    faces=[(0,1,2,3),(5,4,1,0),(3,2,4,5),(0,3,5),(1,4,2)]\n    mesh=bpy.data.meshes.new(name+'.mesh'); mesh.from_pydata(verts,[],faces); mesh.update()\n    obj=bpy.data.objects.new(name,mesh); bpy.context.collection.objects.link(obj); return obj\n\ndef add_hull(name, size):\n    x,y,z=[v*.5 for v in size]\n    verts=[(-x,-y,-z),(x,-y,-z),(x,-y,z),(-x,-y,z),(-x*.68,y,-z*.85),(x*.68,y,-z*.85),(x*.68,y,z*.85),(-x*.68,y,z*.85)]\n    faces=[(0,1,2,3),(4,7,6,5),(0,4,5,1),(3,2,6,7),(0,3,7,4),(1,5,6,2)]\n    mesh=bpy.data.meshes.new(name+'.mesh'); mesh.from_pydata(verts,[],faces); mesh.update()\n    obj=bpy.data.objects.new(name,mesh); bpy.context.collection.objects.link(obj); return obj\n\ndef add_part(part):\n    kind=part['primitive']; size=part['size']; name=part['id']\n    if kind=='BOX':\n        bpy.ops.mesh.primitive_cube_add(size=1); obj=bpy.context.object; obj.name=name; obj.dimensions=size\n        bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)\n    elif kind=='CYLINDER':\n        bpy.ops.mesh.primitive_cylinder_add(vertices=24, radius=.5, depth=1); obj=bpy.context.object; obj.name=name; obj.dimensions=size\n        bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)\n    elif kind=='WEDGE': obj=add_wedge(name,size)\n    elif kind=='HULL_SEGMENT': obj=add_hull(name,size)\n    else: raise RuntimeError('unsupported primitive '+kind)\n    obj.location=part['position']\n    obj.rotation_euler=[math.radians(v) for v in part['rotationDeg']]\n    obj['subspace_part_id']=name\n    obj['subspace_role']=part['role']\n    obj['subspace_material_zone']=part['materialZone']\n    return obj\n\ndef main():\n    clear_scene(); by_id={{}}\n    for part in RECIPE['parts']: by_id[part['id']]=add_part(part)\n    for socket in RECIPE['sockets']:\n        owner=by_id[socket['partId']]\n        owner['socket.'+socket['id']]=json.dumps(socket,sort_keys=True,separators=(',',':'))\n    scene=bpy.context.scene\n    scene['subspace_schema']={RECIPE_SCHEMA!r}\n    scene['subspace_asset_id']=RECIPE['assetId']\n    scene['subspace_recipe_sha256']=RECIPE['recipeSha256']\n    args=sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else []\n    out=args[0] if args else os.path.abspath(RECIPE['assetId']+'.glb')\n    bpy.ops.export_scene.gltf(filepath=out,export_format='GLB',export_apply=True)\n    print('SUBSPACE_KITBASH_FOUNDRY_OK',out,RECIPE['recipeSha256'])\n\nif __name__=='__main__': main()\n'''


def main() -> int:
    ap = argparse.ArgumentParser()
    sub = ap.add_subparsers(dest="cmd", required=True)
    p = sub.add_parser("inventory")
    p.add_argument("inventory", type=Path); p.add_argument("output", type=Path)
    p = sub.add_parser("validate")
    p.add_argument("recipe", type=Path)
    p = sub.add_parser("emit-worker")
    p.add_argument("recipe", type=Path); p.add_argument("output", type=Path)
    args = ap.parse_args()
    try:
        if args.cmd == "inventory":
            out = capability_registry(load_json(args.inventory))
            atomic_write(args.output, json.dumps(out, indent=2, sort_keys=True)+"\n")
            print(f"Kitbash donor registry: {out['donorCount']} donor files -> {len(out['capabilities'])} capability groups")
        elif args.cmd == "validate":
            r = validate_recipe(load_json(args.recipe)); print("VALID", r["assetId"], r["recipeSha256"])
        else:
            worker = emit_worker(load_json(args.recipe)); atomic_write(args.output, worker)
            print("WORKER", args.output)
        return 0
    except Exception as exc:
        print(f"KITBASH_FOUNDRY_BLOCKED: {exc}", file=sys.stderr)
        return 2

if __name__ == "__main__":
    raise SystemExit(main())
