#!/usr/bin/env python3
from __future__ import annotations
import argparse
from pathlib import Path
import sys

REQUIRED = {
    'engine/src/ship_editor/ShipyardBuilderSystem.cpp': [
        'ConstructionScalePivotSystem.h',
        'opposite face anchored',
        '"CONSTRUCT"',
        '"MIRROR X"',
        '"TRANSFORM SPACE: VIEW"',
        '"TRANSFORM SPACE: PARENT"',
        '"TRANSFORM SPACE: OBJECT"',
    ],
    'engine/src/ship_editor/ShipyardProfessionalVisibleCutover.cpp': [
        'ConstructionUiLayoutPolicy.h',
        'one authoritative workspace strip',
        '"CONSTRUCT"',
        '"BACK"',
        'ConstructionUiLayoutPolicy::TabWidth',
        'ConstructionUiLayoutPolicy::ShowAssetSecondaryActions',
        '"GEOMETRY"',
        '"ASSEMBLY"',
    ],
    'engine/src/ship_editor/ShipyardWorkspaceSystem.cpp': [
        'ShipyardWorkspaceMode::Build,ShipyardWorkspaceMode::Interior',
        'case ShipyardWorkspaceMode::Build:return"CONSTRUCT";',
        'case ShipyardWorkspaceMode::Model:return"GEOMETRY";',
        'add("tool_rail","Tools","tool_left",true,1.0f,48,420,false,false,false);',
        'Parent/Object are explicit alternatives.',
    ],
    'engine/src/modeling/ShipyardModelingSystem.cpp': [
        'Coordinate contract is context-neutral +Y forward.',
        'aft edge on -Y',
    ],
}
FORBIDDEN = {
    'engine/src/ship_editor/ShipyardProfessionalVisibleCutover.cpp': [
        'const float y=l.workspaceBarY+l.workspaceBarHeight+3.0f*s;',
        'tab(ShipyardBuilderCommand::WorkspaceModel,"MODEL"',
    ],
    'engine/src/ship_editor/ShipyardWorkspaceSystem.cpp': [
        'ShipyardWorkspaceSystem::PrimaryWorkspaces(){return {ShipyardWorkspaceMode::Build,ShipyardWorkspaceMode::Model,',
    ],
}

def main() -> int:
    ap=argparse.ArgumentParser();ap.add_argument('--root',type=Path,default=Path.cwd());args=ap.parse_args()
    root=args.root.resolve();errors=[]
    for rel,tokens in REQUIRED.items():
        path=root/rel
        if not path.is_file(): errors.append(f'missing {rel}'); continue
        text=path.read_text(encoding='utf-8')
        for token in tokens:
            if token not in text: errors.append(f'{rel}: missing {token!r}')
        for token in FORBIDDEN.get(rel,[]):
            if token in text: errors.append(f'{rel}: retired token still present {token!r}')
    if errors:
        print('R32 SOURCE ASSERTIONS FAILED',file=sys.stderr)
        for e in errors: print('  '+e,file=sys.stderr)
        return 2
    print('R32 source assertions PASS: Construct/axis/layout cutover is physically present.')
    return 0
if __name__=='__main__': raise SystemExit(main())
