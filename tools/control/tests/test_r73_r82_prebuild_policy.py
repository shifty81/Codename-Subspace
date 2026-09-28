from pathlib import Path
root=Path(__file__).resolve().parents[3]
ps=(root/'SubspaceTools.ps1').read_text(encoding='utf-8')
assert 'Invoke-StudioConvergenceAuditNormalizationIfRequired' in ps
assert 'apply_studio_r73_r82_convergence_audit.py' in ps
for gate in ('Invoke-BuildRender','Invoke-FullGate','Invoke-FastDevelopmentGate'):
    start=ps.index('function '+gate)
    end=ps.find('\nfunction ',start+10)
    block=ps[start:end if end!=-1 else len(ps)]
    markers=[
        'Studio Construct source cutover',
        'Studio bulk polish normalization',
        'Studio transform-space authority',
        'Studio transform UI/state hygiene',
        'Studio selection/placement ergonomics',
        'Studio convergence audit normalization',
        'Pass/source continuity audit',
    ]
    pos=[block.index(x) for x in markers]
    assert pos==sorted(pos),(gate,pos)
print('R73-R82 PCC prebuild ordering: PASS')
