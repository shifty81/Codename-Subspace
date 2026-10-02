from pathlib import Path
import sys

root = Path(sys.argv[1] if len(sys.argv) > 1 else '.').resolve()
r183 = (root / 'tools/control/static-gates/nullharbor_r183_planetary_materialization_authority.cmake').read_text(encoding='utf-8')
r186 = (root / 'tools/control/static-gates/nullharbor_r186_recovery_descendant_compatibility.cmake').read_text(encoding='utf-8')

required = (
    'test_r158_r177_planetary_command_source.py',
    'nullharbor_r179_planetary_command_convergence.cmake',
    'nullharbor_r184_pcc_planetary_orchestration.cmake',
    'canonical-semantic-verification',
)
for token in required:
    assert token in r183, f'R183 gate missing semantic token: {token}'

retired = (
    'PLANETARY_AUTHORITIES',
    'PLANETARY_MARKERS',
    'validate_planetary_authorities',
    'write_planetary_materialization_markers',
)
for token in retired:
    assert token not in r183, f'R183 gate still depends on removed implementation token: {token}'

assert 'current tree remains a semantically valid descendant' in r186
assert 'recorded afterSha256 remains provenance' in r186
print('R187 R183 semantic-gate supersession verification PASS')
