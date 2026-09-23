#!/usr/bin/env python3
from pathlib import Path
import argparse, sys

REQUIRED = {
    'engine/src/studio/StudioAxisGizmo.cpp': [
        'ConstructionTransformBasisSystem::AssemblyWorld',
        'model.transformTool==ShipyardTransformTool::Scale',
        'rotated kitbash pieces without swapping serialized X and Y',
    ],
    'engine/src/ship_editor/ShipyardBuilderSystem.cpp': [
        'ConstructionScalePivotSystem::OppositeFaceShift',
        'ConstructionScalePivotSystem::AnchoredModulePosition',
        '"CONSTRUCT"',
        '"EDIT GEOMETRY"',
        '"MIRROR X"', '"MIRROR Y"', '"MIRROR Z"',
    ],
    'engine/src/ship_editor/ShipyardProfessionalVisibleCutover.cpp': [
        '"CONSTRUCT"',
        '"GEOMETRY"',
        'panelButtonsLeft',
        'searchAvailable',
        '"ASSEMBLY"',
    ],
    'engine/src/ship_editor/ShipyardWorkspaceSystem.cpp': [
        'ShipyardWorkspaceMode::Build,ShipyardWorkspaceMode::Interior',
        'case ShipyardWorkspaceMode::Build:return"CONSTRUCT";',
        'case ShipyardWorkspaceMode::Model:return"GEOMETRY";',
        '48,420,false,false,false',
    ],
    'engine/src/modeling/ShipyardModelingSystem.cpp': [
        'aft edge on -Y',
        'forward edge on +Y',
        'const Vector3 a{-x,y,0},b{x,y,0},c{x,-y,-z}',
    ],
}
FORBIDDEN = {
    'engine/src/ship_editor/ShipyardProfessionalVisibleCutover.cpp': [
        'tab(ShipyardBuilderCommand::WorkspaceModel,"MODEL"',
    ],
}

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--root',type=Path,default=Path.cwd());args=ap.parse_args()
    root=args.root.resolve();fail=[]
    for rel,tokens in REQUIRED.items():
        p=root/rel
        if not p.is_file():fail.append(f'MISSING {rel}');continue
        text=p.read_text(encoding='utf-8')
        for token in tokens:
            if token not in text:fail.append(f'{rel}: missing required token {token!r}')
        for token in FORBIDDEN.get(rel,[]):
            if token in text:fail.append(f'{rel}: forbidden legacy token remains {token!r}')
    if fail:
        print('R22 SOURCE ASSERTIONS FAIL',file=sys.stderr)
        for item in fail:print(' - '+item,file=sys.stderr)
        return 2
    print('R22 source assertions PASS')
    return 0
if __name__=='__main__':raise SystemExit(main())
