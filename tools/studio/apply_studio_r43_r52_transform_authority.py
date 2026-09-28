#!/usr/bin/env python3
"""Guarded R43-R52 transform-space authority normalization.

Requires the R32 Construct cutover and R33-R42 bulk polish. Changes are
idempotent, exact/approved-preimage bounded, transactional and backed up.
"""
from __future__ import annotations
import argparse, hashlib, json, os, re, shutil, sys
from datetime import datetime, timezone
from pathlib import Path
from studio_verified_normalized_state import verify_complete_r82r1


def sha256(data: bytes) -> str: return hashlib.sha256(data).hexdigest()

def replace_exact(text: str, old: str, new: str, label: str):
    if new in text: return text, False
    n=text.count(old)
    if n!=1: raise RuntimeError(f"{label}: expected one approved preimage, found {n}")
    return text.replace(old,new,1), True

def replace_one_of(text: str, olds: list[str], new: str, label: str):
    if new in text: return text, False
    hits=[old for old in olds if old in text]
    if len(hits)!=1: raise RuntimeError(f"{label}: expected exactly one approved preimage variant, found {len(hits)}")
    return text.replace(hits[0],new,1), True

def replace_regex(text: str, pattern: str, repl: str, label: str):
    if repl in text: return text, False
    out,n=re.subn(pattern,repl,text,count=1,flags=re.S)
    if n!=1: raise RuntimeError(f"{label}: expected one approved regex preimage, found {n}")
    return out, True

def require(text: str, token: str, label: str):
    if token not in text: raise RuntimeError(f"{label}: required token missing: {token}")


