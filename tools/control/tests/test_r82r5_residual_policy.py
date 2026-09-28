#!/usr/bin/env python3
from pathlib import Path
import subprocess, sys
root=Path(__file__).resolve().parents[3]
t=(root/'engine/tests/studio_duplicate_snap_preservation_r73_tests.cpp').read_text(encoding='utf-8')
assert 'const auto expected=' in t and 'const auto staged=g;' in t
assert 'const auto& expected=' not in t
assert 'if(!validCandidate)return 1;' in t
hist=(root/'engine/tests/pass735_744_shipyard_dev_pcc_tests.cpp').read_text(encoding='utf-8')
assert 'functional authoring workspaces remain reachable' in hist
assert 'const auto primary=ShipyardWorkspaceSystem::PrimaryWorkspaces();' not in hist
old=(root/'tools/control/static-gates/pass1466_1505_shipyard_foundry.cmake').read_text(encoding='utf-8')
assert 'p1505_require("engine/src/ship_editor/ShipyardBuilderSystem.cpp" "GLOBAL-SHIP (press again for LOCAL)")' not in old
assert 'bool ShipyardBuilderSystem::SetTransformConstraint(' in old
subprocess.run([sys.executable,str(root/'tools/control/tests/test_r82r4_four_failure_policy.py')],check=True)
print('R82R5 stable snapshot and historical gate policy: PASS')
