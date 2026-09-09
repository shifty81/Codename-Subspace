#!/usr/bin/env python3
"""Batch-audit Greyoxide Shipyard v0.7 filename classification.

This intentionally classifies from the original leaf filename, never from the
historical `shipyard_a_<class>_...` prefix. It also performs lightweight OBJ
geometry checks so obvious name-based findings are verified against the source
mesh corpus without destructively changing geometry.
"""
from __future__ import annotations

import argparse
import csv
import json
import math
import re
from collections import Counter, defaultdict, deque
from pathlib import Path


def canonical_leaf(value: str) -> str:
    value = Path(value).stem.lower()
    nested = value.find("_shipyard_")
    if nested >= 0:
        value = value[nested + 1 :]
    for prefix in ("shipyard_a_", "shipyard_"):
        if value.startswith(prefix):
            rest = value[len(prefix) :]
            parts = rest.split("_", 2)
            if len(parts) == 3:
                value = parts[2]
    return value


def classify(value: str):
    v = canonical_leaf(value)
    has = lambda *xs: any(x in v for x in xs)
    if v.startswith("hull") or has("_hull", "fuselage"):
        if has("bow", "nose", "front"):
            return "hull", "HULL_BOW", 100, "hull-name/bow"
        if has("aft", "rear", "stern"):
            return "hull", "HULL_AFT", 100, "hull-name/aft"
        return "hull", "HULL_MID", 100, "hull-name"
    if v.startswith("hardpoint"):
        if has("gun", "weapon", "cannon", "missile"):
            return "hardpoint", "WEAPON_MOUNT", 98, "hardpoint-gun-name"
        return "hardpoint", "TURRET_HARDPOINT", 96, "hardpoint-base-name"
    if has("bridge", "cockpit", "canopy", "command"):
        return "command", ("COMMAND_COCKPIT" if has("cockpit", "canopy") else "COMMAND_BRIDGE"), 100, "command-name"
    if v.startswith("engine") or v.startswith("enigne"):
        if has("trussworkwing", "wing"):
            return "wing", "WING", 100, "engine-builder-wing"
        if has("strut"):
            return "component", "STRUCTURAL_FRAME", 100, "engine-builder-strut"
        if has("body", "bracket"):
            return "propulsion", "ENGINE_HOUSING", 98, "engine-builder-body/housing"
        if has("trumpet", "vanes", "flap"):
            return "propulsion", "ENGINE_NOZZLE", 94, "engine-builder-nozzle/control"
        return "propulsion", "MAIN_ENGINE", 92, "engine-builder-drive"
    if has("bipolarengine"):
        return "propulsion", "MAIN_ENGINE", 100, "explicit-engine-name"
    if has("rcs", "retro", "maneuver", "thruster"):
        return "propulsion", "RCS_THRUSTER", 100, "thruster-name"
    if has("nozzle", "bell", "exhaust"):
        return "propulsion", "ENGINE_NOZZLE", 100, "nozzle-name"
    if has("housing", "shroud", "nacelle", "enginepod", "engine_pod"):
        return "propulsion", "ENGINE_HOUSING", 98, "engine-housing-name"
    if has("drive", "propulsion"):
        return "propulsion", "MAIN_ENGINE", 90, "generic-propulsion-name"
    if has("vertmisstube", "misstube", "missiletube", "missile_tube"):
        return "hardpoint", "WEAPON_MOUNT", 98, "missile-tube-name"
    if has("wing", "fin"):
        return "wing", "WING", 97, "wing/fin-name"
    if has("sensor", "antenna", "radar", "instrument", "telescope", "dish", "mast"):
        return "detail", "SENSOR", 92, "sensor/instrument-name"
    if has("greeble", "detail", "vent", "panel", "leafpanel"):
        return "detail", "SURFACE_DETAIL", 96, "surface-detail-name"
    if has("outrigger", "railrunner"):
        return "component", "STRUCTURAL_FRAME", 84, "structural-component-name"
    if has("adapter", "connector", "neck", "fairing"):
        return "adapter", "ADAPTER", 92, "adapter-name"
    if has("turret", "weapon", "gun", "mount"):
        return "hardpoint", ("WEAPON_MOUNT" if has("gun", "weapon") else "TURRET_HARDPOINT"), 88, "generic-hardpoint-name"
    return "component", "COMPONENT", 72, "generic-component"


