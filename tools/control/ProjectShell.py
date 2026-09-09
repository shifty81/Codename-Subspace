#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
from datetime import datetime
from pathlib import Path

DANGEROUS_PATTERNS = [
    r"\bgit\s+reset\s+--hard\b",
    r"\bgit\s+clean\b[^\r\n]*(?:-f|-d)",
    r"\bgit\s+push\b[^\r\n]*(?:--force|-f)\b",
    r"\brm\s+-rf\b",
    r"\brmdir\b[^\r\n]*/s\b",
    r"\bdel\b[^\r\n]*/s\b",
    r"\bremove-item\b[^\r\n]*-recurse\b",
    r"\bformat\b[^\r\n]*[a-z]:",
    r"\bdiskpart\b",
    r"\breg\s+delete\b",
]

class ShellError(RuntimeError):
    pass


def discover_root(start: Path) -> Path:
    current = start.resolve()
    if current.is_file(): current = current.parent
    for candidate in [current, *current.parents]:
        if (candidate / 'project.control.json').is_file(): return candidate
    raise ShellError(f'No project.control.json found at or above {start}')


def contract(root: Path) -> dict:
    return json.loads((root/'project.control.json').read_text(encoding='utf-8-sig'))


def log_root(root: Path) -> Path:
    c = contract(root)
    configured = (((c.get('projectOps') or {}).get('artifactLayout') or {}).get('logs')) or 'artifacts/logs'
    p = Path(str(configured)); p = p if p.is_absolute() else root/p
    p = p/'commands'; p.mkdir(parents=True, exist_ok=True); return p


def resolve_shell(kind: str) -> tuple[list[str], str]:
    kind = kind.lower()
    if kind in {'powershell','ps','p'}:
        exe = shutil.which('pwsh') or shutil.which('powershell') or shutil.which('powershell.exe')
        if not exe: raise ShellError('PowerShell was not found on PATH')
        return [exe,'-NoProfile','-ExecutionPolicy','Bypass','-Command'], 'PowerShell'
    if kind in {'cmd','command','c'}:
        exe = shutil.which('cmd') or shutil.which('cmd.exe')
        if not exe: raise ShellError('Command Prompt was not found')
        return [exe,'/d','/s','/c'], 'Command Prompt'
    if kind in {'bash','gitbash','b'}:
        candidates=[shutil.which('bash'),r'C:\Program Files\Git\bin\bash.exe',r'C:\Program Files\Git\usr\bin\bash.exe']
        exe=next((str(p) for p in candidates if p and Path(p).is_file()),None)
        if not exe: raise ShellError('Git Bash was not found')
        return [exe,'-lc'], 'Git Bash'
    raise ShellError(f'Unknown shell: {kind}')


def dangerous(command: str) -> bool:
    return any(re.search(pattern, command, flags=re.IGNORECASE) for pattern in DANGEROUS_PATTERNS)


def confirm(command: str) -> bool:
    if not dangerous(command): return True
    print('\nWARNING: destructive/high-risk operation detected:')
    print(command)
    return input('Type RUN DESTRUCTIVE COMMAND to continue: ').strip() == 'RUN DESTRUCTIVE COMMAND'


def run(root: Path, shell_kind: str, command: str) -> int:
    if not confirm(command): print('Command cancelled.'); return 2
    argv,label=resolve_shell(shell_kind)
    stamp=datetime.now().strftime('%Y%m%d-%H%M%S')
    log=log_root(root)/f"command-{stamp}-{re.sub(r'[^A-Za-z0-9]+','-',label).strip('-').lower()}.log"
    with log.open('w',encoding='utf-8',newline='\n') as stream:
        stream.write(f'PROJECTOPS PROJECT SHELL\nProject: {contract(root).get("name")}\nRoot: {root}\nShell: {label}\nCommand:\n{command}\n--- output ---\n')
        stream.flush()
        print(f'Shell: {label}\nProject: {root}\nLog: {log}\n------------------------------------------------------------------------')
        proc=subprocess.Popen([*argv,command],cwd=str(root),stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,encoding='utf-8',errors='replace',bufsize=1)
        assert proc.stdout is not None
        for line in proc.stdout:
            print(line,end=''); stream.write(line); stream.flush()
        code=proc.wait(); stream.write(f'\n--- exit code: {code} ---\n')
    print('------------------------------------------------------------------------'); print(f'Exit code: {code}'); return code


