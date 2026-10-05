from pathlib import Path
import importlib.util,sys
p=Path(sys.argv[1]).resolve();spec=importlib.util.spec_from_file_location('r189',p);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
h='#include "fleet/FleetCaptainAiSystem.h"\n    void UpdateFleetCaptains();\n    StrategicFlightSystem _strategicFlight;\n'
o=m.transform_header(h)
assert 'FleetStrategyControlSystem _fleetStrategy' in o and 'PlayerController _playerController' in o
rh='#include "interior/ShipEmbodimentSystem.h"\n    bool strategicFlightMode = false;\n'
ro=m.transform_renderer_header(rh)
assert 'FirstPersonViewPose firstPersonPose' in ro
cm='cmake_minimum_required(VERSION 3.20)\n'
co=m.transform_cmake(cm)
assert 'SubspaceR189StrategyFpsCutoverTests' in co and m.transform_cmake(co)==co
print('R189 materializer helper tests PASS')
source=p.read_text(encoding='utf-8')
assert "b = text.index('\\nvoid NativeGameApplication::UpdateDocking()', a)" in source
assert "b = text.index('\\nvoid NativeGameApplication::UpdateCameraAndSelection()', a)" not in source
baseline='prefix\n'+m.TEMP_UNDOCK_BASELINE+'suffix\n'
descendant='prefix\n'+m.TEMP_UNDOCK_DESCENDANT+'suffix\n'
assert m.normalize_known_preimage(m.TARGETS[1],descendant,baseline)==baseline
try:
    m.normalize_known_preimage(m.TARGETS[1],descendant+'unknown drift\n',baseline)
    raise AssertionError('unknown descendant was incorrectly accepted')
except RuntimeError:
    pass
print('R189 boundary/known-descendant regression tests PASS')

# R189R2: CMake preimage normalization is formatting-only and fail-closed.
cmake_baseline='cmake_minimum_required(VERSION 3.16)\nproject(SubspaceEngine LANGUAGES CXX)\n'
cmake_format='\ufeffcmake_minimum_required(VERSION 3.16)  \r\nproject(SubspaceEngine LANGUAGES CXX)\t\r\n\r\n'
assert m.normalize_known_preimage(m.TARGETS[4],cmake_format,cmake_baseline)==cmake_baseline
try:
    m.normalize_known_preimage(m.TARGETS[4],cmake_baseline+'add_definitions(-DUNKNOWN_DRIFT)\n',cmake_baseline)
    raise AssertionError('semantic CMake drift was incorrectly accepted')
except RuntimeError as exc:
    message=str(exc)
    assert 'baseline_sha256=' in message and 'current_sha256=' in message
    assert 'UNKNOWN_DRIFT' in message
print('R189 CMake format-only normalization/diagnostic tests PASS')

# R189R3: exact governed CMake descendant is preserved rather than reset to Git.
# Exercise the branch with a temporary monkey-patched known hash so the helper
# test stays compact while still proving marker + exact-hash gating.
governed='''cmake_minimum_required(VERSION 3.16)
subspace_studio_duplicate_snap_r73_tests
SubspaceStudioR82R2GateRepairSourceGate
SubspaceStudioR82R4FourFailureRecoverySourceGate
'''
known=m.R189_KNOWN_CMAKE_GOVERNED_DESCENDANT_SHA256
m.R189_KNOWN_CMAKE_GOVERNED_DESCENDANT_SHA256=m._sha256_text(governed)
assert m.normalize_known_preimage(m.TARGETS[4],governed,cmake_baseline)==governed
assert 'SubspaceR189StrategyFpsCutoverTests' in m.transform_cmake(governed)
try:
    m.normalize_known_preimage(m.TARGETS[4],governed+'# unknown semantic drift\n',cmake_baseline)
    raise AssertionError('unknown CMake descendant was incorrectly accepted')
except RuntimeError:
    pass
m.R189_KNOWN_CMAKE_GOVERNED_DESCENDANT_SHA256=known
print('R189 governed CMake descendant preservation tests PASS')

# R191R1: historical R189 Full Gate guard accepts exact governed forward
# descendants only; marker presence without exact hash remains fail-closed.
fixture="""GameplayControlMode fixture\nR191 marker\n"""
old_spec=m.R189_KNOWN_FORWARD_DESCENDANTS.get(m.TARGETS[0])
m.R189_KNOWN_FORWARD_DESCENDANTS[m.TARGETS[0]]=(m._sha256_text(fixture),("GameplayControlMode", "R191 marker"))
assert m._is_known_forward_descendant(m.TARGETS[0],fixture)
assert m._satisfies_r189_target(m.TARGETS[0],fixture)
assert not m._is_known_forward_descendant(m.TARGETS[0],fixture+'unknown drift\n')
assert not m._satisfies_r189_target(m.TARGETS[0],fixture+'unknown drift\n')
if old_spec is None:
    del m.R189_KNOWN_FORWARD_DESCENDANTS[m.TARGETS[0]]
else:
    m.R189_KNOWN_FORWARD_DESCENDANTS[m.TARGETS[0]]=old_spec
print('R189/R191 forward-descendant gate compatibility tests PASS')

# R192: a second exact forward lineage may coexist with the certified R191 one.
fixture192="""StarterInteriorScene fixture
ExecuteInteriorInteraction fixture
"""
old192=m.R189_KNOWN_R192_FORWARD_DESCENDANTS.get(m.TARGETS[0])
m.R189_KNOWN_R192_FORWARD_DESCENDANTS[m.TARGETS[0]]=(m._sha256_text(fixture192),("StarterInteriorScene", "ExecuteInteriorInteraction"))
assert m._is_known_forward_descendant(m.TARGETS[0],fixture192)
assert not m._is_known_forward_descendant(m.TARGETS[0],fixture192+'unknown drift\n')
if old192 is None:
    del m.R189_KNOWN_R192_FORWARD_DESCENDANTS[m.TARGETS[0]]
else:
    m.R189_KNOWN_R192_FORWARD_DESCENDANTS[m.TARGETS[0]]=old192
print('R189/R192 forward-descendant gate compatibility tests PASS')
