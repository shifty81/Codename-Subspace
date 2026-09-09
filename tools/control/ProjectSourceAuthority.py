#!/usr/bin/env python3
from __future__ import annotations

import argparse
import fnmatch
import hashlib
import json
import os
import sys
from pathlib import Path
from typing import Any

DEFAULT_POLICY_REL = Path('content/architecture/projectops_source_authority_v1.json')

class SourceAuthorityError(RuntimeError):
    pass


def normalize_rel(value: str) -> str:
    value = value.replace('\\', '/')
    while value.startswith('./'):
        value = value[2:]
    return value.lstrip('/')


def discover_root(start: Path) -> Path:
    current = start.resolve()
    if current.is_file():
        current = current.parent
    for candidate in [current, *current.parents]:
        if (candidate / 'project.control.json').is_file():
            return candidate
    raise SourceAuthorityError(f'No project.control.json found at or above {start}')


def load_project(root: Path) -> dict[str, Any]:
    try:
        return json.loads((root / 'project.control.json').read_text(encoding='utf-8-sig'))
    except Exception as exc:
        raise SourceAuthorityError(f'Unable to read project.control.json: {exc}') from exc


def resolve_policy(root: Path, explicit: str | None = None) -> tuple[Path, dict[str, Any]]:
    if explicit:
        path = Path(explicit)
        if not path.is_absolute():
            path = root / path
    else:
        project = load_project(root)
        configured = (((project.get('projectOps') or {}).get('sourceAuthority') or {}).get('contract'))
        path = root / Path(configured or DEFAULT_POLICY_REL)
    if not path.is_file():
        raise SourceAuthorityError(f'Source authority contract is missing: {path}')
    try:
        payload = json.loads(path.read_text(encoding='utf-8-sig'))
    except Exception as exc:
        raise SourceAuthorityError(f'Source authority contract is invalid JSON: {path}: {exc}') from exc
    if not payload.get('id'):
        raise SourceAuthorityError('Source authority contract has no id.')
    return path, payload


def prefix_matches(rel: str, prefix: str) -> bool:
    rel_l = normalize_rel(rel).lower().rstrip('/')
    pre_l = normalize_rel(prefix).lower().rstrip('/')
    return rel_l == pre_l or rel_l.startswith(pre_l + '/')


def ignored(rel: str, policy: dict[str, Any]) -> bool:
    rel = normalize_rel(rel)
    lower = rel.lower()
    for prefix in policy.get('ignorePrefixes') or []:
        if prefix_matches(lower, str(prefix)):
            return True
    parts = [p.lower() for p in lower.split('/') if p]
    ignored_names = {str(x).lower() for x in (policy.get('ignoreDirectoryNames') or [])}
    if any(part in ignored_names for part in parts[:-1]):
        return True
    name = Path(lower).name
    if name in {str(x).lower() for x in (policy.get('ignoreFileNames') or [])}:
        return True
    if Path(name).suffix.lower() in {str(x).lower() for x in (policy.get('ignoreExtensions') or [])}:
        return True
    for pattern in policy.get('ignoreNamePatterns') or []:
        if fnmatch.fnmatch(name, str(pattern).lower()):
            return True
    for exact in policy.get('ignoreExactPaths') or []:
        if lower == normalize_rel(str(exact)).lower():
            return True
    return False


def iter_tree(root: Path, base: Path, policy: dict[str, Any]):
    if not base.exists():
        return
    if base.is_file():
        rel = normalize_rel(str(base.relative_to(root)))
        if not ignored(rel, policy):
            yield rel, base
        return
    for current, dirs, files in os.walk(base, followlinks=False):
        cur = Path(current)
        kept: list[str] = []
        for name in dirs:
            child = cur / name
            rel = normalize_rel(str(child.relative_to(root)))
            if child.is_symlink() or ignored(rel + '/', policy):
                continue
            kept.append(name)
        dirs[:] = kept
        for name in files:
            path = cur / name
            if path.is_symlink():
                continue
            rel = normalize_rel(str(path.relative_to(root)))
            if ignored(rel, policy):
                continue
            yield rel, path


