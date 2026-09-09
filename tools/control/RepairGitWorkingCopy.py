#!/usr/bin/env python3
from __future__ import annotations

import argparse
import importlib.util
import json
import shutil
import subprocess
import sys
from datetime import datetime
from pathlib import Path

class RepairError(RuntimeError):
    pass


def run(cmd: list[str], *, cwd: Path | None = None, check: bool = True, timeout: int = 180) -> subprocess.CompletedProcess[str]:
    cp=subprocess.run(cmd,cwd=str(cwd) if cwd else None,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True,encoding='utf-8',errors='replace',timeout=timeout,check=False)
    if check and cp.returncode != 0:
        raise RepairError((cp.stderr or cp.stdout).strip() or f"Command failed ({cp.returncode}): {' '.join(cmd)}")
    return cp


def git(root: Path,*args: str,check: bool=True,timeout:int=180):
    return run(['git','-C',str(root),*args],check=check,timeout=timeout)


def text(root: Path,*args: str)->str:
    cp=git(root,*args,check=False); return cp.stdout.strip() if cp.returncode==0 else ''


def is_repo(root: Path)->bool: return text(root,'rev-parse','--is-inside-work-tree').lower()=='true'
def has_commit(root: Path)->bool: return git(root,'rev-parse','--verify','HEAD',check=False).returncode==0
def ancestor(root: Path,a:str,b:str)->bool: return git(root,'merge-base','--is-ancestor',a,b,check=False).returncode==0


def load_contract(root: Path)->dict:
    return json.loads((root/'project.control.json').read_text(encoding='utf-8-sig'))


def source_module(root: Path):
    path=root/'tools/control/ProjectSourceAuthority.py'
    spec=importlib.util.spec_from_file_location('project_source_authority',path)
    if spec is None or spec.loader is None: raise RepairError(f'Unable to load source authority: {path}')
    module=importlib.util.module_from_spec(spec); spec.loader.exec_module(module); return module


def source_snapshot(root: Path)->dict:
    module=source_module(root); _,policy=module.resolve_policy(root,None); return module.snapshot(root,policy,include_paths=False)


def green_matches(root: Path,snap:dict)->tuple[bool,str]:
    contract=load_contract(root); state=root/str(contract.get('stateDirectory') or '.projectops')/'last-green-quality-gate.json'
    if not state.is_file(): return False,'No GREEN marker.'
    try: marker=json.loads(state.read_text(encoding='utf-8-sig'))
    except Exception as exc: return False,f'GREEN marker unreadable: {exc}'
    if marker.get('result')!='PASS': return False,'Latest promotion marker is not PASS.'
    if not marker.get('sourceFingerprint'): return False,'GREEN marker has no ProjectOps source fingerprint.'
    if str(marker.get('sourceFingerprint')).lower()!=snap['fingerprint'].lower(): return False,'Governed source changed after GREEN.'
    if int(marker.get('sourcePathCount') or -1)!=int(snap['pathCount']): return False,'Governed source path count changed after GREEN.'
    return True,''


def backup_name(root:Path)->str:
    base='projectops-local-history-backup-'+datetime.now().strftime('%Y%m%d-%H%M%S'); name=base; i=2
    existing=set(text(root,'branch','--format=%(refname:short)').splitlines())
    while name in existing: name=f'{base}-{i}'; i+=1
    return name


def ensure_origin(root:Path,remote:str)->None:
    current=text(root,'remote','get-url','origin')
    if current and current!=remote: git(root,'remote','set-url','origin',remote); print(f'Origin updated: {remote}')
    elif not current: git(root,'remote','add','origin',remote); print(f'Origin added: {remote}')


def main()->int:
    ap=argparse.ArgumentParser(description='ProjectOps safe Git working-folder adoption/repair')
    ap.add_argument('--root',required=True); ap.add_argument('--remote'); ap.add_argument('--branch'); ap.add_argument('--dry-run',action='store_true')
    args=ap.parse_args(); root=Path(args.root).resolve()
    if not root.is_dir(): raise RepairError(f'Project root does not exist: {root}')
    if shutil.which('git') is None: raise RepairError('Git was not found on PATH')
    contract=load_contract(root); repo=contract.get('repository') or {}
    remote=(args.remote or repo.get('remote') or '').strip(); branch=(args.branch or repo.get('branch') or 'main').strip() or 'main'
    if not remote: raise RepairError('No repository remote is configured in project.control.json or --remote.')
    before=source_snapshot(root); green_ok,green_reason=green_matches(root,before)
    print('PROJECTOPS GIT WORKING-FOLDER REPAIR')
    print(f' Project      : {contract.get("name")}\n Root         : {root}\n Remote       : {remote}\n Branch       : {branch}\n GREEN source : {"MATCH" if green_ok else "NOT CERTIFIED"}')
    if args.dry_run: return 0
    if not is_repo(root):
        cp=run(['git','init','-b',branch,str(root)],check=False,timeout=30)
        if cp.returncode!=0: run(['git','init',str(root)],timeout=30); git(root,'branch','-M',branch)
        print('Git repository initialized.')
    current_branch=text(root,'symbolic-ref','--quiet','--short','HEAD')
    if current_branch and current_branch!=branch: git(root,'branch','-M',branch); print(f'Local branch renamed: {current_branch} -> {branch}')
    ensure_origin(root,remote); git(root,'fetch','origin',branch,timeout=240)
    remote_ref=f'origin/{branch}'
    if git(root,'rev-parse','--verify',remote_ref,check=False).returncode!=0: raise RepairError(f'{remote_ref} could not be resolved after fetch')
    remote_head=text(root,'rev-parse',remote_ref); local_head=text(root,'rev-parse','--verify','HEAD') if has_commit(root) else ''
    if not local_head: relation='adopt-remote'
    elif local_head==remote_head: relation='already-aligned'
    elif ancestor(root,local_head,remote_head): relation='fast-forward-adopt'
    elif ancestor(root,remote_head,local_head): relation='keep-local-ahead'
    else: relation='divergent-adopt'
    print(f' Local HEAD   : {local_head or "<unborn>"}\n Remote HEAD  : {remote_head}\n Relationship : {relation}')
    backup=''
    if relation in {'adopt-remote','fast-forward-adopt','divergent-adopt'}:
        if relation=='divergent-adopt':
            if not green_ok: raise RepairError('Local and remote histories diverge and governed working source is not GREEN-certified. '+green_reason)
            backup=backup_name(root); git(root,'branch',backup,local_head); print(f'Backup branch created: {backup}')
        git(root,'reset','--mixed',remote_ref,timeout=180); print('Remote history adopted without overwriting working files.')
    git(root,'branch',f'--set-upstream-to={remote_ref}',branch,check=False)
    after=source_snapshot(root)
    if before['fingerprint']!=after['fingerprint'] or before['pathCount']!=after['pathCount']:
        raise RepairError('Governed working-source bytes changed during Git repair; inspect before continuing.')
    print('WORKING SOURCE PRESERVATION: PASS')
    print(f" Governed paths: {after['pathCount']}\n Fingerprint   : {after['fingerprint']}")
    if backup: print(f' Recovery ref  : {backup}')
    return 0

if __name__=='__main__':
    try: raise SystemExit(main())
    except (RepairError,subprocess.TimeoutExpired) as exc:
        print(f'FAIL: {exc}',file=sys.stderr); raise SystemExit(1)
