#!/usr/bin/env python3
"""Read native SUBSPACE_SHIP_BLUEPRINT_V1 as a *review-only* exemplar candidate.

Only the canonical native blueprint file format is accepted. No attempt is made
 to repair source, reinterpret unversioned data, or grant asset certification.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
from pathlib import Path
import shlex
import sys

from exemplar_intake import (AUDIT_SCHEMA, CANDIDATE_SCHEMA, MAX_INSTANCES,
                             MAX_SOURCE_BYTES, COORDINATES, atomic_json,
                             compile_candidate, identifier, read_catalog)

MAGIC = 'SUBSPACE_SHIP_BLUEPRINT_V1'
ALLOWED_ROWS = {'META', 'RECIPE', 'ORIENTATION', 'TAG', 'MODULE', 'ATTACH',
                'ARTIC', 'EQUIP', 'PAINT', 'WEAR', 'DECAL'}


def number(value: str, field: str, positive=False) -> float:
    try:
        n = float(value)
    except (ValueError, OverflowError) as exc:
        raise ValueError(field + ': invalid number') from exc
    if not math.isfinite(n) or abs(n) > 1e9 or (positive and n <= 0):
        raise ValueError(field + ': nonfinite, unreasonable or nonpositive number')
    return n


def bounded_int(value: str, field: str, minimum: int, maximum: int) -> int:
    try:
        if not value or value.strip() != value or any(c not in '-0123456789' for c in value):
            raise ValueError()
        n = int(value)
    except (ValueError, OverflowError) as exc:
        raise ValueError(field + ': invalid integer') from exc
    if not minimum <= n <= maximum:
        raise ValueError(field + ': integer out of bounds')
    return n


def row_tokens(line: str, lineno: int) -> list[str]:
    # Native C++ std::quoted writes double-quoted, escaped strings. shlex's
    # POSIX quoted-token handling matches relevant normal UTF-8 module IDs.
    # Round-trip is intentionally restricted to supported, versioned v1 rows.
    try:
        result = shlex.split(line, comments=False, posix=True)
    except ValueError as exc:
        raise ValueError('line {}: invalid quoted string: {}'.format(lineno, exc)) from exc
    if not result or result[0] not in ALLOWED_ROWS:
        raise ValueError('line {}: unsupported row; do not silently skip format extensions'.format(lineno))
    return result


def parse_native(raw: bytes, catalog=None, catalog_hash=None):
    if len(raw) > MAX_SOURCE_BYTES:
        raise ValueError('native blueprint exceeds 16 MiB')
    try:
        text = raw.decode('utf-8-sig')
    except UnicodeDecodeError as exc:
        raise ValueError('native blueprint is not UTF-8') from exc
    lines = text.splitlines()
    if not lines or lines[0].strip() != MAGIC:
        raise ValueError('native blueprint requires exact SUBSPACE_SHIP_BLUEPRINT_V1 header')
    catalog = catalog or {}
    source_hash = hashlib.sha256(raw).hexdigest()
    modules, edges, names = [], [], {}
    seen_meta = seen_recipe = False
    for lineno, line in enumerate(lines[1:], 2):
        if not line.strip():
            continue
        t = row_tokens(line, lineno)
        kind, data = t[0], t[1:]
        if kind == 'META':
            if seen_meta or len(data) != 4:
                raise ValueError('line {}: duplicate/malformed META'.format(lineno))
            seen_meta = True
            names['blueprintId'] = identifier(data[0], 'META blueprintId')
            names['name'] = identifier(data[1], 'META name')
            bounded_int(data[3], 'META revision', 0, 2**31 - 1)
        elif kind == 'RECIPE':
            if seen_recipe or len(data) != 8:
                raise ValueError('line {}: duplicate/malformed RECIPE'.format(lineno))
            seen_recipe = True
            names['recipeId'] = identifier(data[0], 'RECIPE id')
            names['role'] = identifier(data[1], 'RECIPE role')
            names['seed'] = bounded_int(data[2], 'RECIPE seed', 0, 2**32 - 1)
            number(data[6], 'RECIPE widthScale', positive=True)
            number(data[7], 'RECIPE lengthScale', positive=True)
        elif kind == 'MODULE':
            if not 11 <= len(data) <= 15:
                raise ValueError('line {}: malformed MODULE (11..15 fields)'.format(lineno))
            if len(modules) >= MAX_INSTANCES:
                raise ValueError('too many modules')
            mid = identifier(data[0], 'MODULE moduleId')
            p = [number(v, 'MODULE position') for v in data[1:4]]
            s = [number(v, 'MODULE scale', positive=True) for v in data[4:7]]
            r = [number(v, 'MODULE angles') for v in data[7:10]]
            material = bounded_int(data[10], 'MODULE material enum', -10000, 10000)
            flags = [bounded_int(v, 'MODULE boolean', 0, 1) for v in data[11:]]
            flags += [0, 0, 0, 1][len(flags):]
            row = catalog.get(mid, {})
            ordinal = len(modules)
            modules.append({'instanceId': 'native-{}-index-{:05d}'.format(source_hash[:12], ordinal),
                'moduleId': mid, 'moduleClass': row.get('class', ''),
                'semantic': row.get('semantic', ''), 'size': '', 'anchor': '',
                'transform': {'position': p, 'rotationEulerDeg': r,
                              'scale': s, 'mirrorX': bool(flags[0])},
                'connection': {'parentInstanceId': '', 'parentSocket': '', 'childSocket': ''},
                'equipmentSlots': []})
            # Native format has extra mirroring axes + a material enum; do not
            # discard their evidence and pretend the bridge reproduced them.
            modules[-1]['_nativeEvidence'] = {'ordinal': ordinal, 'materialEnum': material,
                'yawPitchRollDegrees': r, 'mirrorY': bool(flags[1]),
                'mirrorZ': bool(flags[2]), 'sourceMaterialsEnabled': bool(flags[3])}
        elif kind == 'ATTACH':
            if len(data) != 6:
                raise ValueError('line {}: malformed ATTACH'.format(lineno))
            p = bounded_int(data[0], 'ATTACH parent index', 0, MAX_INSTANCES - 1)
            c = bounded_int(data[1], 'ATTACH child index', 0, MAX_INSTANCES - 1)
            gap = number(data[4], 'ATTACH measured gap')
            certified = bounded_int(data[5], 'ATTACH certified flag', 0, 1)
            edges.append((p, c, data[2], data[3], gap, certified))
        else:
            # Auxiliary rows are retained in the immutable source file and its
            # hash; their semantic payload is not interpreted or certified.
            if not data:
                raise ValueError('line {}: empty {} row'.format(lineno, kind))
    if not seen_meta or not seen_recipe or not modules:
        raise ValueError('native blueprint lacks META, RECIPE or MODULE records')
    if len(edges) > MAX_INSTANCES * 8:
        raise ValueError('excessive attachment count')
    seen_children = set()
    native_issues = []
    for parent, child, ps, cs, gap, certified in edges:
        if parent >= len(modules) or child >= len(modules):
            raise ValueError('ATTACH references a missing module array index')
        if child in seen_children:
            raise ValueError('ATTACH has multiple parents for one native module')
        seen_children.add(child)
        modules[child]['connection'] = {'parentInstanceId': modules[parent]['instanceId'],
                                        'parentSocket': ps, 'childSocket': cs}
        if not certified:
            native_issues.append({'severity': 'WARNING', 'code': 'UNCERTIFIED_NATIVE_ATTACHMENT',
                'subject': modules[child]['instanceId'],
                'message': 'Native ATTACH certified flag is false; full geometry validation still required'})
        if abs(gap) > .001:
            native_issues.append({'severity': 'WARNING', 'code': 'MEASURED_ATTACHMENT_GAP',
                'subject': modules[child]['instanceId'],
                'message': 'Native persisted measuredGap is not zero: {:.6g} m'.format(gap)})
    design = {'schema': 'subspace.shipyard_design', 'version': 1,
              'coordinateSystem': COORDINATES,
              'ship': {'name': names['name'], 'role': names['role'], 'seed': names['seed']},
              'modules': modules}
    candidate, audit = compile_candidate(design, source_hash, catalog, catalog_hash)
    # Never pretend this is Blender JSON or that index-derived identities are
    # stable across edited/reordered native documents.
    candidate['source'] = {'schema': MAGIC, 'version': 1, 'sha256': source_hash,
        'catalogSha256': catalog_hash, 'blueprintId': names['blueprintId'],
        'recipeId': names['recipeId'], 'identityPolicy': 'DOCUMENT_HASH_AND_ORDINAL_PROVISIONAL'}
    candidate['nativeModuleEvidence'] = [m['_nativeEvidence'] for m in modules]
    candidate['ship']['nativeBlueprintId'] = names['blueprintId']
    candidate['notCertified'] += ['stable serialized module instance identities',
        'native yaw/pitch/roll to Blender Euler equivalence', 'non-X mirroring parity',
        'canonical native ship save/reload equivalence', 'native attachment certificate freshness']
    native_issues += [
        {'severity': 'BLOCKER', 'code': 'NATIVE_INSTANCE_ID_NOT_SERIALIZED',
         'subject': 'assembly', 'message': 'Native MODULE rows have no stable instanceId; generated index aliases must not be persisted as stable identities'},
        {'severity': 'WARNING', 'code': 'NATIVE_COORDINATE_CONTRACT_INFERRED',
         'subject': 'assembly', 'message': 'Native v1 file does not serialize axis/units; interpreted using current Subspace authoring contract'},
        {'severity': 'WARNING', 'code': 'NATIVE_TRANSFORM_PARITY_UNPROVEN',
         'subject': 'assembly', 'message': 'Native yaw/pitch/roll and mirrorY/Z are preserved as evidence but have not been converted to Blender Euler rotations'},
    ]
    audit['issues'] += native_issues
    audit['issues'].sort(key=lambda e: (e['severity'], e['code'], e['subject'], e['message']))
    audit['errorCount'] = sum(e['severity'] == 'ERROR' for e in audit['issues'])
    audit['blockerCount'] = sum(e['severity'] == 'BLOCKER' for e in audit['issues'])
    audit['candidateState'] = 'DRAFT_UNCERTIFIED'
    audit['sourceSha256'] = source_hash
    audit['promotionAllowed'] = False
    audit['nextAuthority'] = 'Canonical native instance IDs + geometry/socket/physics + visual review'
    return candidate, audit


def main(argv=None):
    p = argparse.ArgumentParser(description='Read-only native ship to review-only exemplar candidate')
    p.add_argument('--input', type=Path, required=True)
    p.add_argument('--catalog', type=Path)
    p.add_argument('--out-dir', type=Path, required=True)
    p.add_argument('--overwrite', action='store_true')
    args = p.parse_args(argv)
    try:
        src = args.input.resolve()
        out = args.out_dir.resolve()
        if not src.is_file() or src.stat().st_size > MAX_SOURCE_BYTES or src.suffix != '.subspace_ship':
            raise ValueError('input must be an existing .subspace_ship <=16 MiB')
        if out == src.parent or src.parent in out.parents or out == src:
            raise ValueError('output must be outside native source directory')
        if args.catalog:
            catalog_dir = args.catalog.resolve().parent
            if out == catalog_dir or catalog_dir in out.parents:
                raise ValueError('output must not be inside the source catalog directory')
        catalog, cat_hash = read_catalog(args.catalog)
        candidate, audit = parse_native(src.read_bytes(), catalog, cat_hash)
        paths = [out / 'exemplar_candidate.json', out / 'exemplar_audit.json']
        if not args.overwrite and any(path.exists() for path in paths):
            raise FileExistsError('output already exists; --overwrite required')
        for path, document in zip(paths, [candidate, audit]):
            atomic_json(path, document, args.overwrite)
        print('NATIVE DRAFT ONLY: {} modules, {} links, {} errors, {} blockers'.format(
            len(candidate['nodes']), len(candidate['attachmentEdges']), audit['errorCount'], audit['blockerCount']))
        return 0 if audit['errorCount'] == 0 else 2
    except (OSError, ValueError, TypeError, UnicodeError, json.JSONDecodeError) as exc:
        print('NATIVE EXEMPLAR INTAKE FAILED: ' + str(exc), file=sys.stderr)
        return 2


if __name__ == '__main__':
    raise SystemExit(main())
