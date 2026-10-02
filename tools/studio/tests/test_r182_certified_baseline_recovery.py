#!/usr/bin/env python3
from pathlib import Path
from tempfile import TemporaryDirectory
import importlib.util

script=Path(__file__).resolve().parents[1]/'studio_r180_overlay_repair.py'
spec=importlib.util.spec_from_file_location('r182',script);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)

# The enum tail must preserve current DCC actions and append the R178/R179 actions.
input_base='''    DccConstraintClear,\n    DccCycleTransformSpace,\n    DccDuplicateSelection,\n    Count\n'''
out=m.rebuild_input_state(input_base)
assert out.count('DccCycleTransformSpace')==1
assert out.count('DccDuplicateSelection')==1
assert out.index('DccDuplicateSelection') < out.index('PilotForward') < out.index('PlanetaryCommandCycleOverlay') < out.index('Count')

# Planetary Command F5 must be additive to the certified NativeWindow binding.
window='''        case VK_F3: _inputState.SetAction(InputAction::DccCommandSearch, down); break;\n        case VK_F6: _inputState.SetAction(InputAction::ToggleShipInspection, down); break;\n'''
w=m.rebuild_native_window(window)
assert 'VK_F5' in w and 'PlanetaryCommandCycleOverlay' in w

# R182 must validate Studio transform authority in Studio/Shipyard owners, not
# NativeGameApplication. This is the ownership bug that blocked R181.
assert 'StudioTransformMoveDelta::AssemblyAuthored' not in m.POSTCONDITIONS['engine/src/application/NativeGameApplication.cpp']
assert 'StudioTransformMoveDelta::AssemblyAuthored' in m.EXTERNAL_AUTHORITIES['engine/src/studio/StudioApplication.cpp']
assert 'StudioTransformMoveDelta::ModelAuthored' in m.EXTERNAL_AUTHORITIES['engine/src/studio/StudioApplication.cpp']
assert 'TranslateSelectedResolvedParent' in m.EXTERNAL_AUTHORITIES['engine/src/studio/StudioApplication.cpp']

with TemporaryDirectory() as td:
    root=Path(td)
    for rel,tokens in m.EXTERNAL_AUTHORITIES.items():
        p=root/rel;p.parent.mkdir(parents=True,exist_ok=True);p.write_text('\n'.join(tokens),encoding='utf-8')
    m.validate_external_authorities(root)
    # Remove one actual authority token and prove the check fails closed.
    studio=root/'engine/src/studio/StudioApplication.cpp'
    studio.write_text('StudioTransformMoveDelta::AssemblyAuthored\nTranslateSelectedResolvedParent\n',encoding='utf-8')
    try:
        m.validate_external_authorities(root)
        raise AssertionError('authority drift should have failed closed')
    except RuntimeError as e:
        assert 'StudioTransformMoveDelta::ModelAuthored' in str(e)

print('R182 certified-baseline recovery helper tests PASS')
