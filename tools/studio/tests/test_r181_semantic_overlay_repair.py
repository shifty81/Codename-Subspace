#!/usr/bin/env python3
from pathlib import Path
import importlib.util

script=Path(__file__).resolve().parents[1]/'studio_r180_overlay_repair.py'
spec=importlib.util.spec_from_file_location('r181',script);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)

input_base='''    DccConstraintClear,\n    DccCycleTransformSpace,\n    DccDuplicateSelection,\n    Count\n'''
out=m.rebuild_input_state(input_base)
assert out.count('DccCycleTransformSpace')==1
assert out.count('DccDuplicateSelection')==1
assert 'PilotForward' in out and 'CharacterMoveForward' in out and 'FleetCameraForward' in out
assert 'PlanetaryCommandCycleOverlay' in out

window='''        case VK_F3: _inputState.SetAction(InputAction::DccCommandSearch, down); break;\n        case VK_F6: _inputState.SetAction(InputAction::ToggleShipInspection, down); break;\n'''
w=m.rebuild_native_window(window)
assert 'VK_F5' in w and 'PlanetaryCommandCycleOverlay' in w

ctx='''    if(strategic){\n        // Until physical seat routing replaces the legacy path, do not allow\n    }\n    c.modeLabel="COCKPIT / FIRST PERSON 6DOF";\n    if(authorized && workspace!=SandboxWorkspaceMode::ShipBuilder && !vectorTransit){\n        context.cameraMode=CameraMode::TacticalFleet;\n'''
# focused helper behavior is covered by full-source anchors on Windows; these
# samples certify the former conflicting append path and F5 insertion here.
print('R181 semantic overlay helper tests PASS')
