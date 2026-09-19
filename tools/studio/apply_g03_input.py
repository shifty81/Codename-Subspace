#!/usr/bin/env python3
"""Source-guarded Subspace Studio Assembly interaction hotfix.

PCC installs this tool, but does not run it. --check is read-only; --apply
requires the exact published source Git blobs, makes backups, verifies writes,
and writes a receipt. No builds, git operations, or remote changes.
"""
from __future__ import annotations
import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import sys
import tempfile

BASELINE='4f76f5124c2def175479f11e91422d1da3e015f0'
PREIMAGES={
    'engine/src/studio/StudioApplication.cpp':'13719003195575475305c9edbc408e57c97d0e9f',
    'engine/src/platform/NativeWindow.cpp':'a15a70a19932a80e542b20bcc0d0a5c912ccbff7',
}

def git_blob(data:bytes)->str:
    return hashlib.sha1(b'blob '+str(len(data)).encode()+b'\0'+data).hexdigest()

def once(src:str,old:str,new:str,tag:str)->str:
    count=src.count(old)
    if count!=1:
        raise ValueError(f'{tag}: expected one exact source anchor; found {count}')
    return src.replace(old,new,1)

def edit_studio(src:str)->str:
    src=once(src,'#include "studio/StudioGizmoMath.h"\n',
             '#include "studio/StudioGizmoMath.h"\n#include "studio/StudioToolInteractionPolicy.h"\n','selection policy include')
    src=once(src,
        '    if(input_.WasPressed(InputAction::EditorToolSelect))RouteControl(ShipyardBuilderCommand::ToolSelect);\n'
        '    if(input_.WasPressed(InputAction::EditorToolMove))RouteControl(ShipyardBuilderCommand::ToolMove);\n'
        '    if(input_.WasPressed(InputAction::EditorToolRotate))RouteControl(ShipyardBuilderCommand::ToolRotate);\n'
        '    if(scalePressed&&!window_.IsControlDown())RouteControl(ShipyardBuilderCommand::ToolScale);\n'
        '    if(input_.WasPressed(InputAction::DccCommandSearch))RouteControl(ShipyardBuilderCommand::DccToggleCommandPalette);\n'
        '    if(input_.WasPressed(InputAction::DccWorkspaceNext))RouteControl(ShipyardBuilderCommand::DccWorkspaceNext);\n'
        '    if(input_.WasPressed(InputAction::DccWorkspacePrevious))RouteControl(ShipyardBuilderCommand::DccWorkspacePrevious);\n',
        '''    // NativeWindow publishes the DCC actions; standalone Studio must route
    // them itself (the game app's shortcut dispatcher is never instantiated).
    // Typing in Asset Search must never move or delete a ship component.
    const bool typing=builder_.Model().assetSearchFocused;
    const bool ctrl=window_.IsControlDown();
    if(!typing){
        if(input_.WasPressed(InputAction::DccMaximizeArea))
            RouteControl(ShipyardBuilderCommand::DccToggleMaximizeViewport);
        if(input_.WasPressed(InputAction::DccCommandSearch))
            RouteControl(ShipyardBuilderCommand::DccToggleCommandPalette);
        if(input_.WasPressed(InputAction::DccWorkspaceNext))
            RouteControl(ShipyardBuilderCommand::DccWorkspaceNext);
        if(input_.WasPressed(InputAction::DccWorkspacePrevious))
            RouteControl(ShipyardBuilderCommand::DccWorkspacePrevious);
        if(!ctrl){
            if(input_.WasPressed(InputAction::EditorToolSelect))RouteControl(ShipyardBuilderCommand::ToolSelect);
            if(input_.WasPressed(InputAction::EditorToolMove))RouteControl(ShipyardBuilderCommand::ToolMove);
            if(input_.WasPressed(InputAction::EditorToolRotate))RouteControl(ShipyardBuilderCommand::ToolRotate);
            if(scalePressed)RouteControl(ShipyardBuilderCommand::ToolScale);
            if(input_.WasPressed(InputAction::DccToggleToolbar))RouteControl(ShipyardBuilderCommand::DccToggleToolRail);
            if(input_.WasPressed(InputAction::DccToggleSidebar))RouteControl(ShipyardBuilderCommand::DccToggleSidebar);
            if(input_.WasPressed(InputAction::DccCycleAssetFilter))RouteControl(ShipyardBuilderCommand::DccNextAssetPreset);
            if(input_.WasPressed(InputAction::DccConstraintX))RouteControl(ShipyardBuilderCommand::TransformConstraintX);
            if(input_.WasPressed(InputAction::DccConstraintY))RouteControl(ShipyardBuilderCommand::TransformConstraintY);
            if(input_.WasPressed(InputAction::DccConstraintZ))RouteControl(ShipyardBuilderCommand::TransformConstraintZ);
            if(input_.WasPressed(InputAction::EditorFrameSelected))RouteControl(ShipyardBuilderCommand::FrameSelected);
            if(builder_.Model().workspaceMode==ShipyardWorkspaceMode::Build){
                if(input_.WasPressed(InputAction::EditorDeleteModule))RouteControl(ShipyardBuilderCommand::RemoveModule);
                if(input_.WasPressed(InputAction::EditorNudgeLeft))RouteControl(ShipyardBuilderCommand::NudgePort);
                if(input_.WasPressed(InputAction::EditorNudgeRight))RouteControl(ShipyardBuilderCommand::NudgeStarboard);
                if(input_.WasPressed(InputAction::EditorNudgeForward))RouteControl(ShipyardBuilderCommand::NudgeForward);
                if(input_.WasPressed(InputAction::EditorNudgeAft))RouteControl(ShipyardBuilderCommand::NudgeAft);
                if(input_.WasPressed(InputAction::EditorNudgeUp))RouteControl(ShipyardBuilderCommand::NudgeDorsal);
                if(input_.WasPressed(InputAction::EditorNudgeDown))RouteControl(ShipyardBuilderCommand::NudgeVentral);
            }
        }
    }
''', 'studio shortcut router')
    src=once(src,
        '    if(window_.ConsumePrimaryPress(pressX,pressY)){\n'
        '        const auto layout=ShipyardBuilderSystem::Layout(',
        '    if(window_.ConsumePrimaryPress(pressX,pressY)){\n'
        '        // A previous real drag has no click edge; do not suppress the\n'
        '        // NEXT unrelated click after the user has released the mouse.\n'
        '        suppressClick_=false;\n'
        '        const auto layout=ShipyardBuilderSystem::Layout(', 'stale click suppression')
    src=once(src,
        '''                if(picked>=0){
                    RouteControl(ShipyardBuilderCommand::SelectPlaced,picked);
                    if(builder_.Model().transformTool!=ShipyardTransformTool::Select){
                        pointerTransform_=builder_.Model().inspectorTab==ShipyardInspectorTab::Sockets?
                            builder_.BeginSelectedSocketTransform():builder_.BeginSelectedTransform();
                    }
                }
''',
        '''                if(picked>=0){
                    // Select changes target; other tools can free-drag only the
                    // ALREADY selected part. A miss cannot silently retarget.
                    const auto tool=builder_.Model().transformTool;
                    if(StudioToolInteractionPolicy::AllowsViewportReselection(tool)){
                        RouteControl(ShipyardBuilderCommand::SelectPlaced,picked);
                    }else if(StudioToolInteractionPolicy::AllowsGizmoGesture(tool)&&
                             static_cast<std::size_t>(picked)==builder_.Model().selectedPlacedModule){
                        pointerTransform_=builder_.Model().inspectorTab==ShipyardInspectorTab::Sockets?
                            builder_.BeginSelectedSocketTransform():builder_.BeginSelectedTransform();
                        if(pointerTransform_)suppressClick_=true;
                    }
                }
''', 'selected target gesture lock')
    return src

