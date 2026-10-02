from pathlib import Path
import sys

root = Path(sys.argv[1] if len(sys.argv) > 1 else ".").resolve()
p1598 = (root / "tools/control/static-gates/pass1598_planetary_command_prebuild_repair.cmake").read_text(encoding="utf-8")
p1599 = (root / "tools/control/static-gates/pass1599_planetary_command_migration_matcher.cmake").read_text(encoding="utf-8")

for token in [
    "Invoke-PlanetaryCommandCanonicalVerification",
    "subspace_planetary_command_r158_r177_apply.ps1",
    "prebuild gate superseded safely by R184 canonical verification",
]:
    assert token in p1598, token

for token in [
    "function Invoke-PlanetaryCommandMaterializationIfRequired",
    "apply_planetary_command_r158_r177.py",
    "root-drop Planetary Command installs additive gates/tools first.",
]:
    assert token not in p1598, token

for token in [
    "test_r158_r177_planetary_command_source.py",
    "nullharbor_r179_planetary_command_convergence.cmake",
    "nullharbor_r184_pcc_planetary_orchestration.cmake",
    "matcher gate superseded safely by R179 semantic verification",
]:
    assert token in p1599, token

for token in ["_linewise_matches", "replacementModes", "exact certified preimage is ambiguous"]:
    assert token not in p1599, token

print("R188 historical Planetary prebuild/matcher static-gate normalization PASS")
