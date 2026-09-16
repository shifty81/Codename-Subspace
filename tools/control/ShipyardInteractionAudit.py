#!/usr/bin/env python3
"""Read-only Shipyard command/GUI wiring audit. Standard library only.

This tool is intentionally independent of the game's source revision: it does not
modify or execute the project, claim that UI controls work, or certify GREEN.
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from collections import Counter
from pathlib import Path

ENUM_HEADER = Path('engine/include/ship_editor/ShipyardBuilderSystem.h')
SRC_ROOT = Path('engine/src')
COMMAND_RE = re.compile(r'\bShipyardBuilderCommand::([A-Za-z_]\w*)\b')
CASE_RE = re.compile(r'\bcase\s+ShipyardBuilderCommand::([A-Za-z_]\w*)\s*:')
DECL_RE = re.compile(r'\benum\s+class\s+ShipyardBuilderCommand\s*\{')


def strip_comments(text: str) -> str:
    """Remove C++ comments without corrupting quote contents or line numbers."""
    pattern = re.compile(r'("(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|//[^\n]*|/\*[\s\S]*?\*/)')
    return pattern.sub(lambda m: '\n' * m.group().count('\n') if m.group().startswith(('//', '/*')) else m.group(), text)


def get_enum(text: str) -> list[str]:
    clean = strip_comments(text)
    match = DECL_RE.search(clean)
    if not match:
        raise ValueError('ShipyardBuilderCommand enum not found; cannot produce authoritative command inventory')
    start = match.end()
    depth = 1
    cursor = start
    while cursor < len(clean) and depth:
        if clean[cursor] == '{':
            depth += 1
        elif clean[cursor] == '}':
            depth -= 1
        cursor += 1
    if depth:
        raise ValueError('Unterminated ShipyardBuilderCommand enum')
    body = clean[start:cursor - 1]
    names = []
    for item in body.split(','):
        item = item.split('=', 1)[0].strip()
        if item:
            if not re.fullmatch(r'[A-Za-z_]\w*', item):
                raise ValueError(f'Cannot parse enum entry: {item[:80]}')
            names.append(item)
    if len(names) != len(set(names)):
        raise ValueError('Duplicate ShipyardBuilderCommand enum name')
    return names


def source_files(root: Path) -> list[Path]:
    base = root / SRC_ROOT
    if not base.is_dir():
        raise FileNotFoundError(f'Missing native source directory: {base}')
    return sorted(p for p in base.rglob('*.cpp') if p.is_file() and not p.is_symlink())


def line_number(text: str, offset: int) -> int:
    return text.count('\n', 0, offset) + 1


def audit(root: Path) -> dict:
    header = root / ENUM_HEADER
    if not header.is_file():
        raise FileNotFoundError(f'Missing Shipyard command header: {header}')
    names = get_enum(header.read_text(encoding='utf-8-sig'))
    occurrences: dict[str, list[dict]] = {name: [] for name in names}
    undeclared: dict[str, list[dict]] = {}
    status_only_candidates: list[dict] = []
    all_files = source_files(root)
    for path in all_files:
        relative = path.relative_to(root).as_posix()
        text = strip_comments(path.read_text(encoding='utf-8-sig', errors='replace'))
        # One-line switch bodies that only set status and return true are suspicious,
        # not proof of a bug: higher-level callers may still perform the action.
        for match in re.finditer(r'case\s+ShipyardBuilderCommand::([A-Za-z_]\w*)\s*:\s*([^\n]+)', text):
            body = match.group(2).strip()
            if re.fullmatch(r'(?:model_\.status|status_)\s*=\s*[^;]+;\s*return\s+true\s*;', body):
                status_only_candidates.append({'command': match.group(1),
                    'file': relative, 'line': line_number(text, match.start()),
                    'reason': 'case only updates status and returns true; verify side effect in runtime'})
        for m in COMMAND_RE.finditer(text):
            name = m.group(1)
            near = text[max(0, m.start()-65):m.start()]
            is_case = bool(re.search(r'\bcase\s*$', near))
            evidence = {'file': relative, 'line': line_number(text, m.start()),
                        'usage': 'handler_case' if is_case and '/ship_editor/' in relative else
                        'other_case' if is_case else 'reference'}
            (occurrences if name in occurrences else undeclared).setdefault(name, []).append(evidence)
    rows = []
    for name in names:
        refs = occurrences[name]
        handler = [x for x in refs if x['usage'] == 'handler_case']
        other_cases = [x for x in refs if x['usage'] == 'other_case']
        non_case = [x for x in refs if x['usage'] == 'reference']
        if name == 'None':
            status = 'sentinel'
        elif not refs:
            status = 'no_source_reference'
        elif not handler:
            status = 'manual_review_no_ship_editor_handler_case'
        else:
            status = 'case_present_behavior_unverified'
        rows.append({'command': name, 'status': status, 'ship_editor_cases': len(handler),
                     'other_cases': len(other_cases), 'references': len(non_case),
                     'evidence': refs[:16], 'evidenceTruncated': len(refs) > 16})
    # This is intentionally not a visual geometry or behavior test.
    summary = dict(Counter(row['status'] for row in rows))
    return {'schemaVersion': 1, 'auditType': 'shipyard_static_interaction_inventory',
            'sourceRoot': str(root), 'header': ENUM_HEADER.as_posix(),
            'sourceFileCount': len(all_files), 'commandCount': len(names),
            'summary': summary, 'undeclaredReferences': undeclared,
            'statusOnlyCandidates': status_only_candidates,
            'limitations': [
                'A switch case is source evidence, NOT proof that a UI control works.',
                'Non-switch dispatch, header-inline handlers and generated code require manual review.',
                'No actual window was launched: hit boxes, docking, tooltips and save/load are unverified.',
                'This scan never changes project source or PCC certification state.'
            ], 'commands': rows}


def markdown(report: dict) -> str:
    s = report['summary']
    lines = ['# Shipyard interaction audit (static evidence only)', '',
             f"Commands: {report['commandCount']} | C++ source files scanned: {report['sourceFileCount']}", '',
             '> A handler case does not mean a button is functional. This is a review queue, not a certification.', '',
             '| Evidence class | Count |', '|---|---:|']
    for status, count in sorted(s.items()):
        lines.append(f'| {status} | {count} |')
    lines += ['', '## Review queue', '', '| Command | Status | Location |', '|---|---|---|']
    for entry in report['commands']:
        if entry['status'] in ('sentinel', 'case_present_behavior_unverified'):
            continue
        loc = entry['evidence'][0] if entry['evidence'] else None
        where = f"`{loc['file']}:{loc['line']}`" if loc else 'No source reference'
        lines.append(f"| `{entry['command']}` | {entry['status']} | {where} |")
    if report['statusOnlyCandidates']:
        lines += ['', '## Potential status-only actions (requires manual runtime review)', '']
        for candidate in report['statusOnlyCandidates']:
            lines.append(f"- `{candidate['command']}` — `{candidate['file']}:{candidate['line']}`: {candidate['reason']}")
    if report['undeclaredReferences']:
        lines += ['', '## References to undeclared commands', '']
        for name, refs in sorted(report['undeclaredReferences'].items()):
            lines.append(f"- `{name}`: " + ', '.join(f"`{r['file']}:{r['line']}`" for r in refs[:5]))
    lines += ['', '## Acceptance still required', '',
              '1. Instrument actual BuildControls/HitTest on the running PASS1505R1 game and record visible/enabled/hover/click/command results.',
              '2. Verify action results and undo/redo in runtime; separately certify rendered hitboxes and overlapping rectangles.',
              '3. Record docking, save/reopen, articulated attachments, interiors and material audit with real assets.',
              '', '## Limitations', '']
    lines.extend(f'- {x}' for x in report['limitations'])
    return '\n'.join(lines) + '\n'


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path.cwd(), help='Repository root (read-only)')
    parser.add_argument('--out', type=Path, help='Explicit output directory (default: artifacts/reports/shipyard-interaction)')
    parser.add_argument('--strict', action='store_true', help='Fail only for undeclared references or commands with zero source references')
    args = parser.parse_args(argv)
    root = args.root.resolve()
    try:
        report = audit(root)
        dest = (args.out or root / 'artifacts/reports/shipyard-interaction').resolve()
        if dest == root or dest in (root / 'engine', root / 'tools'):
            raise ValueError('Output must be a report directory, not the source root')
        dest.mkdir(parents=True, exist_ok=True)
        (dest / 'interaction_audit.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
        (dest / 'interaction_audit.md').write_text(markdown(report), encoding='utf-8')
        print(f"PASS1506 inventory: {report['commandCount']} commands, {report['sourceFileCount']} source files")
        print('Review classes:', json.dumps(report['summary'], sort_keys=True))
        print("Reports: " + str(dest / "interaction_audit.json") + " ; " + str(dest / "interaction_audit.md"))
        if args.strict and (report['undeclaredReferences'] or report['summary'].get('no_source_reference', 0)):
            return 2
        return 0
    except (OSError, ValueError) as exc:
        print(f'AUDIT ERROR: {exc}', file=sys.stderr)
        return 1


if __name__ == '__main__':
    raise SystemExit(main())
