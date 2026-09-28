#!/usr/bin/env python3
from __future__ import annotations
import argparse, hashlib, json, os, re, shutil, sys
from datetime import datetime, timezone
from pathlib import Path
from studio_verified_normalized_state import verify_complete_r82r1

def sha256(data: bytes)->str:return hashlib.sha256(data).hexdigest()
def require(text:str,token:str,label:str):
    if token not in text: raise RuntimeError(f"{label}: required token missing: {token}")
def replace_exact(text:str,old:str,new:str,label:str):
    if new in text:return text,False
    n=text.count(old)
    if n!=1:raise RuntimeError(f"{label}: expected one approved preimage, found {n}")
    return text.replace(old,new,1),True
def replace_regex(text:str,pattern:str,new:str,label:str):
    if new in text:return text,False
    out,n=re.subn(pattern,new,text,count=1,flags=re.S)
    if n!=1:raise RuntimeError(f"{label}: expected one approved preimage, found {n}")
    return out,True

def main()->int:
    ap=argparse.ArgumentParser();ap.add_argument('--root',required=True);ap.add_argument('--apply',action='store_true');args=ap.parse_args()
    root=Path(args.root).resolve()
    # An R53+ migration extends R33 postimages; do not replay obsolete exact
    # preimages if the full R82R1 state is independently source-gate proven.
    if verify_complete_r82r1(root):
        print("r82r1_corrective_audit: complete R82R1 normalized source verified; historical migration no-op.")
        return 0
    files={
      'transform_cpp':root/'engine/src/ship_editor/ShipyardTransformSystem.cpp',
      'builder_cpp':root/'engine/src/ship_editor/ShipyardBuilderSystem.cpp',
      'gizmo_cpp':root/'engine/src/studio/StudioAxisGizmo.cpp',
      'overlay_cpp':root/'engine/src/studio/StudioGizmoOverlay.cpp',
    }
    for k,p in files.items():
        if not p.is_file():raise RuntimeError(f"R82R1 missing source {k}: {p}")
    original={k:p.read_text(encoding='utf-8') for k,p in files.items()};changed=dict(original);touched=[]
    require(original['builder_cpp'],'BeginDuplicateSelectedPlacement()','R63 prerequisite')
    require(original['builder_cpp'],'DeleteSelectionSafe','R78 prerequisite')
    require(original['gizmo_cpp'],'ShipyardTransformSpacePolicy::Effective','R43 prerequisite')
    if 'StudioTransformStatusPolicy::SpaceName(snapshot.effectiveSpace)' not in original['overlay_cpp'] and 'StudioTransformStatusPolicy::EffectiveSpaceName(snapshot.transformTool,snapshot.effectiveSpace)' not in original['overlay_cpp']:
        raise RuntimeError('R53/R82R1 prerequisite: transform HUD effective-space token missing')
    def edit(k,old,new,label):
        nonlocal changed,touched
        changed[k],did=replace_exact(changed[k],old,new,label)
        if did and k not in touched:touched.append(k)
    def regex(k,pat,new,label):
        nonlocal changed,touched
        changed[k],did=replace_regex(changed[k],pat,new,label)
        if did and k not in touched:touched.append(k)

    edit('transform_cpp',
'''    const Vector3 authored=tx.space==ShipyardTransformSpace::Local?
        ConstructionTransformBasisSystem::ApplyAssemblyRotation(d,tx.before):d;
    tx.working.x+=authored.x*scale;tx.working.y+=authored.y*scale;tx.working.z+=authored.z*scale;''',
'''    Vector3 authored=d;
    if(tx.space==ShipyardTransformSpace::Local){
        const auto basis=ConstructionTransformBasisSystem::AssemblyLocal(tx.before,true);
        authored=basis.x*d.x+basis.y*d.y+basis.z*d.z;
    }
    tx.working.x+=authored.x*scale;tx.working.y+=authored.y*scale;tx.working.z+=authored.z*scale;''',
         'mirrored OBJECT translation authority')

    regex('builder_cpp',
          r'bool ShipyardBuilderSystem::SetTransformConstraint\(ShipyardTransformConstraint c,bool toggleLocalOnRepeat\)\{.*?\n\}\n\nvoid ShipyardBuilderSystem::ClearTransformConstraint\(\)',
'''bool ShipyardBuilderSystem::SetTransformConstraint(ShipyardTransformConstraint c,bool toggleLocalOnRepeat){
    if(c==ShipyardTransformConstraint::Free){ClearTransformConstraint();return true;}
    if(model_.transformConstraint==c&&toggleLocalOnRepeat){
        if(model_.transformTool!=ShipyardTransformTool::Move){
            model_.transformConstraint=ShipyardTransformConstraint::Free;
            model_.transformConstraintLocal=false;
            model_.status="Transform constraint cleared";
            return true;
        }
        if(!model_.transformConstraintLocal){
            model_.transformConstraintLocal=true;
            model_.status=std::string("Constraint ")+TransformConstraintName(c)+" / OBJECT";
        }else{
            model_.transformConstraint=ShipyardTransformConstraint::Free;
            model_.transformConstraintLocal=false;
            model_.status="Transform constraint cleared";
        }
        return true;
    }
    model_.transformConstraint=c;
    model_.transformConstraintLocal=false;
    if(model_.transformTool==ShipyardTransformTool::Move)
        model_.status=std::string("Constraint ")+TransformConstraintName(c)+" / PARENT (press again for OBJECT)";
    else if(model_.transformTool==ShipyardTransformTool::Rotate)
        model_.status=std::string("Constraint ")+TransformConstraintName(c)+" / EULER (press again to clear)";
    else
        model_.status=std::string("Constraint ")+TransformConstraintName(c)+" / OBJECT W-L-H (press again to clear)";
    return true;
}

void ShipyardBuilderSystem::ClearTransformConstraint()''',
          'tool-truthful repeated constraint behavior')

    edit('builder_cpp',
'''    if(model_.transformTool==ShipyardTransformTool::Move)model_.status="MOVE / "+orientation+" axes / SHIFT = 0.1x precision";
    else if(model_.transformTool==ShipyardTransformTool::Rotate)model_.status="ROTATE / "+orientation+" axes / SHIFT = 0.1x precision";
    else model_.status="SCALE / OBJECT W-L-H / SHIFT = 0.1x precision";''',
'''    if(model_.transformTool==ShipyardTransformTool::Move)model_.status="MOVE / "+orientation+" axes / SHIFT = 0.1x precision";
    else if(model_.transformTool==ShipyardTransformTool::Rotate)model_.status="ROTATE / EULER components / SHIFT = 0.1x precision";
    else model_.status="SCALE / OBJECT W-L-H / SHIFT = 0.1x precision";''',
         'truthful Rotate status')

    edit('gizmo_cpp',
'''    out.effectiveSpaceOverride=out.effectiveSpace!=out.selectedSpace;''',
'''    out.effectiveSpaceOverride=StudioTransformStatusPolicy::HasEffectiveOverride(
        model.transformTool,model.transformSpace,model.transformConstraintLocal);''',
         'truthful effective-space override')
    if '#include "studio/StudioTransformStatusPolicy.h"' not in changed['gizmo_cpp']:
        changed['gizmo_cpp'],did=replace_exact(changed['gizmo_cpp'],
            '#include "ship_editor/ShipyardTransformSpacePolicy.h"',
            '#include "ship_editor/ShipyardTransformSpacePolicy.h"\n#include "studio/StudioTransformStatusPolicy.h"',
            'gizmo transform status include')
        if did and 'gizmo_cpp' not in touched:touched.append('gizmo_cpp')

    edit('overlay_cpp',
'''        glColor4f(.93f,.96f,1.0f,1.0f);Label(StudioTransformStatusPolicy::SpaceName(snapshot.effectiveSpace),x+235,y+108);''',
'''        glColor4f(.93f,.96f,1.0f,1.0f);Label(StudioTransformStatusPolicy::EffectiveSpaceName(snapshot.transformTool,snapshot.effectiveSpace),x+235,y+108);''',
         'EULER HUD disclosure')

    require(changed['transform_cpp'],'ConstructionTransformBasisSystem::AssemblyLocal(tx.before,true)','mirrored local move')
    require(changed['builder_cpp'],'ROTATE / EULER components','Rotate disclosure')
    require(changed['builder_cpp'],'/ EULER (press again to clear)','Rotate constraint disclosure')
    require(changed['gizmo_cpp'],'StudioTransformStatusPolicy::HasEffectiveOverride','gizmo truthful override')
    require(changed['overlay_cpp'],'EffectiveSpaceName(snapshot.transformTool,snapshot.effectiveSpace)','HUD truthful override')
    if not args.apply:
        print('R82R1 DRY RUN PASS; files requiring normalization:',', '.join(touched) if touched else 'none');return 0
    if not touched:
        print('R82R1 corrective audit already present; no mutation required.');return 0
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%SZ');backup=root/'artifacts/gates/migrations/studio_r82r1'/stamp;backup.mkdir(parents=True,exist_ok=False)
    receipt={'schema':'subspace.studio-r82r1-migration.v1','timestampUtc':stamp,'files':[]}
    for k in touched:
        p=files[k];rel=p.relative_to(root);dst=backup/rel;dst.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,dst);receipt['files'].append({'path':rel.as_posix(),'beforeSha256':sha256(p.read_bytes())})
    try:
        for k in touched:
            p=files[k];tmp=p.with_name(p.name+'.r82r1.tmp');tmp.write_text(changed[k],encoding='utf-8',newline='\n');os.replace(tmp,p)
        for item in receipt['files']:item['afterSha256']=sha256((root/item['path']).read_bytes())
        (backup/'receipt.json').write_text(json.dumps(receipt,indent=2)+'\n',encoding='utf-8')
    except Exception:
        for item in receipt['files']:
            rel=Path(item['path']);src=backup/rel
            if src.exists():shutil.copy2(src,root/rel)
        raise
    print(f'R82R1 corrective audit applied transactionally to {len(touched)} file(s).')
    print(f'Backup/receipt: {backup}')
    return 0
if __name__=='__main__':
    try:raise SystemExit(main())
    except Exception as exc:
        print(f'R82R1 NORMALIZATION BLOCKED: {exc}',file=sys.stderr);raise SystemExit(2)