def edit_native(src:str)->str:
    return once(src,
        '''        case WM_KILLFOCUS:
            _inputState.Clear();
            _primaryButtonDown=false; _primaryPressPending=false; _primaryReleasePending=false; _primaryDragDeltaX=0.0f; _primaryDragDeltaY=0.0f; _cameraOrbitDragging=false; _cameraPanDragging=false; _altDown=false;
            return 0;
''',
        '''        case WM_KILLFOCUS:
            _inputState.Clear();
            _primaryButtonDown=false; _primaryPressPending=false; _primaryReleasePending=false; _primaryDragDeltaX=0.0f; _primaryDragDeltaY=0.0f; _cameraOrbitDragging=false; _cameraPanDragging=false;
            // Win32 may never deliver the matching KEYUP after focus leaves.
            // A stale Ctrl blocks W/Q/E/S; a stale Shift changes shortcuts.
            _altDown=false; _controlDown=false; _shiftDown=false;
            _secondaryClickPending=false; _primaryClickPending=false;
            return 0;
''','native modifier and pointer focus reset')

EDITORS={
    'engine/src/studio/StudioApplication.cpp':edit_studio,
    'engine/src/platform/NativeWindow.cpp':edit_native,
}

def stage(root:Path, strict:bool=True):
    pending={};status={}
    if strict:
        p=root/'.git'
        if not p.exists():raise ValueError('Git checkout not found; source ZIP staging cannot be mutated as a local repo')
        if not (root/'engine/include/studio/StudioToolInteractionPolicy.h').is_file():
            raise ValueError('Selection policy header missing; do not apply on old baseline')
    for rel, editor in EDITORS.items():
        path=root/rel
        if not path.is_file():raise ValueError(f'Missing source: {rel}')
        orig=path.read_bytes()
        if b'\r\n' in orig:raise ValueError(f'Unexpected line endings: {rel}')
        blob=git_blob(orig)
        if strict and blob!=PREIMAGES[rel]:
            raise ValueError(f'{rel}: PREIMAGE_CONFLICT expected {PREIMAGES[rel]}, got {blob}; no writes')
        edit=editor(orig.decode('utf-8')).encode('utf-8')
        if edit==orig:raise ValueError(f'{rel}: no changes')
        pending[rel]=(orig,edit)
        status[rel]={'oldGitBlob':blob,'oldSha256':hashlib.sha256(orig).hexdigest(),
                     'newSha256':hashlib.sha256(edit).hexdigest(),'bytes':len(edit)}
    return pending,status

