#!/usr/bin/env python3
"""Build a review-only authored-ship learning bundle in one command.

The pipeline intentionally reuses the certified native intake and graph compiler.
It never promotes grammar into runtime PCG, never mutates the source ship, and
records hashes/provenance so a later approval step can be audited.
"""
from __future__ import annotations
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import sys
from datetime import datetime, timezone

HERE=Path(__file__).resolve().parent
sys.path.insert(0,str(HERE))
import native_blueprint_intake as native
import exemplar_graph_compiler as graph

SCHEMA='subspace.exemplar-review-bundle.v1'

def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()

def write_json(path: Path, data: dict):
    path.parent.mkdir(parents=True,exist_ok=True)
    tmp=path.with_suffix(path.suffix+'.tmp')
    tmp.write_text(json.dumps(data,indent=2,sort_keys=True)+'\n',encoding='utf-8')
    tmp.replace(path)

def main(argv=None) -> int:
    p=argparse.ArgumentParser(description='Create a review-only native exemplar learning bundle')
    p.add_argument('--ship',type=Path,required=True)
    p.add_argument('--catalog',type=Path)
    p.add_argument('--out-dir',type=Path,required=True)
    p.add_argument('--grammar-id',required=True)
    p.add_argument('--overwrite',action='store_true')
    args=p.parse_args(argv)
    try:
        ship=args.ship.resolve();out=args.out_dir.resolve()
        if not ship.is_file(): raise ValueError('ship source does not exist')
        if out==ship.parent or ship.parent in out.parents: raise ValueError('review output must be outside the source ship directory')
        if out.exists():
            if not args.overwrite: raise FileExistsError('output exists; --overwrite required')
            if out.is_file(): raise ValueError('output path is a file')
            shutil.rmtree(out)
        intake=out/'intake';grammar=out/'grammar'/'family_grammar.json'
        rc=native.main(['--input',str(ship),'--out-dir',str(intake)] + (['--catalog',str(args.catalog.resolve())] if args.catalog else []))
        if rc!=0: raise RuntimeError(f'native intake failed with code {rc}')
        rc=graph.main(['--exemplar-dir',str(intake),'--grammar-id',args.grammar_id,'--output',str(grammar)])
        if rc!=0: raise RuntimeError(f'graph compiler failed with code {rc}')
        candidate=intake/'exemplar_candidate.json';audit=intake/'exemplar_audit.json'
        manifest={
            'schema':SCHEMA,
            'state':'REVIEW_ONLY_UNCERTIFIED',
            'promotionAllowed':False,
            'createdUtc':datetime.now(timezone.utc).isoformat(),
            'grammarId':args.grammar_id,
            'sourceShip':{'name':ship.name,'sha256':sha256(ship)},
            'artifacts':[
                {'path':'intake/exemplar_candidate.json','sha256':sha256(candidate)},
                {'path':'intake/exemplar_audit.json','sha256':sha256(audit)},
                {'path':'grammar/family_grammar.json','sha256':sha256(grammar)},
            ],
            'nextAuthority':'Studio physical geometry/socket/interior/function validation + explicit designer approval',
            'runtimeInstallAllowed':False,
        }
        write_json(out/'review_manifest.json',manifest)
        print(f"EXEMPLAR REVIEW BUNDLE: {out}")
        print('REVIEW ONLY: no runtime PCG promotion performed.')
        return 0
    except (OSError,ValueError,RuntimeError,TypeError,json.JSONDecodeError) as exc:
        print('EXEMPLAR REVIEW PIPELINE FAILED: '+str(exc),file=sys.stderr)
        return 2

if __name__=='__main__': raise SystemExit(main())
