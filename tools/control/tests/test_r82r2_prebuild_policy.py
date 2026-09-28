from pathlib import Path
import sys
p=Path(sys.argv[1] if len(sys.argv)>1 else 'SubspaceTools.ps1')
s=p.read_text(encoding='utf-8')
required=[
 'function Invoke-StudioR82R2GateRepairIfRequired',
 'Studio R82R2 gate repair',
 'SubspaceStudioR82R2GateRepairSourceGate',
]
for token in required:
    assert token in s, token
for fn in ('Invoke-BuildHeadless','Invoke-BuildRender','Invoke-TestsOnly','Invoke-FullGate','Invoke-FastDevelopmentGate'):
    start=s.index('function '+fn)
    end=s.find('\nfunction ',start+10)
    if end<0:end=len(s)
    block=s[start:end]
    a=block.find('Invoke-StudioR82R1CorrectionsIfRequired')
    b=block.find('Invoke-StudioR82R2GateRepairIfRequired')
    assert a>=0 and b>a,(fn,a,b)
print('R82R2 PCC prebuild ordering: PASS')
