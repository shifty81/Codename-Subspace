from pathlib import Path
root=Path(__file__).resolve().parents[3]
ps=(root/'SubspaceTools.ps1').read_text(encoding='utf-8')
assert 'Invoke-StudioSelectionPlacementNormalizationIfRequired' in ps
assert 'apply_studio_r63_r72_selection_placement.py' in ps
for gate in ('Invoke-BuildRender','Invoke-FullGate','Invoke-FastDevelopmentGate'):
    start=ps.index('function '+gate)
    end=ps.find('\nfunction ',start+10)
    block=ps[start:end if end!=-1 else len(ps)]
    a=block.index('Studio Construct source cutover')
    b=block.index('Studio bulk polish normalization')
    c=block.index('Studio transform-space authority')
    d=block.index('Studio transform UI/state hygiene')
    e=block.index('Studio selection/placement ergonomics')
    f=block.index('Pass/source continuity audit')
    assert a < b < c < d < e < f, (gate,a,b,c,d,e,f)
print('R63-R72 PCC prebuild ordering: PASS')
