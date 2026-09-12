#!/usr/bin/env python3
"""Pass791-890 completion-truth audit.

Report-first by default. --strict turns known runtime blockers into a failing
exit code. This deliberately distinguishes placeholder source from certified
runtime behavior instead of treating symbol presence as completion.
"""
from __future__ import annotations
import argparse, json, pathlib, re, sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
TRUTH = ROOT / "content" / "architecture" / "project_completion_truth_v1.json"

CHECKS = [
    ("formation.placeholder", ROOT/"engine/src/formation/FormationSystem.cpp",
     r"placeholder for future implementation"),
    ("home-save.temporary", ROOT/"engine/src/core/persistence/HomeSystemSaveGame.cpp",
     r"Temporary compatible parser|final JSON save schema is locked"),
    ("shipyard.size-class-bridge", ROOT/"engine/src/ship_editor/ShipyardBuilderSystem.cpp",
     r"SHIPYARD_SIZE_AWARE_GENERATION"),
]

def main() -> int:
    ap=argparse.ArgumentParser()
    ap.add_argument("--strict", action="store_true")
    ns=ap.parse_args()
    print("CODENAME SUBSPACE PASS791-890 COMPLETION TRUTH AUDIT")
    print(f"Root: {ROOT}")
    if TRUTH.exists():
        data=json.loads(TRUTH.read_text(encoding="utf-8"))
        print(f"Truth schema: {data.get('schema')}")
    blockers=[]
    for key,path,pattern in CHECKS:
        text=path.read_text(encoding="utf-8",errors="replace") if path.exists() else ""
        hit=bool(re.search(pattern,text,re.I))
        print(f"[{'OPEN' if hit else 'CLEAR'}] {key}: {path.relative_to(ROOT) if path.exists() else 'MISSING'}")
        if hit:blockers.append(key)
    print(f"Known open blockers: {len(blockers)}")
    if ns.strict and blockers:
        return 1
    return 0
if __name__=="__main__":
    raise SystemExit(main())
