#!/usr/bin/env python3
"""Read-only exemplar candidate extraction for SubspaceShipyard design JSON v1.

This is a pre-certification bridge, not a procedural generator, physics validator,
Blender runtime, or asset publisher. Never load bpy or change the source document.
"""
from __future__ import annotations

import argparse
import csv
import hashlib
import json
import math
import os
from pathlib import Path
import tempfile
from collections import Counter, defaultdict

SCHEMA = "subspace.shipyard_design"
CANDIDATE_SCHEMA = "subspace.ship-exemplar-candidate.v1"
AUDIT_SCHEMA = "subspace.ship-exemplar-audit.v1"
MAX_SOURCE_BYTES = 16 * 1024 * 1024
MAX_INSTANCES = 4096
COORDINATES = {"right": "+X", "forward": "+Y", "up": "+Z", "units": "meters"}


def digest(raw: bytes) -> str:
    return hashlib.sha256(raw).hexdigest()


def unique_object(pairs):
    obj = {}
    for key, value in pairs:
        if key in obj:
            raise ValueError("duplicate JSON key: " + key)
        obj[key] = value
    return obj


def load_json(path: Path):
    if not path.is_file() or path.stat().st_size > MAX_SOURCE_BYTES:
        raise ValueError("source missing or exceeds 16 MiB size limit")
    raw = path.read_bytes()
    return json.loads(raw.decode("utf-8-sig"), object_pairs_hook=unique_object), digest(raw)


def identifier(value, field):
    if not isinstance(value, str) or not value.strip() or len(value) > 256 or any(ord(c) < 32 for c in value):
        raise ValueError(field + " must be a nonempty printable string (<=256)")
    return value


def finite_triplet(value, field, positive=False):
    if not isinstance(value, list) or len(value) != 3:
        raise ValueError(field + " must contain exactly three numbers")
    result = []
    for element in value:
        if isinstance(element, bool) or not isinstance(element, (int, float)) or not math.isfinite(element):
            raise ValueError(field + " contains a nonfinite or invalid number")
        if positive and element <= 0:
            raise ValueError(field + " scale must be positive; mirrorX is a separate field")
        result.append(float(element))
    return result


def read_catalog(path):
    """Read Blender's actual Grade-A CSV columns; no geometry/socket claims."""
    if path is None:
        return {}, None
    path = Path(path)
    if not path.is_file() or path.stat().st_size > MAX_SOURCE_BYTES:
        raise ValueError("catalog missing or exceeds 16 MiB")
    raw = path.read_bytes()
    try:
        text = raw.decode("utf-8-sig")
        rows = list(csv.DictReader(text.splitlines()))
    except (UnicodeError, csv.Error) as exc:
        raise ValueError("invalid catalog CSV: " + str(exc)) from exc
    if not rows or "module_id" not in rows[0]:
        raise ValueError("catalog has no module_id records")
    catalog = {}
    for row in rows:
        key = (row.get("module_id") or "").strip()
        if not key:
            continue
        if key in catalog:
            raise ValueError("duplicate catalog module_id: " + key)
        catalog[key] = {"grade": (row.get("grade") or "").strip().upper(),
                        "class": (row.get("class") or "").strip().lower(),
                        "semantic": (row.get("semantic") or "").strip().upper(),
                        "sourceObj": (row.get("source_obj") or "").strip()}
    return catalog, digest(raw)


def issue(issues, severity, code, subject, message):
    issues.append({"severity": severity, "code": code, "subject": subject, "message": message})


