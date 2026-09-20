from pathlib import Path
import importlib.util
ROOT=Path(__file__).resolve().parents[3]
installer=ROOT/'tools/studio/apply_modeling_closure.py'
spec=importlib.util.spec_from_file_location('closure',installer);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
assert m.BASELINE=='acabf9c5df9e1c382eb87bc1b7264354355828f9'
assert len(m.EDITORS)>=16
assert 'engine/src/application/NativeBattlefieldRenderer.cpp' in m.EDITORS
assert (ROOT/'engine/include/studio/StudioModelDocumentCodec.h').is_file()
assert (ROOT/'engine/src/studio/StudioModelDocumentCodec.cpp').is_file()
assert 'ModelSelectPrimitive' in (ROOT/'tools/studio/apply_modeling_closure.py').read_text()
print('Studio modeling closure payload tests PASS')
