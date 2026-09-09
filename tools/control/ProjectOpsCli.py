#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import os
import shutil
import subprocess
import sys
from datetime import datetime
from pathlib import Path
from typing import Any

class ProjectOpsError(RuntimeError):
    pass


def discover_root(start: Path) -> Path:
    current = start.resolve()
    if current.is_file():
        current = current.parent
    for candidate in [current, *current.parents]:
        if (candidate / 'project.control.json').is_file():
            return candidate
    raise ProjectOpsError(f'No project.control.json found at or above {start}')


def load_contract(root: Path) -> dict[str, Any]:
    try:
        contract = json.loads((root / 'project.control.json').read_text(encoding='utf-8-sig'))
    except Exception as exc:
        raise ProjectOpsError(f'Unable to load project.control.json: {exc}') from exc
    if not contract.get('id') or not contract.get('name'):
        raise ProjectOpsError('project.control.json must contain id and name.')
    commands = contract.get('commands')
    if not isinstance(commands, list):
        raise ProjectOpsError('project.control.json commands must be an array.')
    seen: set[str] = set()
    for command in commands:
        key = str(command.get('key') or '').strip()
        if not key:
            raise ProjectOpsError('Command registry contains a blank key.')
        lower = key.lower()
        if lower in seen:
            raise ProjectOpsError(f'Duplicate command key: {key}')
        seen.add(lower)
    return contract


def artifact_root(root: Path, contract: dict[str, Any]) -> Path:
    configured = (((contract.get('projectOps') or {}).get('artifactLayout') or {}).get('root')) or 'artifacts'
    path = Path(str(configured))
    return path if path.is_absolute() else root / path


def command_log_root(root: Path, contract: dict[str, Any]) -> Path:
    layout = ((contract.get('projectOps') or {}).get('artifactLayout') or {})
    logs = Path(str(layout.get('logs') or 'artifacts/logs'))
    if not logs.is_absolute():
        logs = root / logs
    path = logs / 'commands'
    path.mkdir(parents=True, exist_ok=True)
    return path


def find_command(contract: dict[str, Any], key: str) -> dict[str, Any]:
    for command in contract.get('commands') or []:
        if str(command.get('key') or '').lower() == key.lower():
            return command
    raise ProjectOpsError(f'Unknown project command: {key}')


def resolve_executable(raw: str, root: Path) -> list[str]:
    token = raw.strip()
    lower = token.lower()
    if lower in {'pwsh','powershell'}:
        exe = shutil.which('pwsh') or shutil.which('powershell') or shutil.which('powershell.exe')
        if not exe:
            raise ProjectOpsError('PowerShell was not found on PATH.')
        return [exe]
    if lower in {'python','python3'}:
        exe = shutil.which('python') or shutil.which('python3')
        if exe:
            return [exe]
        py = shutil.which('py')
        if py:
            return [py, '-3']
        raise ProjectOpsError('Python was not found on PATH.')
    candidate = Path(token)
    if not candidate.is_absolute():
        project_candidate = root / candidate
        if project_candidate.exists():
            return [str(project_candidate)]
    exe = shutil.which(token)
    if exe:
        return [exe]
    raise ProjectOpsError(f'Executable not found: {token}')


def normalized_arguments(command: dict[str, Any], root: Path) -> list[str]:
    args: list[str] = []
    for value in command.get('arguments') or []:
        text = str(value).replace('${PROJECT_ROOT}', str(root))
        args.append(text)
    return args


def run_registered(root: Path, contract: dict[str, Any], command: dict[str, Any], assume_yes: bool) -> int:
    if bool(command.get('requiresConfirmation')) and not assume_yes:
        prompt = f"Type EXECUTE {command['key']} to continue: "
        if input(prompt).strip() != f"EXECUTE {command['key']}":
            print('Command cancelled.')
            return 2
    argv = [*resolve_executable(str(command.get('executable') or ''), root), *normalized_arguments(command, root)]
    stamp = datetime.now().strftime('%Y%m%d-%H%M%S')
    safe_key = ''.join(c if c.isalnum() or c in '-_.' else '-' for c in str(command['key']))
    log = command_log_root(root, contract) / f'projectops-{stamp}-{safe_key}.log'
    env = os.environ.copy()
    env['PROJECTOPS_ROOT'] = str(root)
    env['PROJECTOPS_COMMAND_KEY'] = str(command['key'])
    env['PROJECTOPS_PROJECT_ID'] = str(contract['id'])
    with log.open('w', encoding='utf-8', newline='\n') as stream:
        stream.write(f"PROJECTOPS COMMAND\nProject: {contract['name']}\nRoot: {root}\nKey: {command['key']}\nCommand: {argv!r}\n--- output ---\n")
        stream.flush()
        print(f"PROJECTOPS EXECUTE: {command['key']} - {command.get('label','')}")
        print(f"Log: {log}")
        process = subprocess.Popen(argv, cwd=str(root), env=env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, encoding='utf-8', errors='replace', bufsize=1)
        assert process.stdout is not None
        for line in process.stdout:
            print(line, end='')
            stream.write(line)
            stream.flush()
        code = process.wait()
        stream.write(f"\n--- exit code: {code} ---\n")
    return code