def compile_candidate(source, source_sha, catalog=None, catalog_sha=None):
    """Return deterministic candidate + diagnostics, including invalid draft evidence."""
    if not isinstance(source, dict) or source.get("schema") != SCHEMA or source.get("version") != 1:
        raise ValueError("expected subspace.shipyard_design version 1; no automatic schema conversion")
    coordinate = source.get("coordinateSystem")
    if not isinstance(coordinate, dict) or any(coordinate.get(k) != v for k, v in COORDINATES.items()):
        raise ValueError("coordinate contract mismatch; requires +X right, +Y forward, +Z up, meters")
    ship = source.get("ship")
    if not isinstance(ship, dict):
        raise ValueError("ship object missing")
    name = identifier(ship.get("name"), "ship.name")
    role = identifier(ship.get("role"), "ship.role")
    modules = source.get("modules")
    if not isinstance(modules, list) or not 1 <= len(modules) <= MAX_INSTANCES:
        raise ValueError("modules must contain 1..4096 instances")
    catalog = catalog or {}
    issues = []
    nodes, ids = [], set()
    for n, entry in enumerate(modules):
        field = "modules[{}]".format(n)
        if not isinstance(entry, dict):
            raise ValueError(field + " must be an object")
        iid = identifier(entry.get("instanceId"), field + ".instanceId")
        mid = identifier(entry.get("moduleId"), field + ".moduleId")
        if iid in ids:
            issue(issues, "ERROR", "DUPLICATE_INSTANCE_ID", iid, "Instance identity repeated")
        ids.add(iid)
        t = entry.get("transform")
        if not isinstance(t, dict):
            raise ValueError(field + ".transform missing")
        p = finite_triplet(t.get("position"), field + ".position")
        r = finite_triplet(t.get("rotationEulerDeg"), field + ".rotationEulerDeg")
        s = finite_triplet(t.get("scale"), field + ".scale", positive=True)
        mirror = t.get("mirrorX", False)
        if not isinstance(mirror, bool):
            raise ValueError(field + ".mirrorX must be boolean")
        conn = entry.get("connection", {})
        if not isinstance(conn, dict):
            raise ValueError(field + ".connection must be object")
        parent = conn.get("parentInstanceId", "")
        ps = conn.get("parentSocket", "")
        cs = conn.get("childSocket", "")
        if not all(isinstance(v, str) and len(v) <= 256 for v in (parent, ps, cs)):
            raise ValueError(field + ".connection contains invalid strings")
        klass = str(entry.get("moduleClass", "")).strip().lower()
        semantic = str(entry.get("semantic", "")).strip().upper()
        size = str(entry.get("size", "")).strip().upper()
        if parent and (not ps or not cs):
            issue(issues, "ERROR", "SOCKET_PAIR_MISSING", iid, "Attached child lacks explicit parent/child socket names")
        if not parent and (ps or cs):
            issue(issues, "ERROR", "ORPHAN_SOCKET", iid, "Root has a socket connection without a parent")
        if klass != "detail" and max(s) - min(s) > 1.0e-4:
            issue(issues, "ERROR", "NONUNIFORM_STRUCTURE", iid, "Nonuniform structural scale needs an approved parametric recipe")
        if not klass or not semantic:
            issue(issues, "WARNING", "ROLE_UNCLASSIFIED", iid, "Module needs reviewed semantic/class metadata")
        if catalog_sha:
            row = catalog.get(mid)
            if not row:
                issue(issues, "ERROR", "MODULE_NOT_IN_CATALOG", iid, mid + " missing from supplied catalog")
            else:
                if row["grade"] != "A":
                    issue(issues, "ERROR", "MODULE_NOT_GRADE_A", iid, mid + " not Grade A")
                if row["class"] and row["class"] != klass:
                    issue(issues, "ERROR", "CATALOG_CLASS_MISMATCH", iid, "Exported class differs from certified catalog")
                if row["semantic"] and row["semantic"] != semantic:
                    issue(issues, "ERROR", "CATALOG_SEMANTIC_MISMATCH", iid, "Exported semantic differs from certified catalog")
        slots = entry.get("equipmentSlots", [])
        if not isinstance(slots, list):
            raise ValueError(field + ".equipmentSlots must be an array")
        for slot in slots:
            if not isinstance(slot, dict) or not isinstance(slot.get("id"), str):
                raise ValueError(field + ".equipmentSlots contains malformed slot")
        nodes.append({"instanceId": iid, "moduleId": mid, "moduleClass": klass,
                      "semantic": semantic, "size": size, "anchor": str(entry.get("anchor", "")),
                      "transform": {"position": p, "rotationEulerDeg": r, "scale": s, "mirrorX": mirror},
                      "connection": {"parentInstanceId": parent, "parentSocket": ps, "childSocket": cs},
                      "equipmentSlotTypes": sorted(str(slot.get("allowedType", "")) for slot in slots)})
    by_id = {node["instanceId"]: node for node in nodes}
    roots, edges, used_sockets = [], [], defaultdict(list)
    for node in nodes:
        iid, link = node["instanceId"], node["connection"]
        parent = link["parentInstanceId"]
        if not parent:
            roots.append(iid)
            continue
        if parent == iid:
            issue(issues, "ERROR", "SELF_PARENT", iid, "Module cannot attach to itself")
        if parent not in by_id:
            issue(issues, "ERROR", "MISSING_PARENT", iid, parent + " does not exist")
            continue
        used_sockets[(parent, link["parentSocket"])].append(iid)
        pp = by_id[parent]["transform"]["position"]
        cp = node["transform"]["position"]
        edges.append({"parentInstanceId": parent, "childInstanceId": iid,
                      "parentModuleId": by_id[parent]["moduleId"], "childModuleId": node["moduleId"],
                      "parentSemantic": by_id[parent]["semantic"], "childSemantic": node["semantic"],
                      "parentSocket": link["parentSocket"], "childSocket": link["childSocket"],
                      "worldPositionDeltaMeters": [cp[i] - pp[i] for i in range(3)]})
    if len(roots) != 1:
        issue(issues, "ERROR", "ROOT_COUNT", "assembly", "Expected exactly one connected structural root; got " + str(len(roots)))
    for (parent, sock), children in sorted(used_sockets.items()):
        if sock and len(children) > 1:
            issue(issues, "WARNING", "SHARED_PARENT_SOCKET", parent, sock + " reused by " + ",".join(sorted(children)) + "; check multi-occupancy")
    for iid in sorted(by_id):
        seen, cursor = set(), iid
        while cursor in by_id:
            if cursor in seen:
                issue(issues, "ERROR", "PARENT_CYCLE", iid, "Attachment ancestry forms a cycle")
                break
            seen.add(cursor)
            cursor = by_id[cursor]["connection"]["parentInstanceId"]
    if not catalog_sha:
        issue(issues, "WARNING", "CATALOG_ABSENT", "assembly", "Catalog not supplied; role and Grade-A membership unverified")
    issue(issues, "BLOCKER", "PHYSICAL_VALIDATION_REQUIRED", "assembly",
          "Design JSON lacks certified mesh collision, socket geometry, interior and runtime fitting evidence")
    if not any(n["moduleClass"] == "command" for n in nodes):
        issue(issues, "WARNING", "COMMAND_ABSENT", "assembly", "No command module classified")
    if not any(n["semantic"] in {"MAIN_ENGINE", "ENGINE_NOZZLE", "RCS_THRUSTER"} for n in nodes):
        issue(issues, "WARNING", "DRIVE_ABSENT", "assembly", "No propulsion module classified")
    nodes.sort(key=lambda n: n["instanceId"])
    edges.sort(key=lambda e: (e["parentInstanceId"], e["parentSocket"], e["childInstanceId"]))
    issues.sort(key=lambda e: (e["severity"], e["code"], e["subject"], e["message"]))
    axes = list(zip(*(n["transform"]["position"] for n in nodes)))
    mins, maxs = [min(a) for a in axes], [max(a) for a in axes]
    roles = Counter(n["semantic"] for n in nodes)
    # Position-only center symmetry is merely a candidate; socket mirroring
    # and actual silhouettes must be proved by native geometry validation.
    symmetry = []
    for i, left in enumerate(nodes):
        for right in nodes[i+1:]:
            a, b = left["transform"]["position"], right["transform"]["position"]
            if (left["moduleId"] == right["moduleId"] and abs(a[0]+b[0]) < .05
                    and abs(a[1]-b[1]) < .05 and abs(a[2]-b[2]) < .05 and abs(a[0]-b[0]) > .1):
                symmetry.append([left["instanceId"], right["instanceId"]])
    candidate = {
        "schema": CANDIDATE_SCHEMA, "state": "DRAFT_UNCERTIFIED", "generatorEligible": False,
        "source": {"schema": SCHEMA, "version": 1, "sha256": source_sha, "catalogSha256": catalog_sha},
        "ship": {"name": name, "role": role, "seed": ship.get("seed"), "appearance": ship.get("appearance", {}),
                 "orientation": ship.get("orientation", {})},
        "coordinateSystem": COORDINATES, "rootInstanceIds": sorted(roots),
        "nodes": nodes, "attachmentEdges": edges,
        "observations": {"moduleCount": len(nodes), "attachmentCount": len(edges),
                         "semanticCounts": dict(sorted(roles.items())), "positionRangeMeters": {"minimum": mins, "maximum": maxs},
                         "symmetryPairHints": symmetry},
        "notCertified": ["mesh collision", "socket positions/normals", "attachment penetration", "runtime fitting",
                         "interior connectivity", "material fidelity", "silhouette quality", "class envelope"],
    }
    audit = {"schema": AUDIT_SCHEMA, "candidateState": candidate["state"],
             "sourceSha256": source_sha, "errorCount": sum(i["severity"] == "ERROR" for i in issues),
             "blockerCount": sum(i["severity"] == "BLOCKER" for i in issues),
             "issues": issues, "promotionAllowed": False,
             "nextAuthority": "Native Shipyard geometry/socket/physics + human visual approval"}
    return candidate, audit