def governed_paths(root: Path, policy: dict[str, Any]) -> list[str]:
    found: dict[str, Path] = {}
    for rel in policy.get('rootFiles') or []:
        path = root / Path(str(rel))
        if path.is_file() and not path.is_symlink() and not ignored(str(rel), policy):
            found[normalize_rel(str(rel))] = path
    for rel_root in policy.get('includeRoots') or []:
        base = root / Path(str(rel_root))
        for rel, path in iter_tree(root, base, policy) or []:
            found[rel] = path
    # Required bootstrap files are always governed, even when a policy author
    # accidentally forgets to list their parent root.
    for rel in policy.get('requiredBootstrapPaths') or []:
        path = root / Path(str(rel))
        if path.is_file() and not path.is_symlink():
            found[normalize_rel(str(rel))] = path
    return sorted(found.keys(), key=lambda x: (x.lower(), x))


def digest(path: Path) -> str:
    h = hashlib.sha256()
    with path.open('rb') as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()


def validate_required(root: Path, policy: dict[str, Any], paths: list[str]) -> list[str]:
    path_set = set(paths)
    errors: list[str] = []
    for raw in policy.get('requiredBootstrapPaths') or []:
        rel = normalize_rel(str(raw))
        if not (root / Path(rel)).is_file():
            errors.append(f'missing required bootstrap file: {rel}')
        elif rel not in path_set:
            errors.append(f'required bootstrap file absent from governed set: {rel}')
    return errors


def snapshot(root: Path, policy: dict[str, Any], *, include_paths: bool = False) -> dict[str, Any]:
    paths = governed_paths(root, policy)
    errors = validate_required(root, policy, paths)
    if errors:
        raise SourceAuthorityError('Clean-checkout source contract failed:\n' + '\n'.join(f' - {x}' for x in errors))
    h = hashlib.sha256()
    entries: list[dict[str, Any]] = []
    for rel in paths:
        path = root / Path(rel)
        file_hash = digest(path)
        h.update(rel.encode('utf-8', 'surrogateescape'))
        h.update(b'\0FILE\0')
        h.update(file_hash.encode('ascii'))
        h.update(b'\n')
        if include_paths:
            entries.append({'path': rel, 'sha256': file_hash, 'bytes': path.stat().st_size})
    result: dict[str, Any] = {
        'authorityId': str(policy.get('id')),
        'fingerprint': h.hexdigest(),
        'pathCount': len(paths),
        'requiredBootstrapPaths': [normalize_rel(str(x)) for x in (policy.get('requiredBootstrapPaths') or [])],
        'cleanCheckoutContract': 'PASS',
    }
    if include_paths:
        result['files'] = entries
    return result


def emit(payload: Any, as_json: bool) -> None:
    if as_json:
        print(json.dumps(payload, indent=2, ensure_ascii=False))
    else:
        if isinstance(payload, dict):
            for key, value in payload.items():
                if key == 'files':
                    continue
                print(f'{key}: {value}')
        else:
            print(payload)


def main() -> int:
    ap = argparse.ArgumentParser(description='ProjectOps filesystem-owned source authority')
    ap.add_argument('--root', default='.')
    ap.add_argument('--policy')
    sub = ap.add_subparsers(dest='action', required=True)
    snap = sub.add_parser('snapshot')
    snap.add_argument('--json', action='store_true')
    snap.add_argument('--include-paths', action='store_true')
    val = sub.add_parser('validate')
    val.add_argument('--expected', required=True)
    val.add_argument('--expected-count', type=int)
    val.add_argument('--json', action='store_true')
    contract = sub.add_parser('validate-contract')
    contract.add_argument('--json', action='store_true')
    args = ap.parse_args()

    root = discover_root(Path(args.root))
    policy_path, policy = resolve_policy(root, args.policy)

    if args.action == 'validate-contract':
        result = snapshot(root, policy, include_paths=False)
        result['policyPath'] = normalize_rel(str(policy_path.relative_to(root)))
        emit(result, args.json)
        return 0

    result = snapshot(root, policy, include_paths=getattr(args, 'include_paths', False))
    if args.action == 'validate':
        ok = result['fingerprint'].lower() == args.expected.lower()
        if args.expected_count is not None:
            ok = ok and result['pathCount'] == args.expected_count
        result['matchesExpected'] = ok
        emit(result, args.json)
        return 0 if ok else 2

    emit(result, args.json)
    return 0


if __name__ == '__main__':
    try:
        raise SystemExit(main())
    except SourceAuthorityError as exc:
        print(f'FAIL: {exc}', file=sys.stderr)
        raise SystemExit(1)
