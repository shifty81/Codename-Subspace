from pathlib import Path
import re
ROOT=Path(__file__).resolve().parents[3]
text=(ROOT/'SubspaceTools.ps1').read_text(encoding='utf-8')
def body(name):
    m=re.search(rf'function {re.escape(name)}\s*\{{(.*?)(?=\nfunction |\Z)',text,re.S)
    assert m, name
    return m.group(1)
for name in ('Invoke-BuildHeadless','Invoke-BuildRender','Invoke-FullGate'):
    b=body(name)
    order=['Invoke-StudioConstructSourceCutoverIfRequired','Invoke-StudioBulkPolishNormalizationIfRequired',
           'Invoke-StudioTransformAuthorityNormalizationIfRequired','Invoke-StudioTransformUiHygieneNormalizationIfRequired',
           'Invoke-StudioSelectionPlacementNormalizationIfRequired','Invoke-StudioConvergenceAuditNormalizationIfRequired',
           'Invoke-StudioR82R1CorrectionsIfRequired','Invoke-PassContinuityAudit']
    positions=[b.find(x) for x in order]
    assert all(x>=0 for x in positions),(name,positions)
    assert positions==sorted(positions),(name,positions)
# R82R2 supersedes the R82R1 freshness marker while preserving the R82R1
# source gate in engine/CMakeLists.txt. Do not force an older CTest graph.
b=body('Invoke-TestsOnly')
assert 'Invoke-StudioR82R1CorrectionsIfRequired' in b
assert 'Invoke-StudioR82R2GateRepairIfRequired' in b
assert b.index('Invoke-StudioR82R1CorrectionsIfRequired') < b.index('Invoke-StudioR82R2GateRepairIfRequired')
assert 'SubspaceStudioR82R2GateRepairSourceGate' in b
assert 'predates the R82R2 CTest graph' in b
cmake=(ROOT/'engine/CMakeLists.txt').read_text(encoding='utf-8')
assert 'SubspaceStudioR82R1CorrectiveSourceGate' in cmake
assert 'SubspaceStudioR82R2GateRepairSourceGate' in cmake
print('R82R1/R82R2 PCC prebuild/CTest freshness policy: PASS')
