#!/usr/bin/env python3
"""R82R2 targeted gate repair.

Fixes one post-R82R1 WYSIWYG placement bug: a snapped staged duplicate preview
contained the authoritative normalized snap geometry plus preserved material
state, but CommitCatalogDrag() re-read candidate.placement and discarded those
preview-owned properties. The commit now writes the exact staged ghost while
keeping the selected candidate only for attachment metadata.

Exact-preimage, idempotent, transactional, and fail-closed.
"""
from __future__ import annotations
import argparse, hashlib, json, os, shutil, sys
from datetime import datetime, timezone
from pathlib import Path

def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()

def main() -> int:
    ap=argparse.ArgumentParser();ap.add_argument('--root',required=True);ap.add_argument('--apply',action='store_true');args=ap.parse_args()
    root=Path(args.root).resolve();p=root/'engine/src/ship_editor/ShipyardBuilderSystem.cpp'
    if not p.is_file(): raise RuntimeError(f'R82R2 missing source: {p}')
    src=p.read_text(encoding='utf-8')
    if 'BeginDuplicateSelectedPlacement()' not in src or 'DeleteSelectionSafe' not in src:
        raise RuntimeError('R82R2 prerequisite missing: R63-R78 selection/placement source is not present')
    old='''    if(preview.snapped&&preview.selectedCandidate>=0){\n        const auto c=preview.candidates[static_cast<std::size_t>(preview.selectedCandidate)];\n        model_.recipe.modules.push_back(c.placement);\n        model_.recipe.attachments.push_back({c.parentModuleIndex,childIndex,c.parentSocket,c.childSocket,0.0f,true});'''
    new='''    if(preview.snapped&&preview.selectedCandidate>=0){\n        const auto c=preview.candidates[static_cast<std::size_t>(preview.selectedCandidate)];\n        // The staged ghost is the WYSIWYG placement authority. Candidate metadata\n        // still owns the socket edge, but commit must not discard preview-owned\n        // material/source-material state or any solver-normalized geometry.\n        model_.recipe.modules.push_back(preview.ghost);\n        model_.recipe.attachments.push_back({c.parentModuleIndex,childIndex,c.parentSocket,c.childSocket,0.0f,true});'''
    if new in src:
        print('R82R2 gate repair already present; no mutation required.');return 0
    count=src.count(old)
    if count!=1: raise RuntimeError(f'R82R2 snapped commit preimage: expected one approved match, found {count}')
    changed=src.replace(old,new,1)
    if 'model_.recipe.modules.push_back(preview.ghost);' not in changed:
        raise RuntimeError('R82R2 postcondition missing staged ghost commit authority')
    if not args.apply:
        print('R82R2 DRY RUN PASS; ShipyardBuilderSystem.cpp requires snapped commit repair.');return 0
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%SZ')
    backup=root/'artifacts/gates/migrations/studio_r82r2'/stamp
    backup_file=backup/'engine/src/ship_editor/ShipyardBuilderSystem.cpp';backup_file.parent.mkdir(parents=True,exist_ok=False)
    shutil.copy2(p,backup_file)
    receipt={'schema':'subspace.studio-r82r2-migration.v1','timestampUtc':stamp,'files':[{
        'path':'engine/src/ship_editor/ShipyardBuilderSystem.cpp','beforeSha256':sha256(src.encode('utf-8'))}]}
    try:
        tmp=p.with_name(p.name+'.r82r2.tmp');tmp.write_text(changed,encoding='utf-8',newline='\n');os.replace(tmp,p)
        receipt['files'][0]['afterSha256']=sha256(p.read_bytes())
        (backup/'receipt.json').write_text(json.dumps(receipt,indent=2)+'\n',encoding='utf-8')
    except Exception:
        shutil.copy2(backup_file,p);raise
    print('R82R2 snapped commit repair applied transactionally to ShipyardBuilderSystem.cpp.')
    print(f'Backup/receipt: {backup}')
    return 0

if __name__=='__main__':
    try: raise SystemExit(main())
    except Exception as exc:
        print(f'R82R2 NORMALIZATION BLOCKED: {exc}',file=sys.stderr);raise SystemExit(2)