def obj_metrics(path: Path):
    verts = []
    faces = []
    for line in path.read_text(encoding="utf-8", errors="ignore").splitlines():
        if line.startswith("v "):
            p = line.split()
            if len(p) >= 4:
                try:
                    verts.append(tuple(float(x) for x in p[1:4]))
                except ValueError:
                    pass
        elif line.startswith("f "):
            ids = []
            for token in line.split()[1:]:
                try:
                    raw = int(token.split("/")[0])
                    ids.append(raw - 1 if raw > 0 else len(verts) + raw)
                except (ValueError, IndexError):
                    pass
            if len(ids) >= 3:
                # One connectivity entry is enough for the diagnostic.
                faces.append(ids)
    if not verts:
        return dict(vertices=0, faces=0, components=0, dx=0.0, dy=0.0, dz=0.0, major_minor=0.0)
    mins = [min(v[i] for v in verts) for i in range(3)]
    maxs = [max(v[i] for v in verts) for i in range(3)]
    dims = [maxs[i] - mins[i] for i in range(3)]
    s = sorted(dims, reverse=True)
    ratio = s[0] / max(1e-9, s[-1])

    # Face components by shared source vertex. It is diagnostic only: the kit
    # deliberately contains multi-island authored objects.
    vf = defaultdict(list)
    for fi, f in enumerate(faces):
        for vi in f:
            vf[vi].append(fi)
    seen = set()
    components = 0
    for fi in range(len(faces)):
        if fi in seen:
            continue
        components += 1
        seen.add(fi)
        q = [fi]
        while q:
            cur = q.pop()
            for vi in faces[cur]:
                for nxt in vf[vi]:
                    if nxt not in seen:
                        seen.add(nxt)
                        q.append(nxt)
    return dict(vertices=len(verts), faces=len(faces), components=components, dx=dims[0], dy=dims[1], dz=dims[2], major_minor=ratio)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default=".", help="repository root")
    ap.add_argument("--write", action="store_true", help="write CSV/Markdown audit beside certified corpus")
    args = ap.parse_args()
    root = Path(args.root).resolve()
    raw_root = root / "content/third_party/greyoxide_shipyard_v07/source"
    cert_root = root / "content/derived/greyoxide_shipyard_v07/certified"
    catalog_path = cert_root / "certified_module_catalog.csv"

    raw = {}
    for p in sorted(raw_root.rglob("*.obj")):
        leaf = canonical_leaf(p.stem)
        if leaf in raw:
            raise SystemExit(f"duplicate raw leaf: {leaf}: {raw[leaf]} and {p}")
        raw[leaf] = p

    rows = list(csv.DictReader(catalog_path.open(encoding="utf-8-sig", newline="")))
    output = []
    missing = []
    for row in rows:
        leaf = canonical_leaf(row["source_obj"])
        source = raw.get(leaf)
        if not source:
            missing.append((row["module_id"], leaf))
            m = dict(vertices=0, faces=0, components=0, dx=0.0, dy=0.0, dz=0.0, major_minor=0.0)
            folder = "MISSING"
        else:
            m = obj_metrics(source)
            folder = source.parent.name
        cls, semantic, confidence, rule = classify(leaf)
        flags = []
        if semantic == "STRUCTURAL_FRAME" and m["major_minor"] < 2.5:
            flags.append("STRUCTURE_NOT_ELONGATED")
        if semantic == "MAIN_ENGINE" and m["vertices"] < 500:
            flags.append("MAIN_ENGINE_LOW_COMPLEXITY")
        if cls == "propulsion" and "strut" in leaf:
            flags.append("BAD_PROPULSION_STRUT")
        if cls == "propulsion" and "wing" in leaf:
            flags.append("BAD_PROPULSION_WING")
        historical = ""
        mid = row["module_id"].lower()
        if mid.startswith("shipyard_a_"):
            rest = mid[len("shipyard_a_"):]
            historical = rest.split("_", 1)[0]
        output.append({
            "module_id": row["module_id"],
            "canonical_name": leaf,
            "source_folder": folder,
            "historical_id_class": historical,
            "catalog_class": row.get("class", ""),
            "classified_class": cls,
            "semantic": semantic,
            "confidence": confidence,
            "rule": rule,
            "historical_class_corrected": int(historical != cls),
            "catalog_matches_classifier": int(row.get("class", "") == cls),
            "vertices": m["vertices"],
            "faces": m["faces"],
            "components": m["components"],
            "dim_x": f'{m["dx"]:.6f}',
            "dim_y": f'{m["dy"]:.6f}',
            "dim_z": f'{m["dz"]:.6f}',
            "major_minor_ratio": f'{m["major_minor"]:.3f}',
            "geometry_flags": "|".join(flags),
        })

    checks = []
    checks.append(("raw source objects == 156", len(raw) == 156, len(raw)))
    checks.append(("certified objects == 156", len(rows) == 156, len(rows)))
    checks.append(("every certified object maps to a raw leaf", not missing, len(missing)))
    engine_rows = [r for r in output if r["source_folder"] == "engines"]
    checks.append(("engine-builder source objects == 28", len(engine_rows) == 28, len(engine_rows)))
    checks.append(("engine struts are not propulsion", all(r["classified_class"] != "propulsion" for r in engine_rows if "strut" in r["canonical_name"]), 0))
    checks.append(("engine trusswork wing is WING", any(r["canonical_name"] == "enginetrussworkwing" and r["semantic"] == "WING" for r in engine_rows), 0))
    checks.append(("engine bodies are housings", all(r["semantic"] == "ENGINE_HOUSING" for r in engine_rows if "body" in r["canonical_name"]), 0))
    checks.append(("main engine candidates pass complexity sanity check", all(int(r["vertices"]) >= 500 for r in output if r["semantic"] == "MAIN_ENGINE"), 0))
    checks.append(("structural engine struts pass elongation sanity check", all(float(r["major_minor_ratio"]) >= 2.5 for r in engine_rows if r["semantic"] == "STRUCTURAL_FRAME"), 0))
    checks.append(("certified catalog matches filename classifier", all(r["catalog_matches_classifier"] == 1 for r in output), sum(r["catalog_matches_classifier"] == 0 for r in output)))
    checks.append(("no geometry classification flags", all(not r["geometry_flags"] for r in output), sum(bool(r["geometry_flags"]) for r in output)))

    print("Shipyard filename classification audit")
    print(f"raw={len(raw)} certified={len(rows)} historical_class_corrections={sum(r['historical_class_corrected'] for r in output)}")
    print("classes:", dict(Counter(r["classified_class"] for r in output)))
    print("semantics:", dict(Counter(r["semantic"] for r in output)))
    for name, ok, detail in checks:
        print(f"[{'PASS' if ok else 'FAIL'}] {name} ({detail})")

    if args.write:
        audit_csv = cert_root / "classification_audit.csv"
        with audit_csv.open("w", encoding="utf-8", newline="") as fh:
            writer = csv.DictWriter(fh, fieldnames=list(output[0].keys()))
            writer.writeheader(); writer.writerows(output)
        report = root / "docs/SHIPYARD_CLASSIFICATION_AUDIT_2026-09-01.md"
        changed = [r for r in output if r["historical_class_corrected"]]
        propulsion = [r for r in output if r["semantic"] in {"MAIN_ENGINE","ENGINE_HOUSING","ENGINE_NOZZLE","RCS_THRUSTER","STRUCTURAL_FRAME","WING"} and (r["source_folder"] == "engines" or "engine" in r["canonical_name"])]
        with report.open("w", encoding="utf-8") as fh:
            fh.write("# Greyoxide Shipyard v0.7 Classification Audit — 2026-09-01\n\n")
            fh.write("Classification is filename-first and ignores historical class text embedded in certified module IDs. Geometry checks are diagnostic only and never split or remodel authored objects.\n\n")
            fh.write(f"- Raw source objects: **{len(raw)}**\n- Certified objects: **{len(rows)}**\n- Class corrections: **{len(changed)}**\n")
            fh.write("- Corrected class counts: " + ", ".join(f"{k}={v}" for k,v in sorted(Counter(r['classified_class'] for r in output).items())) + "\n\n")
            fh.write("## Batch checks\n\n")
            for name, ok, detail in checks:
                fh.write(f"- [{'x' if ok else ' '}] {name} (`{detail}`)\n")
            fh.write("\n## Engine-builder classification\n\n")
            fh.write("| Name | Class | Semantic | Confidence | Geometry |\n|---|---|---|---:|---|\n")
            for r in propulsion:
                fh.write(f"| `{r['canonical_name']}` | {r['classified_class']} | {r['semantic']} | {r['confidence']} | {r['vertices']} verts; aspect {r['major_minor_ratio']} |\n")
            fh.write("\n## Class corrections\n\n")
            fh.write("| Name | Previous | Corrected | Semantic | Rule |\n|---|---|---|---|---|\n")
            for r in changed:
                fh.write(f"| `{r['canonical_name']}` | {r['historical_id_class']} | {r['classified_class']} | {r['semantic']} | {r['rule']} |\n")
        print(f"wrote {audit_csv}")
        print(f"wrote {report}")

    return 0 if all(ok for _, ok, _ in checks) else 1


if __name__ == "__main__":
    raise SystemExit(main())