def open_shell(root: Path, kind: str) -> int:
    if kind == 'powershell':
        exe=(shutil.which('pwsh') or shutil.which('powershell') or shutil.which('powershell.exe'))
        if not exe: raise ShellError('PowerShell not found')
        quoted=str(root).replace("'","''")
        subprocess.Popen([exe,'-NoExit','-Command',f"Set-Location -LiteralPath '{quoted}'"],cwd=str(root)); return 0
    if kind == 'cmd':
        exe=shutil.which('cmd') or shutil.which('cmd.exe')
        if not exe: raise ShellError('cmd.exe not found')
        subprocess.Popen([exe,'/K',f'cd /d "{root}"'],cwd=str(root)); return 0
    if kind == 'bash':
        exe=resolve_shell('bash')[0][0]; subprocess.Popen([exe,'--login','-i'],cwd=str(root)); return 0
    raise ShellError(f'Unknown shell: {kind}')


def history(root: Path) -> list[Path]:
    return sorted(log_root(root).glob('command-*.log'),key=lambda p:p.stat().st_mtime,reverse=True)


def menu(root: Path) -> int:
    name=contract(root).get('name') or root.name
    while True:
        print('\n========================================================================')
        print(f' {str(name).upper()} PROJECT SHELL / COMMAND RUNNER')
        print('========================================================================')
        print(f' Project : {root}')
        print(' 1. Open PowerShell at project root\n 2. Open Command Prompt at project root\n 3. Open Git Bash at project root\n 4. Run one command\n 5. Command history\n 6. Open latest command log\n 0. Back')
        choice=input('Select: ').strip()
        if choice=='0': return 0
        if choice=='1': open_shell(root,'powershell')
        elif choice=='2': open_shell(root,'cmd')
        elif choice=='3': open_shell(root,'bash')
        elif choice=='4':
            shell=input('Shell [P]owerShell/[C]MD/[B]ash (P): ').strip().lower()
            kind={'':'powershell','p':'powershell','c':'cmd','b':'bash'}.get(shell,shell)
            command=input('Command: ').strip()
            if command: run(root,kind,command)
        elif choice=='5':
            for i,p in enumerate(history(root)[:20],1): print(f'{i:2}. {p.name}')
        elif choice=='6':
            files=history(root)
            if not files: print('No command logs yet.')
            elif os.name=='nt': os.startfile(files[0])  # type: ignore[attr-defined]
            else: print(files[0])
        else: print('Unknown selection.')


def main() -> int:
    ap=argparse.ArgumentParser(description='ProjectOps logged project shell')
    ap.add_argument('--root',default='.')
    ap.add_argument('--action',default='menu',choices=['menu','powershell','cmd','bash','run','history','latest'])
    ap.add_argument('--shell',default='powershell'); ap.add_argument('--command')
    args=ap.parse_args(); root=discover_root(Path(args.root))
    if args.action=='menu': return menu(root)
    if args.action in {'powershell','cmd','bash'}: return open_shell(root,args.action)
    if args.action=='run':
        if not args.command: raise ShellError('--command is required for run')
        return run(root,args.shell,args.command)
    files=history(root)
    if args.action=='history':
        for p in files[:20]: print(p); return 0
    if args.action=='latest':
        if not files: return 1
        if os.name=='nt': os.startfile(files[0])  # type: ignore[attr-defined]
        else: print(files[0])
        return 0
    return 1

if __name__=='__main__':
    try: raise SystemExit(main())
    except ShellError as exc:
        print(f'FAIL: {exc}',file=sys.stderr); raise SystemExit(1)