def atomic_json(path: Path, document, overwrite=False):
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.exists() and not overwrite:
        raise FileExistsError(str(path) + " exists; use --overwrite for deliberate replacement")
    fd, temp = tempfile.mkstemp(prefix=".exemplar-", suffix=".tmp", dir=str(path.parent))
    try:
        with os.fdopen(fd, "w", encoding="utf-8", newline="\n") as out:
            json.dump(document, out, indent=2, ensure_ascii=False, allow_nan=False, sort_keys=True)
            out.write("\n")
            out.flush()
            os.fsync(out.fileno())
        os.replace(temp, path)
    finally:
        if os.path.exists(temp):
            os.unlink(temp)


def main(argv=None):
    p = argparse.ArgumentParser(description="Extract a DRAFT exemplar from Blender SubspaceShipyard v1 export (no promotion)")
    p.add_argument("--input", type=Path, required=True, help=".subspace_shipyard.json file")
    p.add_argument("--catalog", type=Path, help="Certified Greyoxide module CSV (strongly recommended)")
    p.add_argument("--out-dir", type=Path, required=True, help="Staging folder outside source assets")
    p.add_argument("--overwrite", action="store_true", help="Explicitly replace existing generated staging files")
    args = p.parse_args(argv)
    try:
        source, src_hash = load_json(args.input)
        catalog, cat_hash = read_catalog(args.catalog)
        candidate, audit = compile_candidate(source, src_hash, catalog, cat_hash)
        output = args.out_dir.resolve()
        source_path = args.input.resolve()
        if output == source_path.parent or output == source_path or source_path in output.parents:
            raise ValueError("output cannot be the source document or within the source document path")
        targets = [output / "exemplar_candidate.json", output / "exemplar_audit.json"]
        if not args.overwrite and any(t.exists() for t in targets):
            raise FileExistsError("generated output exists; --overwrite required")
        atomic_json(targets[0], candidate, args.overwrite)
        atomic_json(targets[1], audit, args.overwrite)
        print("DRAFT ONLY: {} modules, {} edges, {} errors, {} blockers. No exemplar published.".format(
            len(candidate["nodes"]), len(candidate["attachmentEdges"]), audit["errorCount"], audit["blockerCount"]))
        print("Candidate: {}\nAudit: {}".format(*targets))
        return 0 if audit["errorCount"] == 0 else 2
    except (OSError, ValueError, TypeError, UnicodeError, json.JSONDecodeError) as exc:
        print("EXEMPLAR INTAKE FAILED: " + str(exc))
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