def latest_files(path: Path, limit: int = 20) -> list[dict[str, Any]]:
    if not path.exists():
        return []
    rows = [p for p in path.rglob('*') if p.is_file()]
    rows.sort(key=lambda p: p.stat().st_mtime, reverse=True)
    return [{'path': str(p), 'bytes': p.stat().st_size, 'modified': datetime.fromtimestamp(p.stat().st_mtime).isoformat()} for p in rows[:limit]]


def emit(payload: Any, as_json: bool) -> None:
    if as_json:
        print(json.dumps(payload, indent=2, ensure_ascii=False))
    elif isinstance(payload, list):
        for item in payload:
            if isinstance(item, dict):
                print(f"{item.get('key',''):<32} {item.get('label','')}")
            else:
                print(item)
    elif isinstance(payload, dict):
        for key, value in payload.items():
            print(f'{key}: {value}')
    else:
        print(payload)


def main() -> int:
    ap = argparse.ArgumentParser(description='ProjectOps machine-facing project control CLI')
    ap.add_argument('--root', default='.')
    ap.add_argument('--json', action='store_true')
    sub = ap.add_subparsers(dest='action', required=True)
    sub.add_parser('inspect')
    commands = sub.add_parser('commands'); commands.add_argument('--category')
    desc = sub.add_parser('describe'); desc.add_argument('key')
    exe = sub.add_parser('execute'); exe.add_argument('key'); exe.add_argument('--yes', action='store_true')
    sub.add_parser('gate')
    artifacts = sub.add_parser('artifacts'); artifacts.add_argument('--limit', type=int, default=20)
    sub.add_parser('validate')
    args = ap.parse_args()

    root = discover_root(Path(args.root))
    contract = load_contract(root)
    if args.action == 'inspect':
        payload = {
            'id': contract['id'], 'name': contract['name'], 'type': contract.get('type'),
            'root': str(root), 'runtimeAuthority': contract.get('runtimeAuthority'),
            'rootLauncher': contract.get('rootLauncher'), 'commandCount': len(contract.get('commands') or []),
            'repository': contract.get('repository'), 'projectOps': contract.get('projectOps'),
        }
        emit(payload, args.json); return 0
    if args.action == 'commands':
        rows = sorted(contract.get('commands') or [], key=lambda x: (str(x.get('category') or ''), str(x.get('key') or '')))
        if args.category:
            rows = [row for row in rows if str(row.get('category') or '').lower() == args.category.lower()]
        emit(rows, args.json); return 0
    if args.action == 'describe':
        emit(find_command(contract, args.key), args.json); return 0
    if args.action == 'execute':
        return run_registered(root, contract, find_command(contract, args.key), args.yes)
    if args.action == 'gate':
        marker = root / str(contract.get('stateDirectory') or '.projectops') / 'last-green-quality-gate.json'
        payload = json.loads(marker.read_text(encoding='utf-8-sig')) if marker.is_file() else {'result':'NONE','path':str(marker)}
        emit(payload, args.json); return 0
    if args.action == 'artifacts':
        payload = {'root': str(artifact_root(root, contract)), 'latest': latest_files(artifact_root(root, contract), args.limit)}
        emit(payload, args.json); return 0
    if args.action == 'validate':
        project_ops = contract.get('projectOps') or {}
        required = [
            'tools/control/ProjectOpsCommon.psm1',
            'tools/control/ProjectCommandRegistry.ps1',
            'tools/control/ProjectSourceAuthority.py',
            'tools/control/ProjectOpsRootAudit.ps1',
            'tools/control/ProjectOpsMaintenance.ps1',
            'tools/control/ProjectOpsNormalizationAudit.ps1',
            'tools/control/ProjectShell.py',
            'tools/control/RepairGitWorkingCopy.py',
            'tools/control/UniversalTreeStage.psm1',
        ]
        missing = [path for path in required if not (root / path).is_file()]
        root_policy = project_ops.get('rootPolicy') or {}
        payload = {
            'valid': not missing and bool(root_policy),
            'project': contract['id'],
            'commands': len(contract.get('commands') or []),
            'artifactRoot': str(artifact_root(root, contract)),
            'rootPolicy': bool(root_policy),
            'missingAuthorities': missing,
        }
        emit(payload, args.json); return 0 if payload['valid'] else 1
    return 1

if __name__ == '__main__':
    try:
        raise SystemExit(main())
    except ProjectOpsError as exc:
        print(f'FAIL: {exc}', file=sys.stderr)
        raise SystemExit(1)
