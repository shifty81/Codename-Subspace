#!/usr/bin/env python3
"""Prove R82R1 Studio source after R182 certified-baseline gameplay recovery.

R182 verifies Studio authority in its actual owner files, then rebuilds only the
seven known R178/R179 overlay-regression files from certified 8484a08 source plus
explicit gameplay deltas. R186 treats that completed repair as a one-time migration:
later governed descendants are accepted when the receipt and semantic authorities
remain valid. Historical Studio gates remain authoritative and unchanged.
"""
from __future__ import annotations
import argparse
from pathlib import Path
import shutil
import subprocess
import sys

from studio_r180_overlay_repair import repair_known_overlay_regression

GATES = (
    'nullharbor_r182_certified_baseline_recovery.cmake',
    'pass1511_1520_studio_bulk_polish.cmake',
    'pass1521_1530_studio_transform_authority.cmake',
    'pass1531_1540_studio_transform_ui_hygiene.cmake',
    'pass1541_1550_studio_selection_placement.cmake',
    'pass1551_1560_studio_convergence_audit.cmake',
    'pass1561_1564_studio_r82r1_corrective.cmake',
)
MILESTONE = {
    'engine/src/ship_editor/ShipyardBuilderSystem.cpp': (
        'BeginDuplicateSelectedPlacement()',
        'DeleteSelectionSafe',
        'ROTATE / EULER components',
        'PreserveDuplicateAuthoredTraits(authoredBeforeSnap,model_.dragPreview.ghost)',
    ),
    'engine/src/studio/StudioGizmoOverlay.cpp': (
        'EffectiveSpaceName(snapshot.transformTool,snapshot.effectiveSpace)',
    ),
    'engine/src/ship_editor/ShipyardTransformSystem.cpp': (
        'ConstructionTransformBasisSystem::AssemblyLocal(tx.before,true)',
    ),
    'engine/src/studio/StudioAxisGizmo.cpp': (
        'StudioTransformStatusPolicy::HasEffectiveOverride',
    ),
}

def verify_complete_r82r1(root: Path, *, cmake: str | None = None) -> bool:
    root = Path(root).resolve()
    repair_known_overlay_regression(root)

    for relative, signatures in MILESTONE.items():
        path = root / relative
        if not path.is_file():
            return False
        source = path.read_text(encoding='utf-8')
        if any(signature not in source for signature in signatures):
            return False
    cmake_exe = cmake or shutil.which('cmake')
    if not cmake_exe:
        raise RuntimeError('R82R1 final source detected, but CMake is unavailable for historical source proof')
    gate_dir = root / 'tools/control/static-gates'
    for filename in GATES:
        gate = gate_dir / filename
        if not gate.is_file():
            raise RuntimeError(f'R82R1 final source detected but source gate is missing: {gate}')
        result = subprocess.run([cmake_exe, '-P', str(gate)], cwd=str(root),
                                text=True, capture_output=True, check=False)
        if result.returncode:
            detail=(result.stderr+'\n'+result.stdout).strip()
            raise RuntimeError(f'R82R1 final source detected, but {filename} rejected it: {detail}')
    return True

if __name__=='__main__':
    ap=argparse.ArgumentParser(); ap.add_argument('--root', required=True); args=ap.parse_args()
    try:
        if verify_complete_r82r1(Path(args.root)):
            print('PASS: R33-R82R1 normalized source proven after R182 certified-baseline recovery')
            raise SystemExit(0)
        print('EARLIER/PARTIAL: normal guarded migration chain remains authoritative')
        raise SystemExit(3)
    except Exception as exc:
        print(f'R182 NORMALIZED-STATE PROOF BLOCKED: {exc}',file=sys.stderr)
        raise SystemExit(2)
