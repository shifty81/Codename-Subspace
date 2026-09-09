#!/usr/bin/env python3
"""
Codename: Subspace - Contract Scanner

Scans the C++ engine source files for forbidden API usage that
violates engine determinism contracts. Simulation code (ECS, physics,
AI, procedural generation) must not use non-deterministic APIs so that
replays and multiplayer stay in sync.

Usage:
    python tools/contract_scan.py [--path engine]

Returns exit code 1 if violations are found.
"""

import argparse
import pathlib
import sys

# APIs forbidden in deterministic simulation code
FORBIDDEN_PATTERNS = [
    ("std::chrono", "Non-deterministic time source"),
    ("time(", "Wall-clock time access"),
    ("rand(", "Non-deterministic randomness"),
    ("<random>", "OS-dependent random header"),
    ("srand(", "Non-deterministic seed"),
    ("clock(", "Wall-clock time access"),
    ("gettimeofday", "Wall-clock time access"),
    ("clock_gettime", "Wall-clock time access"),
]

# Directories under engine/src/ that must remain deterministic
SIMULATION_DIRS = [
    "core/ecs",
    "core/physics",
    "ai",
    "procedural",
    "navigation",
    "combat",
    "ships",
]

# Files that legitimately use otherwise-forbidden APIs
SKIP_FILES: set = set()

SOURCE_EXTENSIONS = {".cpp", ".h", ".hpp", ".cxx"}


def scan_file(filepath: pathlib.Path) -> list:
    """Scan a single file for contract violations."""
    violations = []
    try:
        text = filepath.read_text(errors="ignore")
    except OSError:
        return violations

    for pattern, reason in FORBIDDEN_PATTERNS:
        if pattern in text:
            violations.append(
                f"{filepath}: uses forbidden API `{pattern}` -- {reason}"
            )

    return violations


def main():
    parser = argparse.ArgumentParser(description="Subspace Engine Contract Scanner")
    parser.add_argument(
        "--path",
        default="engine",
        help="Root engine directory to scan (default: engine)",
    )
    args = parser.parse_args()

    root = pathlib.Path(args.path)
    if not root.exists():
        print(f"Error: directory {root} not found", file=sys.stderr)
        sys.exit(2)

    src_root = root / "src"
    if not src_root.exists():
        print(f"Error: source directory {src_root} not found", file=sys.stderr)
        sys.exit(2)

    all_violations = []

    # Scan deterministic simulation directories for forbidden APIs
    for sim_dir in SIMULATION_DIRS:
        scan_path = src_root / sim_dir
        if not scan_path.exists():
            continue
        for filepath in scan_path.rglob("*"):
            if filepath.suffix in SOURCE_EXTENSIONS:
                if filepath.name in SKIP_FILES:
                    continue
                all_violations.extend(scan_file(filepath))

    if all_violations:
        print(f"FAIL: {len(all_violations)} contract violation(s) found:\n")
        for v in all_violations:
            print(f"  {v}")
        sys.exit(1)
    else:
        print("PASS: No contract violations found.")
        sys.exit(0)


if __name__ == "__main__":
    main()