def main()->int:
    ap=argparse.ArgumentParser();ap.add_argument('--root',required=True);ap.add_argument('--apply',action='store_true');args=ap.parse_args()
    root=Path(args.root).resolve()
    # An R53+ migration extends R33 postimages; do not replay obsolete exact
    # preimages if the full R82R1 state is independently source-gate proven.
    if verify_complete_r82r1(root):
        print("r43_r52_transform_authority: complete R82R1 normalized source verified; historical migration no-op.")
        return 0
    files={
      'builder_h':root/'engine/include/ship_editor/ShipyardBuilderSystem.h',
      'transform_h':root/'engine/include/ship_editor/ShipyardTransformSystem.h',
      'transform_cpp':root/'engine/src/ship_editor/ShipyardTransformSystem.cpp',
      'builder_cpp':root/'engine/src/ship_editor/ShipyardBuilderSystem.cpp',
      'gizmo_cpp':root/'engine/src/studio/StudioAxisGizmo.cpp',
      'app_cpp':root/'engine/src/studio/StudioApplication.cpp',
      'workspace_cpp':root/'engine/src/ship_editor/ShipyardWorkspaceSystem.cpp',
      'renderer_cpp':root/'engine/src/application/NativeBattlefieldRenderer.cpp',
      'pass506':root/'engine/tests/pass506r7_universal_build_controls_tests.cpp',
    }
    for k,p in files.items():
        if not p.is_file(): raise RuntimeError(f"R43-R52 missing source {k}: {p}")
    original={k:p.read_text(encoding='utf-8') for k,p in files.items()}
    require(original['workspace_cpp'],'case ShipyardWorkspaceMode::Build:return"CONSTRUCT";','R32 prerequisite')
    require(original['workspace_cpp'],'case ShipyardWorkspaceMode::Model:return"GEOMETRY";','R32 prerequisite')
    require(original['app_cpp'],'StudioOverlayPlacementPolicy::Choose','R33 prerequisite')
    changed=dict(original);touched=[]
    def edit(k,old,new,label):
        nonlocal changed,touched
        changed[k],did=replace_exact(changed[k],old,new,label)
        if did and k not in touched:touched.append(k)
    def one(k,olds,new,label):
        nonlocal changed,touched
        changed[k],did=replace_one_of(changed[k],olds,new,label)
        if did and k not in touched:touched.append(k)
    def regex(k,pat,new,label):
        nonlocal changed,touched
        changed[k],did=replace_regex(changed[k],pat,new,label)
        if did and k not in touched:touched.append(k)

    # R43: PARENT/assembly is the persistent safe default, never VIEW/camera.
    one('builder_h',[
         '    ShipyardTransformSpace transformSpace = ShipyardTransformSpace::View;',
         '    ShipyardTransformSpace transformSpace = ShipyardTransformSpace::Ship;'],
         '    ShipyardTransformSpace transformSpace = ShipyardTransformSpace::Ship; // PARENT is the safe DCC default',
         'builder default transform space')
    one('transform_h',[
         '    ShipyardTransformSpace space = ShipyardTransformSpace::View;',
         '    ShipyardTransformSpace space = ShipyardTransformSpace::Ship;'],
         '    ShipyardTransformSpace space = ShipyardTransformSpace::Ship; // PARENT default',
         'transaction default transform space')
    one('transform_h',[
         '                      ShipyardTransformSpace space=ShipyardTransformSpace::View);',
         '                      ShipyardTransformSpace space=ShipyardTransformSpace::Ship);'],
         '                      ShipyardTransformSpace space=ShipyardTransformSpace::Ship);',
         'Begin default transform space')
    edit('builder_h',
         '    bool TranslateSelected(const Vector3& delta,bool fine=false);',
         '    bool TranslateSelected(const Vector3& delta,bool fine=false);\n    // Delta is already resolved into authored parent coordinates by the Studio gizmo.\n    bool TranslateSelectedResolvedParent(const Vector3& delta);',
         'resolved parent translation API')

    # R44: shared names/cycle policy is used instead of three ad-hoc ternaries.
    edit('builder_cpp','#include "ship_editor/ShipyardBuilderSystem.h"',
         '#include "ship_editor/ShipyardBuilderSystem.h"\n#include "ship_editor/ShipyardTransformSpacePolicy.h"',
         'builder transform-space policy include')
    toggle_old='''        case ShipyardBuilderCommand::ToggleTransformSpace:\n            if(model_.transformSpace==ShipyardTransformSpace::View)model_.transformSpace=ShipyardTransformSpace::Ship;\n            else if(model_.transformSpace==ShipyardTransformSpace::Ship)model_.transformSpace=ShipyardTransformSpace::Local;\n            else model_.transformSpace=ShipyardTransformSpace::View;\n            model_.status=model_.transformSpace==ShipyardTransformSpace::View?"TRANSFORM SPACE: CAMERA":(model_.transformSpace==ShipyardTransformSpace::Ship?"TRANSFORM SPACE: SHIP":"TRANSFORM SPACE: LOCAL");return true;'''
    toggle_r32='''        case ShipyardBuilderCommand::ToggleTransformSpace:\n            if(model_.transformSpace==ShipyardTransformSpace::View)model_.transformSpace=ShipyardTransformSpace::Ship;\n            else if(model_.transformSpace==ShipyardTransformSpace::Ship)model_.transformSpace=ShipyardTransformSpace::Local;\n            else model_.transformSpace=ShipyardTransformSpace::View;\n            model_.status=model_.transformSpace==ShipyardTransformSpace::View?"TRANSFORM SPACE: VIEW":(model_.transformSpace==ShipyardTransformSpace::Ship?"TRANSFORM SPACE: PARENT":"TRANSFORM SPACE: OBJECT");return true;'''
    toggle_new='''        case ShipyardBuilderCommand::ToggleTransformSpace:\n            model_.transformSpace=ShipyardTransformSpacePolicy::Next(model_.transformSpace);\n            model_.status=std::string("TRANSFORM SPACE: ")+ShipyardTransformSpacePolicy::Name(model_.transformSpace);return true;'''
    one('builder_cpp',[toggle_old,toggle_r32],toggle_new,'transform-space cycle')
    regex('builder_cpp',r'        const std::string spaceLabel=model\.transformSpace==ShipyardTransformSpace::View\?"(?:CAMERA|VIEW)":\(model\.transformSpace==ShipyardTransformSpace::Ship\?"(?:SHIP|PARENT)":"(?:LOCAL|OBJECT)"\);',
          '        const std::string spaceLabel=ShipyardTransformSpacePolicy::Name(model.transformSpace);',
          'Build transform-space label')
    regex('builder_cpp',r'\{ShipyardBuilderCommand::ToggleTransformSpace,model\.transformSpace==ShipyardTransformSpace::View\?"(?:CAMERA|VIEW)":\(model\.transformSpace==ShipyardTransformSpace::Ship\?"(?:SHIP|PARENT)":"(?:LOCAL|OBJECT)"\),false,hasSocket\}',
          '{ShipyardBuilderCommand::ToggleTransformSpace,ShipyardTransformSpacePolicy::Name(model.transformSpace),false,hasSocket}',
          'Socket transform-space label')
    one('builder_cpp',[
      '    model_.status=std::string("Constraint ")+TransformConstraintName(c)+" / GLOBAL-SHIP (press again for LOCAL)";',
      '    model_.status=std::string("Constraint ")+TransformConstraintName(c)+" / PARENT (press again for OBJECT)";'],
      '    model_.status=std::string("Constraint ")+TransformConstraintName(c)+" / PARENT (press again for OBJECT)";',
      'constraint parent label')
    one('renderer_cpp',[
      'case ShipyardBuilderCommand::ToggleTransformSpace:help="Transform Space: cycle CAMERA, SHIP, and LOCAL movement axes.";break;',
      'case ShipyardBuilderCommand::ToggleTransformSpace:help="Transform Space: cycle VIEW, PARENT, and OBJECT movement axes.";break;'],
      'case ShipyardBuilderCommand::ToggleTransformSpace:help="Transform Space: cycle PARENT, OBJECT, and VIEW orientations. Scale always uses OBJECT W/L/H.";break;',
      'native transform-space help')
    pass506_old='''    Check(builder.Model().transformSpace==ShipyardTransformSpace::View,"player-friendly camera transform space is the default");
    builder.Activate(ShipyardBuilderCommand::ToggleTransformSpace);
    Check(builder.Model().transformSpace==ShipyardTransformSpace::Ship,"transform space cycles camera to ship");
    builder.Activate(ShipyardBuilderCommand::ToggleTransformSpace);
    Check(builder.Model().transformSpace==ShipyardTransformSpace::Local,"transform space cycles ship to local");
    builder.Activate(ShipyardBuilderCommand::ToggleTransformSpace);
    Check(builder.Model().transformSpace==ShipyardTransformSpace::View,"transform space cycles local back to camera");'''
    pass506_new='''    Check(builder.Model().transformSpace==ShipyardTransformSpace::Ship,"PARENT/assembly transform space is the safe authoring default");
    builder.Activate(ShipyardBuilderCommand::ToggleTransformSpace);
    Check(builder.Model().transformSpace==ShipyardTransformSpace::Local,"transform space cycles PARENT to explicit OBJECT");
    builder.Activate(ShipyardBuilderCommand::ToggleTransformSpace);
    Check(builder.Model().transformSpace==ShipyardTransformSpace::View,"transform space cycles OBJECT to explicit VIEW");
    builder.Activate(ShipyardBuilderCommand::ToggleTransformSpace);
    Check(builder.Model().transformSpace==ShipyardTransformSpace::Ship,"transform space cycles VIEW back to PARENT");'''
    edit('pass506',pass506_old,pass506_new,'Pass506R7 transform-space expectations')
    one('builder_cpp',[
      '            model_.status=std::string("Constraint ")+TransformConstraintName(c)+" / LOCAL";',
      '            model_.status=std::string("Constraint ")+TransformConstraintName(c)+" / OBJECT";'],
      '            model_.status=std::string("Constraint ")+TransformConstraintName(c)+" / OBJECT";',
      'constraint object label')

    # R45-R47: PARENT Move/Rotate remain fixed when an object rotates; deliberate
    # OBJECT Move now actually traverses the rotated local axis in authored data.
    edit('transform_cpp','#include "ship_editor/ShipyardTransformSystem.h"',
         '#include "ship_editor/ShipyardTransformSystem.h"\n#include "editor/ConstructionTransformBasisSystem.h"',
         'transform local-basis include')
    old_translate='void ShipyardTransformSystem::Translate(ShipyardTransformTransaction& tx,const Vector3& d,bool fine){if(!tx.active)return;const float scale=fine?.10f:1.0f;tx.working.x+=d.x*scale;tx.working.y+=d.y*scale;tx.working.z+=d.z*scale;if(tx.snap&&!fine){tx.working.x=Snap(tx.working.x,tx.translationSnap);tx.working.y=Snap(tx.working.y,tx.translationSnap);tx.working.z=Snap(tx.working.z,tx.translationSnap);}}'
    new_translate='''void ShipyardTransformSystem::Translate(ShipyardTransformTransaction& tx,const Vector3& d,bool fine){\n    if(!tx.active)return;\n    const float scale=fine?.10f:1.0f;\n    // PARENT is stored authoring XYZ. OBJECT deliberately rotates the gesture\n    // by the selected module orientation. VIEW is supplied camera-relative by\n    // the caller and remains a presentation/input concern.\n    const Vector3 authored=tx.space==ShipyardTransformSpace::Local?\n        ConstructionTransformBasisSystem::ApplyAssemblyRotation(d,tx.before):d;\n    tx.working.x+=authored.x*scale;tx.working.y+=authored.y*scale;tx.working.z+=authored.z*scale;\n    if(tx.snap&&!fine){tx.working.x=Snap(tx.working.x,tx.translationSnap);tx.working.y=Snap(tx.working.y,tx.translationSnap);tx.working.z=Snap(tx.working.z,tx.translationSnap);}\n}'''
    edit('transform_cpp',old_translate,new_translate,'OBJECT/local translation semantics')

    resolved_impl='''bool ShipyardBuilderSystem::TranslateSelectedResolvedParent(const Vector3& delta){
    // Used only after Studio has resolved PARENT/OBJECT/VIEW into the document's
    // authored parent coordinates. Do not re-apply an axis constraint or local basis.
    if(model_.workspaceMode==ShipyardWorkspaceMode::Model){
        if(model_.modeling.recipe.primitives.empty())return false;
        if(!modelTransformActive_&&!BeginSelectedTransform())return false;
        const auto index=std::min(model_.modeling.selectedPrimitiveIndex,model_.modeling.recipe.primitives.size()-1);
        const bool ok=ShipyardModelingSystem::TranslatePrimitive(model_.modeling.recipe,index,delta);
        if(ok){model_.dirty=true;model_.status="Modeled shape moved / resolved transform orientation";}
        return ok;
    }
    if(!model_.transform.active&&!BeginSelectedTransform())return false;
    if(model_.transform.tool!=ShipyardTransformTool::Move)return false;
    const auto savedSpace=model_.transform.space;const bool savedSnap=model_.transform.snap;
    model_.transform.space=ShipyardTransformSpace::Ship;model_.transform.snap=false;
    ShipyardTransformSystem::Translate(model_.transform,delta,false);
    model_.transform.space=savedSpace;model_.transform.snap=savedSnap;
    if(model_.transform.moduleIndex<model_.recipe.modules.size())
        model_.recipe.modules[model_.transform.moduleIndex]=model_.transform.working;
    model_.dirty=true;InvalidateRecipeMetadata();return true;
}

'''
    if 'bool ShipyardBuilderSystem::TranslateSelectedResolvedParent' not in changed['builder_cpp']:
        changed['builder_cpp'],did=replace_exact(changed['builder_cpp'],
            '\nbool ShipyardBuilderSystem::RotateSelected(const Vector3& deltaDegrees,bool fine){',
            '\n'+resolved_impl+'bool ShipyardBuilderSystem::RotateSelected(const Vector3& deltaDegrees,bool fine){',
            'resolved parent translation implementation')
        if did and 'builder_cpp' not in touched:touched.append('builder_cpp')

    # R48-R50: gizmo orientation derives from exactly one effective policy.
    edit('gizmo_cpp','#include "studio/StudioGizmoProjectionPolicy.h"',
         '#include "studio/StudioGizmoProjectionPolicy.h"\n#include "studio/StudioTransformViewBasis.h"\n#include "ship_editor/ShipyardTransformSpacePolicy.h"',
         'Studio gizmo transform-space includes')
    model_old='''        ConstructionTransformBasis basis{};\n        if(model.transformTool==ShipyardTransformTool::Scale||model.transformConstraintLocal)\n            basis=ConstructionTransformBasisSystem::ModelLocal(p.rotationDegrees);'''
    model_new='''        ConstructionTransformBasis basis{};\n        const auto effectiveSpace=ShipyardTransformSpacePolicy::Effective(\n            model.transformTool,model.transformSpace,model.transformConstraintLocal);\n        if(effectiveSpace==ShipyardTransformSpace::Local)\n            basis=ConstructionTransformBasisSystem::ModelLocal(p.rotationDegrees);\n        else if(effectiveSpace==ShipyardTransformSpace::View)\n            basis=StudioTransformViewBasis::Build(camera);'''
    edit('gizmo_cpp',model_old,model_new,'model gizmo effective space')
    assembly_old='''    ConstructionTransformBasis basis=ConstructionTransformBasisSystem::ShipWorld(yaw,rootScale);\n    if(model.transformTool==ShipyardTransformTool::Scale||\n       model.transformSpace==ShipyardTransformSpace::Local||model.transformConstraintLocal)\n        basis=ConstructionTransformBasisSystem::AssemblyWorld(part,yaw,rootScale,true);'''
    assembly_new='''    const auto effectiveSpace=ShipyardTransformSpacePolicy::Effective(\n        model.transformTool,model.transformSpace,model.transformConstraintLocal);\n    ConstructionTransformBasis basis=ConstructionTransformBasisSystem::ShipWorld(yaw,rootScale);\n    if(effectiveSpace==ShipyardTransformSpace::Local)\n        basis=ConstructionTransformBasisSystem::AssemblyWorld(part,yaw,rootScale,true);\n    else if(effectiveSpace==ShipyardTransformSpace::View)\n        basis=StudioTransformViewBasis::Build(camera);'''
    edit('gizmo_cpp',assembly_old,assembly_new,'assembly gizmo effective space')

    # R49: status text must expose the effective orientation instead of old
    # camera-follow claims that no longer match the professional Studio path.
    begin_old='''    if(model_.transformTool==ShipyardTransformTool::Move)model_.status="MOVE / arrows follow camera; SHIFT = 0.1x precision";\n    else if(model_.transformTool==ShipyardTransformTool::Rotate)model_.status="ROTATE / drag yaw+pitch; CTRL-drag roll; SHIFT = 0.1x";\n    else model_.status="SCALE / drag for uniform scale; SHIFT = 0.1x precision";'''
    begin_new='''    const auto effectiveSpace=ShipyardTransformSpacePolicy::Effective(\n        model_.transformTool,model_.transformSpace,model_.transformConstraintLocal);\n    const std::string orientation=ShipyardTransformSpacePolicy::Name(effectiveSpace);\n    if(model_.transformTool==ShipyardTransformTool::Move)model_.status="MOVE / "+orientation+" axes / SHIFT = 0.1x precision";\n    else if(model_.transformTool==ShipyardTransformTool::Rotate)model_.status="ROTATE / "+orientation+" axes / SHIFT = 0.1x precision";\n    else model_.status="SCALE / OBJECT W-L-H / SHIFT = 0.1x precision";'''
    edit('builder_cpp',begin_old,begin_new,'effective transform status')

    # R48B: the drawn VIEW/OBJECT axis and the actual Move delta must be the same
    # basis. Studio resolves the gesture once, then hands parent/authored XYZ to
    # the builder without a second hidden transform-space interpretation.
    edit('app_cpp','#include "studio/StudioGizmoDragPolicy.h"',
         '#include "studio/StudioGizmoDragPolicy.h"\n#include "studio/StudioTransformMoveDelta.h"',
         'Studio resolved move-delta include')
    model_move_old='''                            if(tool==ShipyardTransformTool::Move){
                                const float amount=StudioGizmoDragPolicy::MoveUnits(gizmoStartHandle_,
                                    {window_.GetPointerX()-gizmoPressX_,window_.GetPointerY()-gizmoPressY_},fine);
                                builder_.TranslateSelected({axis==StudioAxis::X?amount:0,axis==StudioAxis::Y?amount:0,axis==StudioAxis::Z?amount:0},false);'''
    model_move_new='''                            if(tool==ShipyardTransformTool::Move){
                                const float amount=StudioGizmoDragPolicy::MoveUnits(gizmoStartHandle_,
                                    {window_.GetPointerX()-gizmoPressX_,window_.GetPointerY()-gizmoPressY_},fine);
                                const auto resolved=StudioTransformMoveDelta::ModelAuthored(
                                    builder_.Model(),camera_,axis,amount);
                                builder_.TranslateSelectedResolvedParent(resolved);'''
    edit('app_cpp',model_move_old,model_move_new,'model resolved Move basis')
    assembly_move_pattern=r'''                    \}else if\(builder_\.Model\(\)\.transformTool==ShipyardTransformTool::Move\)\{
                        const float before=StudioGizmoMath::Component\(axis,tx\.before\.x,tx\.before\.y,tx\.before\.z\);
                        const float working=StudioGizmoMath::Component\(axis,tx\.working\.x,tx\.working\.y,tx\.working\.z\);
                        const float desired=before\+StudioGizmoDragPolicy::MoveUnits\(gizmoStartHandle_,
                            \{window_\.GetPointerX\(\)-gizmoPressX_,window_\.GetPointerY\(\)-gizmoPressY_\},fine\);
                        const float delta=desired-working;
                        const float sourceDelta=fine\?delta\*10\.0f:delta;
                        const Vector3 translation\{axis==StudioAxis::X\?sourceDelta:0,
                            axis==StudioAxis::Y\?sourceDelta:0,axis==StudioAxis::Z\?sourceDelta:0\};
                        if\(std::fabs\(delta\)>1e-6f\)builder_\.TranslateSelected\(translation,fine\);'''
    assembly_move_new='''                    }else if(builder_.Model().transformTool==ShipyardTransformTool::Move){
                        float amount=StudioGizmoDragPolicy::MoveUnits(gizmoStartHandle_,
                            {window_.GetPointerX()-gizmoPressX_,window_.GetPointerY()-gizmoPressY_},fine);
                        if(tx.snap&&!fine&&tx.translationSnap>0.0f)
                            amount=std::round(amount/tx.translationSnap)*tx.translationSnap;
                        if(builder_.ResetSelectedTransformPreview()){
                            const auto resolved=StudioTransformMoveDelta::AssemblyAuthored(
                                builder_.Model(),camera_,axis,amount);
                            if(resolved.length()>1e-6f)builder_.TranslateSelectedResolvedParent(resolved);
                        }'''
    regex('app_cpp',assembly_move_pattern,assembly_move_new,'assembly resolved Move basis')

    # Assertions guard the exact UX requested by the R43-R52 batch.
    require(changed['builder_h'],'transformSpace = ShipyardTransformSpace::Ship','PARENT default')
    require(changed['transform_h'],'space = ShipyardTransformSpace::Ship','transaction PARENT default')
    require(changed['builder_cpp'],'ShipyardTransformSpacePolicy::Next','explicit cycle')
    require(changed['builder_cpp'],'SCALE / OBJECT W-L-H','scale local status')
    require(changed['transform_cpp'],'tx.space==ShipyardTransformSpace::Local','OBJECT move semantics')
    require(changed['gizmo_cpp'],'ShipyardTransformSpacePolicy::Effective','effective gizmo policy')
    require(changed['gizmo_cpp'],'StudioTransformViewBasis::Build(camera)','VIEW basis')
    require(changed['renderer_cpp'],'Scale always uses OBJECT W/L/H','native transform help')
    require(changed['pass506'],'PARENT/assembly transform space is the safe authoring default','forward historical test')
    require(changed['builder_h'],'TranslateSelectedResolvedParent','resolved translation API')
    require(changed['builder_cpp'],'resolved transform orientation','resolved translation implementation')
    require(changed['app_cpp'],'StudioTransformMoveDelta::AssemblyAuthored','assembly movement parity')
    require(changed['app_cpp'],'StudioTransformMoveDelta::ModelAuthored','model movement parity')

    if not args.apply:
        print('R43-R52 DRY RUN PASS; files requiring normalization:',', '.join(touched) if touched else 'none');return 0
    if not touched:
        print('R43-R52 transform authority already present; no mutation required.');return 0
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%SZ')
    backup=root/'artifacts/gates/migrations/studio_r43_r52'/stamp;backup.mkdir(parents=True,exist_ok=False)
    receipt={'schema':'subspace.studio-r43-r52-migration.v1','timestampUtc':stamp,'files':[]}
    for k in touched:
        p=files[k];rel=p.relative_to(root);dst=backup/rel;dst.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,dst)
        receipt['files'].append({'path':rel.as_posix(),'beforeSha256':sha256(p.read_bytes())})
    try:
        for k in touched:
            p=files[k];tmp=p.with_name(p.name+'.r43r52.tmp');tmp.write_text(changed[k],encoding='utf-8',newline='\n');os.replace(tmp,p)
        for item in receipt['files']:item['afterSha256']=sha256((root/item['path']).read_bytes())
        (backup/'receipt.json').write_text(json.dumps(receipt,indent=2)+'\n',encoding='utf-8')
    except Exception:
        for item in receipt['files']:
            rel=Path(item['path']);src=backup/rel
            if src.exists():shutil.copy2(src,root/rel)
        raise
    print(f'R43-R52 transform authority applied transactionally to {len(touched)} file(s).')
    print(f'Backup/receipt: {backup}')
    return 0

if __name__=='__main__':
    try: raise SystemExit(main())
    except Exception as exc:
        print(f'R43-R52 NORMALIZATION BLOCKED: {exc}',file=sys.stderr);raise SystemExit(2)
