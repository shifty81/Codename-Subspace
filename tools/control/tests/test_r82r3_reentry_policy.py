#!/usr/bin/env python3
from pathlib import Path
import importlib.util
import tempfile
import subprocess
import sys
import shutil

ROOT=Path(__file__).resolve().parents[3]
sys.path.insert(0,str(ROOT/'tools/studio'))
from studio_verified_normalized_state import GATES,MILESTONE,verify_complete_r82r1

def main():
    assert shutil.which('cmake'), 'cmake required for R82R3 verification'
    with tempfile.TemporaryDirectory() as d:
        root=Path(d)
        assert not verify_complete_r82r1(root), 'missing milestone must use old guarded chain'
        for rel, signatures in MILESTONE.items():
            p=root/rel;p.parent.mkdir(parents=True,exist_ok=True)
            p.write_text('\n'.join(signatures)+'\n',encoding='utf8')
        gate_dir=root/'tools/control/static-gates';gate_dir.mkdir(parents=True)
        for name in GATES:(gate_dir/name).write_text('cmake_minimum_required(VERSION 3.20)\nmessage(STATUS "fixture PASS")\n',encoding='utf8')
        assert verify_complete_r82r1(root), 'full normalized state must skip historical preimages'
        for name in GATES:
            (gate_dir/name).write_text('message(FATAL_ERROR "intentional fixture failure")\n',encoding='utf8')
            try:verify_complete_r82r1(root)
            except RuntimeError as ex:assert name in str(ex) and 'intentional fixture failure' in str(ex)
            else:raise AssertionError(name+' failing source gate incorrectly accepted')
            (gate_dir/name).write_text('cmake_minimum_required(VERSION 3.20)\n',encoding='utf8')
        assert verify_complete_r82r1(root)
        # Every prior migration must return before needing its old preimage.
        for name in (
          'apply_studio_r33_r42_bulk_polish.py',
          'apply_studio_r43_r52_transform_authority.py',
          'apply_studio_r53_r62_transform_ui_hygiene.py',
          'apply_studio_r63_r72_selection_placement.py',
          'apply_studio_r73_r82_convergence_audit.py',
          'apply_studio_r82r1_corrective_audit.py',
        ):
            script=ROOT/'tools/studio'/name
            r=subprocess.run([sys.executable,str(script),'--root',str(root),'--apply'],capture_output=True,text=True)
            assert r.returncode==0,(name,r.stdout,r.stderr)
            assert 'historical migration no-op' in r.stdout,(name,r.stdout)
    print('R82R3 migration re-entry tests: PASS (complete, partial, gate failure, six script reentries)')
if __name__=='__main__':main()
