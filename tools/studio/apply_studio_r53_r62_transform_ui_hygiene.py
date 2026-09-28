#!/usr/bin/env python3
"""Guarded R53-R62 Studio transform UI/state hygiene normalization.

Requires R43-R52 transform authority. Edits are exact-preimage bounded,
idempotent, transactional, and backed up before replacement.
"""
from __future__ import annotations
import argparse, hashlib, json, os, shutil, sys
from datetime import datetime, timezone
from pathlib import Path
from studio_verified_normalized_state import verify_complete_r82r1


def sha256(data: bytes)->str: return hashlib.sha256(data).hexdigest()
def replace_exact(text:str,old:str,new:str,label:str):
    if new in text:return text,False
    n=text.count(old)
    if n!=1:raise RuntimeError(f"{label}: expected one approved preimage, found {n}")
    return text.replace(old,new,1),True

def require(text:str,token:str,label:str):
    if token not in text:raise RuntimeError(f"{label}: required token missing: {token}")


def main()->int:
    ap=argparse.ArgumentParser();ap.add_argument('--root',required=True);ap.add_argument('--apply',action='store_true');args=ap.parse_args()
    root=Path(args.root).resolve()
    # An R53+ migration extends R33 postimages; do not replay obsolete exact
    # preimages if the full R82R1 state is independently source-gate proven.
    if verify_complete_r82r1(root):
        print("r53_r62_transform_ui_hygiene: complete R82R1 normalized source verified; historical migration no-op.")
        return 0
    files={
      'input_h':root/'engine/include/input/InputState.h',
      'window_cpp':root/'engine/src/platform/NativeWindow.cpp',
      'builder_cpp':root/'engine/src/ship_editor/ShipyardBuilderSystem.cpp',
      'app_cpp':root/'engine/src/studio/StudioApplication.cpp',
      'gizmo_h':root/'engine/include/studio/StudioAxisGizmo.h',
      'gizmo_cpp':root/'engine/src/studio/StudioAxisGizmo.cpp',
      'overlay_cpp':root/'engine/src/studio/StudioGizmoOverlay.cpp',
      'overlay_policy':root/'engine/include/studio/StudioOverlayPlacementPolicy.h',
      'status_policy':root/'engine/include/studio/StudioTransformStatusPolicy.h',
    }
    for k,p in files.items():
        if not p.is_file():raise RuntimeError(f"R53-R62 missing source {k}: {p}")
    original={k:p.read_text(encoding='utf-8') for k,p in files.items()}
    require(original['builder_cpp'],'ShipyardTransformSpacePolicy::Next','R43-R52 prerequisite')
    require(original['gizmo_cpp'],'StudioTransformViewBasis::Build(camera)','R43-R52 prerequisite')
    require(original['app_cpp'],'StudioTransformMoveDelta::AssemblyAuthored','R43-R52 prerequisite')
    require(original['overlay_policy'],'kHudHeight=130.0f','R59 package prerequisite')
    changed=dict(original);touched=[]
    def edit(k,old,new,label):
        nonlocal changed,touched
        changed[k],did=replace_exact(changed[k],old,new,label)
        if did and k not in touched:touched.append(k)

    # R54: append a DCC action so historical action indices remain stable.
    edit('input_h',
         '    DccConstraintClear,\n    Count',
         '    DccConstraintClear,\n    DccCycleTransformSpace,\n    Count',
         'append transform-space action')
    edit('window_cpp',
         "        case VK_OEM_6: _inputState.SetAction(InputAction::DccWorkspaceNext, down); break;\n        case VK_RETURN:",
         "        case VK_OEM_6: _inputState.SetAction(InputAction::DccWorkspaceNext, down); break;\n        case VK_OEM_COMMA: _inputState.SetAction(InputAction::DccCycleTransformSpace, down); break;\n        case VK_RETURN:",
         'comma transform-space binding')

    # R55: a user-selected space is authoritative. Old local-axis constraints
    # must not silently override the newly selected PARENT/OBJECT/VIEW mode.
    edit('builder_cpp',
         '''        case ShipyardBuilderCommand::ToggleTransformSpace:\n            model_.transformSpace=ShipyardTransformSpacePolicy::Next(model_.transformSpace);\n            model_.status=std::string("TRANSFORM SPACE: ")+ShipyardTransformSpacePolicy::Name(model_.transformSpace);return true;''',
         '''        case ShipyardBuilderCommand::ToggleTransformSpace:\n            ClearTransformConstraint();\n            model_.transformSpace=ShipyardTransformSpacePolicy::Next(model_.transformSpace);\n            model_.status=std::string("TRANSFORM SPACE: ")+ShipyardTransformSpacePolicy::Name(model_.transformSpace)+\n                " / Transform space changed; transient axis/local constraint cleared";return true;''',
         'authoritative space selector')

    # R56: axis/local locks are operation/tool state, not hidden persistent
    # orientation. Tool changes preserve transformSpace but clear the lock.
    tool_edits=[
      ('case ShipyardBuilderCommand::ToolSelect:if(model_.inspectorTab!=ShipyardInspectorTab::Sockets&&model_.workspaceMode!=ShipyardWorkspaceMode::Model){model_.workspaceMode=ShipyardWorkspaceMode::Build;model_.inspectorTab=ShipyardInspectorTab::Transform;}model_.transformTool=ShipyardTransformTool::Select;CancelTransform();CancelSocketTransform();model_.status="Select tool";return true;',
       'case ShipyardBuilderCommand::ToolSelect:if(model_.inspectorTab!=ShipyardInspectorTab::Sockets&&model_.workspaceMode!=ShipyardWorkspaceMode::Model){model_.workspaceMode=ShipyardWorkspaceMode::Build;model_.inspectorTab=ShipyardInspectorTab::Transform;}model_.transformTool=ShipyardTransformTool::Select;CancelTransform();CancelSocketTransform();ClearTransformConstraint();model_.status="Select tool";return true;', 'Select constraint hygiene'),
      ('case ShipyardBuilderCommand::ToolMove:if(model_.inspectorTab!=ShipyardInspectorTab::Sockets&&model_.workspaceMode!=ShipyardWorkspaceMode::Model){model_.workspaceMode=ShipyardWorkspaceMode::Build;model_.inspectorTab=ShipyardInspectorTab::Transform;}model_.transformTool=ShipyardTransformTool::Move;CancelTransform();CancelSocketTransform();model_.status=model_.inspectorTab==ShipyardInspectorTab::Sockets?"Socket MOVE tool":"Move tool";return true;',
       'case ShipyardBuilderCommand::ToolMove:if(model_.inspectorTab!=ShipyardInspectorTab::Sockets&&model_.workspaceMode!=ShipyardWorkspaceMode::Model){model_.workspaceMode=ShipyardWorkspaceMode::Build;model_.inspectorTab=ShipyardInspectorTab::Transform;}model_.transformTool=ShipyardTransformTool::Move;CancelTransform();CancelSocketTransform();ClearTransformConstraint();model_.status=model_.inspectorTab==ShipyardInspectorTab::Sockets?"Socket MOVE tool":"Move tool";return true;', 'Move constraint hygiene'),
      ('case ShipyardBuilderCommand::ToolRotate:if(model_.inspectorTab!=ShipyardInspectorTab::Sockets&&model_.workspaceMode!=ShipyardWorkspaceMode::Model){model_.workspaceMode=ShipyardWorkspaceMode::Build;model_.inspectorTab=ShipyardInspectorTab::Transform;}model_.transformTool=ShipyardTransformTool::Rotate;CancelTransform();CancelSocketTransform();model_.status=model_.inspectorTab==ShipyardInspectorTab::Sockets?"Socket ROTATE tool":"Rotate tool";return true;',
       'case ShipyardBuilderCommand::ToolRotate:if(model_.inspectorTab!=ShipyardInspectorTab::Sockets&&model_.workspaceMode!=ShipyardWorkspaceMode::Model){model_.workspaceMode=ShipyardWorkspaceMode::Build;model_.inspectorTab=ShipyardInspectorTab::Transform;}model_.transformTool=ShipyardTransformTool::Rotate;CancelTransform();CancelSocketTransform();ClearTransformConstraint();model_.status=model_.inspectorTab==ShipyardInspectorTab::Sockets?"Socket ROTATE tool":"Rotate tool";return true;', 'Rotate constraint hygiene'),
      ('case ShipyardBuilderCommand::ToolScale:if(model_.workspaceMode!=ShipyardWorkspaceMode::Model){model_.workspaceMode=ShipyardWorkspaceMode::Build;model_.inspectorTab=ShipyardInspectorTab::Transform;}model_.transformTool=ShipyardTransformTool::Scale;CancelTransform();model_.status="Scale tool";return true;',
       'case ShipyardBuilderCommand::ToolScale:if(model_.workspaceMode!=ShipyardWorkspaceMode::Model){model_.workspaceMode=ShipyardWorkspaceMode::Build;model_.inspectorTab=ShipyardInspectorTab::Transform;}model_.transformTool=ShipyardTransformTool::Scale;CancelTransform();ClearTransformConstraint();model_.status="Scale tool / OBJECT W-L-H";return true;', 'Scale constraint hygiene'),
    ]
    for old,new,label in tool_edits:edit('builder_cpp',old,new,label)

    # R58: comma is a Studio DCC shortcut only when typing does not own input.
    edit('app_cpp',
         '''        if(!ctrl){\n            if(input_.WasPressed(InputAction::EditorToolSelect))RouteControl(ShipyardBuilderCommand::ToolSelect);''',
         '''        if(!ctrl){\n            if(input_.WasPressed(InputAction::DccCycleTransformSpace))RouteControl(ShipyardBuilderCommand::ToggleTransformSpace);\n            if(input_.WasPressed(InputAction::EditorToolSelect))RouteControl(ShipyardBuilderCommand::ToolSelect);''',
         'search-safe transform-space shortcut')

    # R53/R57: snapshot carries both the selected persistent space and the
    # effective tool space, so Scale/local constraints can never be hidden.
    edit('gizmo_h',
         '''    bool readoutVisible=false;\n    float readoutLeft=0.0f,readoutTop=0.0f;\n    StudioTransformReadout readout{};''',
         '''    bool readoutVisible=false;\n    float readoutLeft=0.0f,readoutTop=0.0f;\n    ShipyardTransformSpace selectedSpace=ShipyardTransformSpace::Ship;\n    ShipyardTransformSpace effectiveSpace=ShipyardTransformSpace::Ship;\n    ShipyardTransformTool transformTool=ShipyardTransformTool::Select;\n    bool effectiveSpaceOverride=false;\n    StudioTransformReadout readout{};''',
         'gizmo transform status fields')
    edit('gizmo_cpp',
         '''    StudioGizmoSnapshot out;\n    const bool modelMode=model.workspaceMode==ShipyardWorkspaceMode::Model&&!model.modeling.recipe.primitives.empty();''',
         '''    StudioGizmoSnapshot out;\n    out.selectedSpace=model.transformSpace;\n    out.transformTool=model.transformTool;\n    out.effectiveSpace=ShipyardTransformSpacePolicy::Effective(\n        model.transformTool,model.transformSpace,model.transformConstraintLocal);\n    out.effectiveSpaceOverride=out.effectiveSpace!=out.selectedSpace;\n    const bool modelMode=model.workspaceMode==ShipyardWorkspaceMode::Model&&!model.modeling.recipe.primitives.empty();''',
         'gizmo status snapshot')

    # R53/R57/R59: extend the existing compact OpenGL HUD. No new renderer,
    # font ownership or transform state is introduced.
    edit('overlay_cpp',
         '#include "studio/StudioMeasurementFormat.h"',
         '#include "studio/StudioMeasurementFormat.h"\n#include "studio/StudioTransformStatusPolicy.h"',
         'transform HUD status include')
    edit('overlay_cpp',
         '''        case 'M':Line(x,y+13,x,y);Line(x,y,x+4,y+6);Line(x+4,y+6,x+8,y);Line(x+8,y,x+8,y+13);break;\n        default:break;''',
         '''        case 'M':Line(x,y+13,x,y);Line(x,y,x+4,y+6);Line(x+4,y+6,x+8,y);Line(x+8,y,x+8,y+13);break;\n        case 'A':Line(x,y+13,x+4,y);Line(x+4,y,x+8,y+13);Line(x+2,y+7,x+6,y+7);break;\n        case 'B':Line(x,y,x,y+13);Line(x,y,x+6,y);Line(x+6,y,x+8,y+3);Line(x+8,y+3,x+6,y+6);Line(x+6,y+6,x,y+6);Line(x+6,y+6,x+8,y+10);Line(x+8,y+10,x+6,y+13);Line(x+6,y+13,x,y+13);break;\n        case 'E':Line(x+8,y,x,y);Line(x,y,x,y+13);Line(x,y+6,x+7,y+6);Line(x,y+13,x+8,y+13);break;\n        case 'J':Line(x,y,x+8,y);Line(x+5,y,x+5,y+11);Line(x+5,y+11,x+3,y+13);Line(x+3,y+13,x,y+11);break;\n        case 'N':Line(x,y+13,x,y);Line(x,y,x+8,y+13);Line(x+8,y+13,x+8,y);break;\n        case 'U':Line(x,y,x,y+10);Line(x,y+10,x+3,y+13);Line(x+3,y+13,x+5,y+13);Line(x+5,y+13,x+8,y+10);Line(x+8,y+10,x+8,y);break;\n        case 'V':Line(x,y,x+4,y+13);Line(x+4,y+13,x+8,y);break;\n        case 'W':Line(x,y,x+2,y+13);Line(x+2,y+13,x+4,y+7);Line(x+4,y+7,x+6,y+13);Line(x+6,y+13,x+8,y);break;\n        default:break;''',
         'HUD orientation glyphs')
    edit('overlay_cpp',
         '    if(availableWidth<455.0f||availableHeight<145.0f)return;',
         '    if(availableWidth<455.0f||availableHeight<169.0f)return;',
         'HUD minimum height')
    edit('overlay_cpp',
         '''    glBegin(GL_QUADS);glVertex2f(x,y);glVertex2f(x+435,y);\n       glVertex2f(x+435,y+106);glVertex2f(x,y+106);glEnd();''',
         '''    glBegin(GL_QUADS);glVertex2f(x,y);glVertex2f(x+435,y);\n       glVertex2f(x+435,y+130);glVertex2f(x,y+130);glEnd();''',
         'HUD background height')
    edit('overlay_cpp',
         '''    // DIM is NOMINAL catalog-space W/L/H in meters. It is not mesh-exact,\n    // a rotated world AABB, a Boolean result, or a promise of interior volume.\n}''',
         '''    // DIM is NOMINAL catalog-space W/L/H in meters. It is not mesh-exact,\n    // a rotated world AABB, a Boolean result, or a promise of interior volume.\n    glColor4f(.76f,.86f,.94f,.98f);glLineWidth(1.6f);\n    Label("SPACE",x+8,y+108);\n    glColor4f(.93f,.96f,1.0f,1.0f);\n    Label(StudioTransformStatusPolicy::SpaceName(snapshot.selectedSpace),x+70,y+108);\n    if(snapshot.effectiveSpaceOverride){\n        glColor4f(.72f,.78f,.86f,.94f);Label("USE",x+194,y+108);\n        glColor4f(.93f,.96f,1.0f,1.0f);Label(StudioTransformStatusPolicy::SpaceName(snapshot.effectiveSpace),x+235,y+108);\n    }\n}''',
         'HUD orientation line')

    # Postconditions.
    require(changed['input_h'],'DccCycleTransformSpace','DCC input')
    require(changed['window_cpp'],'VK_OEM_COMMA','comma binding')
    require(changed['app_cpp'],'InputAction::DccCycleTransformSpace','Studio shortcut')
    require(changed['builder_cpp'],'transient axis/local constraint cleared','selector hygiene')
    require(changed['builder_cpp'],'Scale tool / OBJECT W-L-H','Scale disclosure')
    require(changed['gizmo_h'],'effectiveSpaceOverride','gizmo status')
    require(changed['gizmo_cpp'],'out.selectedSpace=model.transformSpace','gizmo selected space')
    require(changed['overlay_cpp'],'Label("SPACE"','HUD selected space')
    require(changed['overlay_cpp'],'Label("USE"','HUD effective override')

    if not args.apply:
        print('R53-R62 DRY RUN PASS; files requiring normalization:',', '.join(touched) if touched else 'none');return 0
    if not touched:
        print('R53-R62 transform UI/state hygiene already present; no mutation required.');return 0
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%SZ')
    backup=root/'artifacts/gates/migrations/studio_r53_r62'/stamp;backup.mkdir(parents=True,exist_ok=False)
    receipt={'schema':'subspace.studio-r53-r62-migration.v1','timestampUtc':stamp,'files':[]}
    for k in touched:
        p=files[k];rel=p.relative_to(root);dst=backup/rel;dst.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,dst)
        receipt['files'].append({'path':rel.as_posix(),'beforeSha256':sha256(p.read_bytes())})
    try:
        for k in touched:
            p=files[k];tmp=p.with_name(p.name+'.r53r62.tmp');tmp.write_text(changed[k],encoding='utf-8',newline='\n');os.replace(tmp,p)
        for item in receipt['files']:item['afterSha256']=sha256((root/item['path']).read_bytes())
        (backup/'receipt.json').write_text(json.dumps(receipt,indent=2)+'\n',encoding='utf-8')
    except Exception:
        for item in receipt['files']:
            rel=Path(item['path']);src=backup/rel
            if src.exists():shutil.copy2(src,root/rel)
        raise
    print(f'R53-R62 transform UI/state hygiene applied transactionally to {len(touched)} file(s).')
    print(f'Backup/receipt: {backup}')
    return 0

if __name__=='__main__':
    try:raise SystemExit(main())
    except Exception as exc:
        print(f'R53-R62 NORMALIZATION BLOCKED: {exc}',file=sys.stderr);raise SystemExit(2)
