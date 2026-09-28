#!/usr/bin/env python3
from pathlib import Path
import sys, subprocess
root=Path(__file__).resolve().parents[3]
cm=(root/'engine/CMakeLists.txt').read_text()
assert 'SubspaceStudioR82R2GateRepairSourceGate' in cm
assert 'SubspaceStudioR82R4FourFailureRecoverySourceGate' in cm
s=(root/'engine/tests/studio_axis_contract_r34_tests.cpp').read_text()
assert 'assert(' not in s
assert 'tx.snap=false;' in s
assert 'tx.snap=true;tx.rotationSnapDegrees=15.0f;' in s
assert 'functional authoring workspaces remain reachable' in (root/'engine/tests/pass735_744_shipyard_dev_pcc_tests.cpp').read_text()
assert 'historical PAINT is now the Appearance domain' in (root/'tools/control/static-gates/pass1268_1292_visible_professional_shipyard_cutover.cmake').read_text()
subprocess.run([sys.executable,str(root/'tools/control/tests/test_r82r2_prebuild_policy.py'),str(root/'SubspaceTools.ps1')],check=True)
print('R82R4 four-failure integration policy: PASS')
