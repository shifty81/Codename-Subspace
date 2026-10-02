#!/usr/bin/env python3
from __future__ import annotations
import argparse, hashlib, json, shutil, sys
from datetime import datetime, timezone
from pathlib import Path

def digest(p: Path) -> str:
    return hashlib.sha256(p.read_bytes()).hexdigest()

def _linewise_matches(text: str, before: str):
    source = text.splitlines(keepends=True)
    needle = before.splitlines()
    if not needle or len(source) < len(needle):
        return []

    normalized = [line.strip() for line in needle]
    offsets = []
    pos = 0
    for line in source:
        offsets.append(pos)
        pos += len(line)
    offsets.append(pos)

    matches = []
    width = len(needle)
    for start in range(0, len(source) - width + 1):
        window = source[start:start + width]
        if all(window[i].strip() == normalized[i] for i in range(width)):
            matches.append((offsets[start], offsets[start + width], window))
    return matches

def _indent_after(after: str, matched_window) -> str:
    base = ""
    for line in matched_window:
        raw = line.rstrip("\r\n")
        if raw.strip():
            base = raw[:len(raw) - len(raw.lstrip())]
            break
    if not base:
        return after
    return "\n".join((base + line if line else line) for line in after.splitlines())

def replace_approved(text: str, before: str, after: str, marker: str, label: str, path: str):
    if marker in text:
        return text, False, "already"

    exact = text.count(before)
    if exact == 1:
        return text.replace(before, after, 1), True, "exact"
    if exact > 1:
        raise RuntimeError(f"{label}: exact certified preimage is ambiguous in {path} ({exact} matches)")

    matches = _linewise_matches(text, before)
    if len(matches) != 1:
        raise RuntimeError(
            f"{label}: expected one certified preimage in {path}; "
            f"exact=0 indentation-normalized={len(matches)}"
        )

    start, end, window = matches[0]
    replacement = _indent_after(after, window)
    return text[:start] + replacement + text[end:], True, "indent-normalized"

def main() -> int:
    ap = argparse.ArgumentParser(description="Guarded R158-R177 Planetary Command source materialization")
    ap.add_argument("--root", required=True)
    ap.add_argument("--dry-run", action="store_true")
    ns = ap.parse_args()

    root = Path(ns.root).resolve()
    plan_path = root / "tools/planetary/r158_r177_transforms.json"
    if not plan_path.is_file():
        raise RuntimeError("transform plan missing: " + str(plan_path))

    plan = json.loads(plan_path.read_text(encoding="utf-8"))
    transforms = plan["transforms"]
    changed, original, modes = {}, {}, []

    for t in transforms:
        p = root / t["path"]
        if not p.is_file():
            raise RuntimeError("missing target: " + t["path"])
        if t["path"] not in original:
            original[t["path"]] = p.read_text(encoding="utf-8")
            changed[t["path"]] = original[t["path"]]

        updated, did_change, mode = replace_approved(
            changed[t["path"]], t["before"], t["after"],
            t["marker"], t["label"], t["path"])
        changed[t["path"]] = updated
        if did_change:
            modes.append((t["path"], t["label"], mode))

    modified = [rel for rel in changed if changed[rel] != original[rel]]
    if not modified:
        print("PASS: R158-R177 Planetary Command source is already materialized.")
        return 0

    required = root / "engine/include/economy/PlanetaryCommandMapSystem.h"
    if not required.is_file():
        raise RuntimeError("PlanetaryCommandMapSystem.h missing; apply the PCC package before source materialization.")

    if ns.dry_run:
        print(f"PASS: R158-R177 dry-run verified {len(modified)} source file(s).")
        for rel, label, mode in modes:
            print(f"  WOULD UPDATE {rel} :: {label} [{mode}]")
        return 0

    stamp = datetime.now(timezone.utc).strftime("%Y%m%d-%H%M%S")
    backup = root / "artifacts" / "migrations" / f"planetary-command-r158-r177-{stamp}"
    backup.mkdir(parents=True, exist_ok=True)
    written = []

    try:
        for rel in modified:
            src = root / rel
            dst = backup / rel
            dst.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(src, dst)

        for rel in modified:
            (root / rel).write_text(changed[rel], encoding="utf-8", newline="\n")
            written.append(rel)
    except Exception:
        for rel in written:
            src = backup / rel
            if src.is_file():
                shutil.copy2(src, root / rel)
        raise

    receipt = {
        "schema": "subspace.source-migration-receipt.v1",
        "migration": "R158-R177 Planetary Command / R177R2 matcher repair",
        "timestamp": datetime.now(timezone.utc).isoformat(),
        "backup": str(backup),
        "replacementModes": [
            {"path": p, "label": l, "mode": m} for p, l, m in modes
        ],
        "files": [
            {"path": rel, "sha256": digest(root / rel)} for rel in modified
        ],
    }
    (backup / "receipt.json").write_text(json.dumps(receipt, indent=2) + "\n", encoding="utf-8")

    print(f"PASS: R158-R177 Planetary Command materialized {len(modified)} source file(s).")
    for rel, label, mode in modes:
        print(f"  UPDATED {rel} :: {label} [{mode}]")
    print("Backup:", backup)
    return 0

if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as e:
        print("FAIL:", e, file=sys.stderr)
        raise SystemExit(1)
