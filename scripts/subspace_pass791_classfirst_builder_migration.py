#!/usr/bin/env python3
from __future__ import annotations
import argparse, pathlib, re, sys

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--root", required=True)
    ap.add_argument("--check", action="store_true")
    args=ap.parse_args()
    root=pathlib.Path(args.root).resolve()
    p=root/"engine/src/ship_editor/ShipyardBuilderSystem.cpp"
    text=p.read_text(encoding="utf-8")
    r2="SHIPYARD_SIZE_AWARE_GENERATION"
    if r2 not in text:
        print("Pass790R2 size-aware marker is absent; no legacy bridge detected.")
        return 0
    print("Legacy Pass790R2 target-size -> class bridge is present.")
    if args.check:
        return 1
    # We deliberately refuse to mutate the live tree here. Forge must replace
    # this source transactionally from an exact current preimage.
    print("REFUSED: use the exact-byte Forge replacement patch; no in-place mutation performed.")
    return 2

if __name__=="__main__":
    raise SystemExit(main())