def apply(root:Path,pending:dict,status:dict):
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    backup=root/'.subspace'/'recovery'/'studio-interaction-hotfix'/stamp
    backup.mkdir(parents=True,exist_ok=False)
    for rel,(orig,_) in pending.items():
        b=backup/rel;b.parent.mkdir(parents=True,exist_ok=True);b.write_bytes(orig)
    receipt=backup/'RECEIPT.json'
    meta={'schema':'subspace.studio-interaction-hotfix.v1','status':'BACKED_UP',
          'sourceBaseline':BASELINE,'utc':datetime.now(timezone.utc).isoformat(),
          'backupRoot':str(backup),'files':status}
    receipt.write_text(json.dumps(meta,indent=2)+'\n',encoding='utf-8')
    written=[]
    try:
        # Confirm all originals remain unchanged before any first write.
        for rel,(orig,_) in pending.items():
            if (root/rel).read_bytes()!=orig:raise RuntimeError(f'CONCURRENT_EDIT: {rel}')
        for rel,(orig,new) in pending.items():
            p=root/rel
            if p.read_bytes()!=orig:raise RuntimeError(f'CONCURRENT_EDIT: {rel}')
            fd,tmp=tempfile.mkstemp(prefix='.studio-input-',dir=p.parent)
            try:
                with os.fdopen(fd,'wb') as f:
                    f.write(new);f.flush();os.fsync(f.fileno())
                os.replace(tmp,p)
            finally:
                if os.path.exists(tmp):os.unlink(tmp)
            written.append(rel)
            if hashlib.sha256(p.read_bytes()).hexdigest()!=status[rel]['newSha256']:
                raise RuntimeError(f'VERIFY_FAILED: {rel}')
    except Exception:
        for rel in written:
            (root/rel).write_bytes((backup/rel).read_bytes())
        meta['status']='ROLLED_BACK';receipt.write_text(json.dumps(meta,indent=2)+'\n',encoding='utf-8')
        raise
    meta['status']='APPLIED';receipt.write_text(json.dumps(meta,indent=2)+'\n',encoding='utf-8')
    return receipt

def main(argv=None):
    p=argparse.ArgumentParser(description='Subspace Studio source-guarded Assembly input fix')
    mode=p.add_mutually_exclusive_group(required=True)
    mode.add_argument('--check',action='store_true')
    mode.add_argument('--apply',action='store_true')
    p.add_argument('--root',type=Path,default=Path(__file__).resolve().parents[2]);a=p.parse_args(argv)
    root=a.root.resolve()
    try:
        pending,status=stage(root)
        if a.check:
            print('STUDIO_INPUT_PREIMAGE_CHECK PASS (read-only)')
            for rel,st in status.items():print(rel,st['oldGitBlob'],st['newSha256'])
        else:
            receipt=apply(root,pending,status)
            print('STUDIO_INPUT_SOURCE_APPLIED backup and receipt:',receipt)
            print('NEXT: PCC Full Quality Gate + Windows Assembly interaction checks; do NOT claim Model BOX is fixed.')
    except (OSError,UnicodeError,ValueError,RuntimeError) as e:
        print('STUDIO_INPUT_BLOCKED:',e,file=sys.stderr)
        return 1
    return 0

if __name__=='__main__':raise SystemExit(main())
