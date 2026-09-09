"""Codename Subspace authored Shipyard reference-vessel usage audit.

Run with Blender:
  blender --background --python tools/blender/shipyard_reference_usage_audit.py -- \
      --project-root <repo> --input <shipyard_strikes_back.glb>

This is evidence generation only. It never changes source assets or runtime classifications.
It decomposes the three authored reference ship meshes into disconnected geometry components,
compares those components against certified loose Shipyard module signatures, and emits only
high-confidence matches. Ambiguous evidence stays unresolved rather than being guessed.
"""
from __future__ import annotations

import argparse
import csv
import json
import math
import sys
from collections import defaultdict, deque
from pathlib import Path

import bpy

REFERENCE_NAMES = ("ScoutShip_156", "BattleShip_157", "CruiserShip_158")


def parse_args():
    argv = sys.argv
    argv = argv[argv.index("--") + 1 :] if "--" in argv else []
    ap = argparse.ArgumentParser()
    ap.add_argument("--project-root", required=True)
    ap.add_argument("--input", required=True)
    return ap.parse_args(argv)


def canonical(value: str) -> str:
    return "".join(c.lower() for c in value if c.isalnum())


def obj_signature(path: Path):
    verts = []
    faces = 0
    try:
        for raw in path.read_text(encoding="utf-8", errors="ignore").splitlines():
            line = raw.strip()
            if line.startswith("v "):
                p = line.split()
                if len(p) >= 4:
                    verts.append((float(p[1]), float(p[2]), float(p[3])))
            elif line.startswith("f "):
                faces += max(1, len(line.split()) - 3)
    except Exception:
        return None
    if not verts:
        return None
    xs, ys, zs = zip(*verts)
    dims = tuple(sorted((max(xs)-min(xs), max(ys)-min(ys), max(zs)-min(zs))))
    return {"vertices": len(verts), "faces": faces, "dims": dims}


def component_signatures(obj):
    mesh = obj.data
    adjacency = defaultdict(set)
    used = set()
    for poly in mesh.polygons:
        vs = list(poly.vertices)
        used.update(vs)
        for a, b in zip(vs, vs[1:] + vs[:1]):
            adjacency[a].add(b); adjacency[b].add(a)
    components = []
    pending = set(used)
    while pending:
        seed = next(iter(pending)); q = deque([seed]); group = set([seed]); pending.remove(seed)
        while q:
            a = q.popleft()
            for b in adjacency[a]:
                if b in pending:
                    pending.remove(b); group.add(b); q.append(b)
        coords = [mesh.vertices[i].co.copy() for i in group]
        xs=[v.x for v in coords]; ys=[v.y for v in coords]; zs=[v.z for v in coords]
        dims=tuple(sorted((max(xs)-min(xs), max(ys)-min(ys), max(zs)-min(zs))))
        face_count=sum(1 for p in mesh.polygons if p.vertices and p.vertices[0] in group)
        centroid=sum(coords, coords[0]*0.0) / max(1,len(coords))
        components.append({"vertices":len(group),"faces":face_count,"dims":dims,"centroid":tuple(centroid)})
    return components


def score(sig, candidate):
    if sig["vertices"] <= 0 or candidate["vertices"] <= 0:
        return 0.0
    vr = min(sig["vertices"], candidate["vertices"]) / max(sig["vertices"], candidate["vertices"])
    fr = min(max(1,sig["faces"]), max(1,candidate["faces"])) / max(max(1,sig["faces"]), max(1,candidate["faces"]))
    ds=[]
    for a,b in zip(sig["dims"],candidate["dims"]):
        if max(abs(a),abs(b)) < 1e-6: ds.append(1.0)
        else: ds.append(min(abs(a),abs(b))/max(abs(a),abs(b)))
    return vr*.48 + fr*.22 + (sum(ds)/3.0)*.30


def find_reference_objects():
    refs = {}
    for ref in REFERENCE_NAMES:
        key=canonical(ref)
        matches=[o for o in bpy.context.scene.objects if o.type=="MESH" and key in canonical(o.name)]
        if matches:
            refs[ref]=max(matches,key=lambda o:len(o.data.vertices))
    return refs


def main():
    args=parse_args(); root=Path(args.project_root).resolve(); source=Path(args.input).resolve()
    catalog=root/"content"/"derived"/"greyoxide_shipyard_v07"/"certified"/"certified_module_catalog.csv"
    if not source.exists(): raise SystemExit(f"reference GLB missing: {source}")
    if not catalog.exists(): raise SystemExit(f"certified module catalog missing: {catalog}")

    module_sigs={}; module_rows={}
    with catalog.open(newline="",encoding="utf-8-sig") as fh:
        for row in csv.DictReader(fh):
            module_id=row.get("module_id","").strip()
            rel=row.get("runtime_path","").strip()
            obj=(root/rel) if rel else root/"content"/"derived"/"greyoxide_shipyard_v07"/"certified"/"modules"/f"{module_id}.obj"
            sig=obj_signature(obj)
            if module_id and sig:
                module_sigs[module_id]=sig; module_rows[module_id]=row

    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.gltf(filepath=str(source))
    refs=find_reference_objects()
    out_dir=root/"content"/"derived"/"shipyard_strikes_back_v1"/"reference_assemblies"
    out_dir.mkdir(parents=True,exist_ok=True)
    usage=[]; summary={"schema":"subspace.shipyard_reference_usage","version":1,"source":str(source),"references":{},"moduleCandidates":len(module_sigs)}

    for ref,obj in refs.items():
        comps=component_signatures(obj); matches=[]
        for idx,comp in enumerate(comps):
            ranked=sorted(((score(comp,sig),mid) for mid,sig in module_sigs.items()),reverse=True)[:3]
            best_score,best_id=ranked[0] if ranked else (0.0,"")
            second=ranked[1][0] if len(ranked)>1 else 0.0
            confidence=best_score if best_score-second>=.035 else best_score*.75
            accepted=confidence>=.93 and best_score-second>=.025
            rec={"component":idx,"centroid":comp["centroid"],"vertices":comp["vertices"],"faces":comp["faces"],"dimensions":comp["dims"],"bestModuleId":best_id,"score":round(best_score,5),"margin":round(best_score-second,5),"accepted":accepted}
            matches.append(rec)
            if accepted:
                usage.append({"reference_ship":ref,"component":idx,"module_id":best_id,"confidence":round(confidence,5),"x":comp["centroid"][0],"y":comp["centroid"][1],"z":comp["centroid"][2]})
        payload={"schema":"subspace.shipyard_reference_assembly","version":1,"reference":ref,"object":obj.name,"components":matches}
        (out_dir/f"{ref}.reference.json").write_text(json.dumps(payload,indent=2),encoding="utf-8")
        summary["references"][ref]={"object":obj.name,"components":len(comps),"acceptedMatches":sum(1 for m in matches if m["accepted"])}

    summary["missingReferences"]=[r for r in REFERENCE_NAMES if r not in refs]
    with (out_dir/"reference_module_usage.csv").open("w",newline="",encoding="utf-8") as fh:
        fields=["reference_ship","component","module_id","confidence","x","y","z"]
        w=csv.DictWriter(fh,fieldnames=fields);w.writeheader();w.writerows(usage)
    (out_dir/"REFERENCE_AUDIT_SUMMARY.json").write_text(json.dumps(summary,indent=2),encoding="utf-8")
    print(json.dumps(summary,indent=2))
    if not refs: raise SystemExit("No authored reference ship objects were found in the GLB")

if __name__ == "__main__":
    main()
