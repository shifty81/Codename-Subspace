from __future__ import annotations
import argparse, hashlib, subprocess
from pathlib import Path

BASELINE_COMMIT = "8484a081f8d60be980d3aac300ba2f1f9397cd75"
VERIFY_SHA256 = "23cdef1341421b816d3b1ee65f5e5fc078be4e1b0145214be81d59992be67739"
BLOCK = """function Invoke-PlanetaryCommandCanonicalVerification {\n    $verifier = Join-Path $Global:SubspaceRoot 'scripts\\subspace_planetary_command_r158_r177_apply.ps1'\n    if (-not (Test-Path -LiteralPath $verifier -PathType Leaf)) { return }\n\n    # R184: R158-R177 is historical migration lineage only. Modern trees are\n    # verified semantically by the R179/R183 compatibility entrypoint; the PCC\n    # must never replay exact/indent-normalized text transforms over newer source.\n    Write-Log 'Planetary Command canonical source verification before native build.' 'INFO'\n    Invoke-ProjectScript -RelativePath 'scripts\\subspace_planetary_command_r158_r177_apply.ps1' `\n        -Arguments @('-Root', $Global:SubspaceRoot)\n    Write-Log 'Planetary Command canonical source verification passed; historical text replay is retired.' 'PASS'\n}\n\n"""
CALL = "    Invoke-PlanetaryCommandCanonicalVerification\n\n"

def norm(text: str) -> str:
    return text.replace("\r\n", "\n").replace("\r", "\n")

def main() -> int:
    ap=argparse.ArgumentParser()
    ap.add_argument("--root", required=True)
    ap.add_argument("--baseline-file")
    args=ap.parse_args()
    root=Path(args.root).resolve()
    pcc=root/"SubspaceTools.ps1"
    verifier=root/"scripts/subspace_planetary_command_r158_r177_apply.ps1"
    current=norm(pcc.read_text(encoding="utf-8"))
    if current.count(BLOCK)!=1:
        raise AssertionError(f"expected exactly one R184 verifier block, found {current.count(BLOCK)}")
    without=current.replace(BLOCK,"",1)
    if without.count(CALL)!=1:
        raise AssertionError(f"expected exactly one R184 verifier call, found {without.count(CALL)}")
    without=without.replace(CALL,"",1)
    if args.baseline_file:
        baseline=norm(Path(args.baseline_file).read_text(encoding="utf-8"))
    else:
        r=subprocess.run(["git","show",f"{BASELINE_COMMIT}:SubspaceTools.ps1"],cwd=root,text=True,capture_output=True,check=False)
        if r.returncode:
            raise AssertionError(f"git baseline read failed: {r.stderr.strip()}")
        baseline=norm(r.stdout)
    if without != baseline:
        raise AssertionError("R184 PCC is not exactly Git 8484a08 plus the certified verifier insertion")
    digest=hashlib.sha256(verifier.read_bytes()).hexdigest()
    if digest != VERIFY_SHA256:
        raise AssertionError(f"verification-only Planetary script hash drifted: {digest}")
    forbidden=(
        "package is installed but guarded source materialization is pending",
        "expected one certified preimage",
        "indentation-normalized",
    )
    for token in forbidden:
        if token in current:
            raise AssertionError(f"retired PCC materializer token returned: {token}")
    print("PASS: R184 PCC = Git 8484a08 + one canonical Planetary verification insertion")
    print("PASS: verification-only Planetary entrypoint SHA256 is certified")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
