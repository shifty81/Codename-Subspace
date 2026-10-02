from pathlib import Path
import sys
root=Path(sys.argv[1]).resolve()
p=root/"SubspaceTools.ps1"
text=p.read_text(encoding="utf-8")
need=[
"function Invoke-PlanetaryCommandMaterializationIfRequired",
"apply_planetary_command_r158_r177.py",
"test_r158_r177_planetary_command_source.py",
"R177R1: root-drop Planetary Command installs additive gates/tools first.",
]
missing=[x for x in need if x not in text]
assert not missing, missing
cmake=text.index("function Invoke-CMakeBuild")
call=text.index("Invoke-PlanetaryCommandMaterializationIfRequired",cmake)
builddir=text.index("$buildDir = Get-BuildDirectory",cmake)
assert call < builddir
print("PASS: R177R1 Planetary Command prebuild repair policy")
