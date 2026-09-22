#!/usr/bin/env python3
"""Compile inspected exemplar drafts into a *non-executable* assembly grammar.

No geometry generation, module substitutions, source mutation, native asset
promotion, or runtime eligibility is authorized by this compiler.
"""
from __future__ import annotations

import argparse
from collections import Counter, defaultdict, deque
import json
from pathlib import Path
import sys

from exemplar_intake import atomic_json, load_json

SCHEMA = 'subspace.ship-exemplar-graph-grammar.v1'
CANDIDATE = 'subspace.ship-exemplar-candidate.v1'
AUDIT = 'subspace.ship-exemplar-audit.v1'
MAX_EXEMPLARS = 32


def require(ok, message):
    if not ok:
        raise ValueError(message)


def inspect_pair(candidate, audit):
    require(isinstance(candidate, dict) and candidate.get('schema') == CANDIDATE,
            'expected candidate schema v1')
    require(candidate.get('state') == 'DRAFT_UNCERTIFIED' and candidate.get('generatorEligible') is False,
            'candidate must remain an unpromoted draft')
    require(isinstance(audit, dict) and audit.get('schema') == AUDIT and
            audit.get('promotionAllowed') is False, 'audit must be an unpromoted v1 audit')
    source = candidate.get('source')
    require(isinstance(source, dict) and isinstance(source.get('sha256'), str) and
            len(source['sha256']) == 64 and audit.get('sourceSha256') == source['sha256'],
            'candidate and audit source fingerprints differ')
    require(audit.get('errorCount') == 0 and isinstance(audit.get('blockerCount'), int) and
            audit['blockerCount'] >= 1, 'candidate has structural errors or missing safety blockers')
    issues = audit.get('issues')
    require(isinstance(issues, list) and any(i.get('severity') == 'BLOCKER' for i in issues),
            'missing physical-validation blocker')
    require(not any(i.get('severity') == 'ERROR' for i in issues), 'candidate has structural errors')
    nodes = candidate.get('nodes')
    edges = candidate.get('attachmentEdges')
    require(isinstance(nodes, list) and 1 <= len(nodes) <= 4096 and isinstance(edges, list),
            'bad nodes or edges')
    ids = [n.get('instanceId') for n in nodes]
    require(all(isinstance(i, str) and i for i in ids) and len(ids) == len(set(ids)),
            'invalid or duplicate instance IDs')
    by_id = {n['instanceId']: n for n in nodes}
    roots = candidate.get('rootInstanceIds')
    require(isinstance(roots, list) and len(roots) == 1 and roots[0] in by_id,
            'graph needs one known root')
    require(len(edges) == len(nodes) - 1, 'graph must have exactly N-1 attachment edges')
    children = defaultdict(list)
    parented = set()
    for e in edges:
        require(isinstance(e, dict), 'edge not a map')
        p, c = e.get('parentInstanceId'), e.get('childInstanceId')
        require(p in by_id and c in by_id and p != c, 'invalid graph edge endpoint')
        require(c not in parented, 'multiple parents in graph')
        parented.add(c)
        require(e.get('parentModuleId') == by_id[p].get('moduleId') and
                e.get('childModuleId') == by_id[c].get('moduleId'),
                'edge module identity does not match nodes')
        require(e.get('parentSemantic') == by_id[p].get('semantic') and
                e.get('childSemantic') == by_id[c].get('semantic'),
                'edge semantic identity does not match nodes')
        require(isinstance(e.get('parentSocket'), str) and e['parentSocket'] and
                isinstance(e.get('childSocket'), str) and e['childSocket'],
                'edge lacks named sockets')
        delta = e.get('worldPositionDeltaMeters')
        require(isinstance(delta, list) and len(delta) == 3 and
                all(isinstance(x, (int, float)) and not isinstance(x, bool) and abs(x) <= 1e9 for x in delta),
                'invalid delta')
        children[p].append(c)
    require(parented == set(ids) - set(roots), 'orphans or root has a parent')
    visited = set()
    order = []
    frontier = deque(roots)
    while frontier:
        i = frontier.popleft()
        require(i not in visited, 'cycle found')
        visited.add(i)
        order.append(i)
        frontier.extend(sorted(children[i]))
    require(len(visited) == len(nodes), 'graph disconnected')
    ship = candidate.get('ship')
    require(isinstance(ship, dict) and isinstance(ship.get('role'), str) and ship['role'],
            'ship role is missing')
    require(isinstance(candidate.get('coordinateSystem'), dict) and
            candidate['coordinateSystem'].get('units') == 'meters', 'coordinate mismatch')
    return by_id, order


