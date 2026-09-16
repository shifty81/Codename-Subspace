#!/usr/bin/env python3
"""Audit hydrated Shipyard/kitbash OBJ+MTL texture dependencies.

This script is deliberately non-destructive. It walks the hydrated source corpus,
resolves mtllib/usemtl declarations, checks texture-map paths, UVs/normals and
writes a machine-readable CSV plus a Markdown review report when --write is used.
It complements the native ShipyardMaterialAuditSystem used at runtime/editor time.
"""
from __future__ import annotations

import argparse
import csv
from collections import Counter
from pathlib import Path

MAP_KEYS = {
    "map_kd": "base_color",
    "map_bump": "normal",
    "bump": "normal",
    "norm": "normal",
    "map_pr": "roughness",
    "map_pm": "metallic",
    "map_ke": "emissive",
    "map_d": "opacity",
}


def parse_mtl(path: Path):
    mats = {}
    current = None
    if not path.exists():
        return mats
    for raw in path.read_text(encoding="utf-8", errors="ignore").splitlines():
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split(maxsplit=1)
        key = parts[0].lower()
        value = parts[1].strip() if len(parts) > 1 else ""
        if key == "newmtl":
            current = value
            mats.setdefault(current, {})
        elif current and key in MAP_KEYS and value:
            # MTL map statements may contain options. The final token is the
            # conventional filename and is the safest portable dependency check.
            tex = value.split()[-1]
            mats[current][MAP_KEYS[key]] = tex
    return mats


def audit_obj(path: Path):
    mtllibs = []
    used = []
    has_vt = False
    has_vn = False
    for raw in path.read_text(encoding="utf-8", errors="ignore").splitlines():
        line = raw.strip()
        if line.startswith("mtllib "):
            mtllibs.extend(line.split()[1:])
        elif line.startswith("usemtl "):
            name = line[7:].strip()
            if name and name not in used:
                used.append(name)
        elif line.startswith("vt "):
            has_vt = True
        elif line.startswith("vn "):
            has_vn = True

    libraries = {}
    missing_mtls = []
    for rel in mtllibs:
        mtl = (path.parent / rel).resolve()
        if not mtl.exists():
            missing_mtls.append(rel)
            continue
        libraries.update(parse_mtl(mtl))

    missing_materials = [name for name in used if name not in libraries]
    missing_textures = []
    slot_rows = []
    for name in used:
        maps = libraries.get(name, {})
        status = {}
        for kind, rel in maps.items():
            candidate = (path.parent / rel).resolve()
            exists = candidate.exists()
            status[kind] = exists
            if not exists:
                missing_textures.append(f"{name}:{kind}:{rel}")
        slot_rows.append((name, maps, status))

    if missing_mtls or missing_materials:
        state = "BROKEN_DEPENDENCY"
    elif not used:
        state = "NORMALIZED_FALLBACK"
    elif not has_vn:
        state = "BROKEN_DEPENDENCY"
    elif not has_vt:
        state = "REVIEW_REQUIRED"
    elif missing_textures:
        state = "REVIEW_REQUIRED"
    elif any("base_color" not in maps for _, maps, _ in slot_rows):
        state = "NORMALIZED_FALLBACK"
    elif any(not all(status.values()) for _, _, status in slot_rows):
        state = "REVIEW_REQUIRED"
    else:
        state = "COMPLETE"

    return {
        "asset": path.as_posix(),
        "material_slots": len(used),
        "mtllib_count": len(mtllibs),
        "has_uv": int(has_vt),
        "has_normals": int(has_vn),
        "missing_mtls": "|".join(missing_mtls),
        "missing_materials": "|".join(missing_materials),
        "missing_textures": "|".join(missing_textures),
        "base_color_slots": sum("base_color" in maps for _, maps, _ in slot_rows),
        "normal_slots": sum("normal" in maps for _, maps, _ in slot_rows),
        "metallic_slots": sum("metallic" in maps for _, maps, _ in slot_rows),
        "roughness_slots": sum("roughness" in maps for _, maps, _ in slot_rows),
        "audit_state": state,
    }


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default=".")
    ap.add_argument("--source", default="content/third_party/greyoxide_shipyard_v07/source")
    ap.add_argument("--write", action="store_true")
    args = ap.parse_args()
    root = Path(args.root).resolve()
    source = (root / args.source).resolve()
    objs = sorted(source.rglob("*.obj")) if source.exists() else []
    rows = [audit_obj(p) for p in objs]
    counts = Counter(r["audit_state"] for r in rows)

    print(f"Shipyard material audit: source={source}")
    print(f"objects={len(rows)} states={dict(counts)}")
    for r in rows:
        if r["audit_state"] != "COMPLETE":
            print(f"[{r['audit_state']}] {Path(r['asset']).name} slots={r['material_slots']} uv={r['has_uv']} normals={r['has_normals']}")

    if args.write:
        out_dir = root / "artifacts" / "shipyard" / "material-audit"
        out_dir.mkdir(parents=True, exist_ok=True)
        csv_path = out_dir / "shipyard_material_audit.csv"
        md_path = out_dir / "shipyard_material_audit.md"
        fields = list(rows[0].keys()) if rows else [
            "asset","material_slots","mtllib_count","has_uv","has_normals","missing_mtls",
            "missing_materials","missing_textures","base_color_slots","normal_slots",
            "metallic_slots","roughness_slots","audit_state"
        ]
        with csv_path.open("w", encoding="utf-8", newline="") as fh:
            w = csv.DictWriter(fh, fieldnames=fields); w.writeheader(); w.writerows(rows)
        with md_path.open("w", encoding="utf-8") as fh:
            fh.write("# Shipyard Material Audit\n\n")
            fh.write(f"Hydrated OBJ modules: **{len(rows)}**\n\n")
            for key in ("COMPLETE","NORMALIZED_FALLBACK","REVIEW_REQUIRED","BROKEN_DEPENDENCY"):
                fh.write(f"- {key}: **{counts.get(key,0)}**\n")
            fh.write("\n## Review queue\n\n")
            fh.write("| Asset | State | Slots | UV | Normals | Missing dependency |\n|---|---|---:|---:|---:|---|\n")
            for r in rows:
                if r["audit_state"] == "COMPLETE": continue
                missing = r["missing_mtls"] or r["missing_materials"] or r["missing_textures"] or "semantic/PBR map review"
                fh.write(f"| `{Path(r['asset']).name}` | {r['audit_state']} | {r['material_slots']} | {r['has_uv']} | {r['has_normals']} | {missing} |\n")
        print(f"wrote {csv_path}")
        print(f"wrote {md_path}")

    # The source may intentionally be absent in a source-only checkout. In that
    # case report it without failing. Hydrated corpora fail only on broken deps.
    if not rows:
        print("[SKIP] hydrated Shipyard OBJ corpus is not present")
        return 0
    return 1 if counts.get("BROKEN_DEPENDENCY", 0) else 0


if __name__ == "__main__":
    raise SystemExit(main())
