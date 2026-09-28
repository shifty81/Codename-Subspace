#!/usr/bin/env python3
"""Prove the already-migrated R82R1 source, without replaying older preimages.

This is an additive migration re-entry guard, not a bypass of source gates.
A partial/older tree follows the original guarded migration chain unchanged.
A tree claiming the final milestone must pass ALL historical static source gates.
"""
from __future__ import annotations
import argparse
from pathlib import Path
import shutil
import subprocess
import sys

GATES = (
    'pass1511_1520_studio_bulk_polish.cmake',
    'pass1521_1530_studio_transform_authority.cmake',
    'pass1531_1540_studio_transform_ui_hygiene.cmake',
    'pass1541_1550_studio_selection_placement.cmake',
    'pass1551_1560_studio_convergence_audit.cmake',
    'pass1561_1564_studio_r82r1_corrective.cmake',
)
# Actual final-state signatures, not a migration receipt or an unrelated file.
# R82R2 may be present or pending; its own guarded migration still runs.
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
    """True only for a complete, source-gate-proven later state; never modifies files.

    Missing final-milestone signatures means an earlier/partial tree: let the
    original sequential exact-preimage migrations decide what is applicable.
    Present signatures but failing historical gates is drift: fail closed.
    """
    root = Path(root).resolve()
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
    ap=argparse.ArgumentParser()
    ap.add_argument('--root', required=True)
    args=ap.parse_args()
    try:
        if verify_complete_r82r1(Path(args.root)):
            print('PASS: R33-R82R1 already-normalized source proven by all six source gates')
            raise SystemExit(0)
        print('EARLIER/PARTIAL: normal guarded migration chain remains authoritative')
        raise SystemExit(3)
    except Exception as exc:
        print(f'R82R3 NORMALIZED-STATE PROOF BLOCKED: {exc}',file=sys.stderr)
        raise SystemExit(2)