def compile_grammar(pairs, grammar_id):
    require(isinstance(grammar_id, str) and grammar_id and len(grammar_id) <= 128,
            'invalid grammar ID')
    require(1 <= len(pairs) <= MAX_EXEMPLARS, 'grammar expects 1..32 exemplars')
    role = None
    observations, profiles, classless = [], Counter(), set()
    edge_patterns, exact_patterns, module_options = Counter(), Counter(), defaultdict(set)
    symmetry_pairs = Counter()
    source_hashes = set()
    for candidate, audit in pairs:
        by_id, order = inspect_pair(candidate, audit)
        src = candidate['source']
        sha = src['sha256']
        require(sha not in source_hashes, 'duplicate source exemplar; no double weighting')
        source_hashes.add(sha)
        ship = candidate['ship']
        if role is None:
            role = ship['role']
        require(role == ship['role'], 'do not mix different roles in the same grammar')
        source_type = src.get('schema')
        require(source_type in ('SUBSPACE_SHIP_BLUEPRINT_V1', 'subspace.shipyard_design'),
                'unrecognized source contract')
        node_counts = Counter()
        for n in by_id.values():
            semantic = n.get('semantic', '')
            cls = n.get('moduleClass', '')
            require(isinstance(semantic, str) and isinstance(cls, str), 'invalid node role')
            node_counts[semantic] += 1
            if not semantic or not cls:
                classless.add(sha)
            module_options[(semantic, cls)].add(n['moduleId'])
        profiles.update(node_counts)
        for e in candidate['attachmentEdges']:
            key = (e['parentSemantic'], e['parentSocket'], e['childSemantic'], e['childSocket'])
            edge_patterns[key] += 1
            exact_patterns[(e['parentModuleId'], e['parentSocket'], e['childModuleId'], e['childSocket'])] += 1
        pairs_hint = candidate.get('observations', {}).get('symmetryPairHints', [])
        for hint in pairs_hint:
            if isinstance(hint, list) and len(hint) == 2 and all(i in by_id for i in hint):
                a, b = by_id[hint[0]], by_id[hint[1]]
                symmetry_pairs[(a['semantic'], a['moduleId'])] += 1
        observations.append({'sourceSha256': sha, 'sourceSchema': source_type,
            'shipName': ship['name'], 'nodeCount': len(by_id),
            'edgeCount': len(candidate['attachmentEdges']), 'rootModuleId': by_id[order[0]]['moduleId'],
            'rootSemantic': by_id[order[0]]['semantic'],
            'constructionOrderInstanceIds': order,
            'instanceIdentityPolicy': src.get('identityPolicy', 'EXPORT_INSTANCE_ID_UNVERIFIED'),
            'catalogSha256': src.get('catalogSha256'),
            'reviewBlockers': sorted({i['code'] for i in audit['issues'] if i['severity'] == 'BLOCKER'})})
    observations.sort(key=lambda o: o['sourceSha256'])
    n = len(pairs)
    patterns = [{'parentSemantic': a, 'parentSocket': b, 'childSemantic': c,
                 'childSocket': d, 'observedOccurrences': count,
                 'perExemplarFrequency': round(count / n, 6),
                 'authority': 'OBSERVATION_NOT_SOCKET_GEOMETRY_CERTIFICATION'}
                for (a,b,c,d), count in sorted(edge_patterns.items())]
    exact = [{'parentModuleId': a, 'parentSocket': b, 'childModuleId': c,
              'childSocket': d, 'observedOccurrences': count}
             for (a,b,c,d), count in sorted(exact_patterns.items())]
    catalog_hashes = sorted({o['catalogSha256'] for o in observations if o['catalogSha256']})
    return {'schema': SCHEMA, 'grammarId': grammar_id, 'state': 'PROPOSED_UNCERTIFIED',
        'generatorEligible': False, 'runtimeExecutable': False, 'sourceMutationAllowed': False,
        'role': role, 'sampleCount': n, 'sourceExemplars': observations,
        'requirements': {'humanApproval': True, 'nativeGeometrySocketCertification': True,
            'persistentModuleInstanceIds': True, 'sameCatalogHashRequiredForRuntime': True,
            'shipClassEnvelopeRequired': True, 'interiorAndFunctionTestsRequired': True,
            'sameGeneratorVersionRequestSeedParityRequired': True},
        'observedSemanticCounts': [{'semantic': k, 'totalOccurrences': v,
             'meanPerExemplar': round(v/n, 6)} for k,v in sorted(profiles.items())],
        'moduleOptionsUncertified': [{'semantic': k[0], 'moduleClass': k[1],
             'assetIds': sorted(v), 'substitutionAllowed': False}
             for k,v in sorted(module_options.items())],
        'semanticSocketPatterns': patterns, 'exactAssetSocketPairs': exact,
        'mirroringHints': [{'semantic': k[0], 'moduleId': k[1], 'observedPairs': v}
                           for k,v in sorted(symmetry_pairs.items())],
        'catalogSha256s': catalog_hashes, 'unclassifiedSourceSha256s': sorted(classless),
        'placementNote': 'Only worldPositionDeltaMeters observations exist; no parent-local/socket-relative pose has been certified.',
        'generationNote': 'Patterns are observational. This file CANNOT be installed as a runtime grammar or used to auto-place parts.'}


def main(argv=None):
    p = argparse.ArgumentParser(description='Compile draft attachment patterns; never promote to live PCG')
    p.add_argument('--exemplar-dir', type=Path, action='append', required=True,
                   help='R10 or R11 intake folder; repeat for multiple ships of same role')
    p.add_argument('--grammar-id', required=True)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--overwrite', action='store_true')
    args = p.parse_args(argv)
    try:
        pairs = []
        inputs = []
        for folder in args.exemplar_dir:
            a, b = folder / 'exemplar_candidate.json', folder / 'exemplar_audit.json'
            candidate, _ = load_json(a)
            audit, _ = load_json(b)
            inputs.extend([a.resolve(), b.resolve()])
            pairs.append((candidate, audit))
        target = args.output.resolve()
        if target in inputs or any(inp.parent == target.parent or inp.parent in target.parents for inp in inputs):
            raise ValueError('output must be outside intake folders and must not replace input')
        grammar = compile_grammar(pairs, args.grammar_id)
        atomic_json(target, grammar, overwrite=args.overwrite)
        print('DRAFT GRAMMAR: {} exemplars, {} edge patterns. NOT installable in runtime.'.format(
            len(pairs), len(grammar['semanticSocketPatterns'])))
        return 0
    except (OSError, ValueError, TypeError, UnicodeError, json.JSONDecodeError) as exc:
        print('EXEMPLAR GRAMMAR FAILED: ' + str(exc), file=sys.stderr)
        return 2


if __name__ == '__main__':
    raise SystemExit(main())
