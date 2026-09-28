from pathlib import Path
import re
root=Path(__file__).resolve().parents[3]
ps=(root/'SubspaceTools.ps1').read_text(encoding='utf-8')
assert 'Invoke-StudioTransformAuthorityNormalizationIfRequired' in ps
assert 'apply_studio_r43_r52_transform_authority.py' in ps
for gate in ('Invoke-BuildRender','Invoke-FullGate','Invoke-FastDevelopmentGate'):
    start=ps.index('function '+gate)
    end=ps.find('\nfunction ',start+10)
    block=ps[start:end if end!=-1 else len(ps)]
    a=block.index('Studio Construct source cutover')
    b=block.index('Studio bulk polish normalization')
    c=block.index('Studio transform-space authority')
    d=block.index('Pass/source continuity audit')
    assert a < b < c < d, (gate,a,b,c,d)
print('R43-R52 PCC prebuild ordering: PASS')
