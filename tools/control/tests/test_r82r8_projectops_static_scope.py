#!/usr/bin/env python3
"""R82R8: exercise the real ProjectOps CMake runner under include-scope ROOT mutation."""
from __future__ import annotations

import argparse
import shutil
import subprocess
import tempfile
from pathlib import Path


def run_cmake(cmake: str, runner: Path, engine: Path) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        [cmake, "-DROOT=" + engine.as_posix(), "-P", runner.as_posix()],
        text=True, capture_output=True, check=False
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--root", type=Path,
        default=Path(__file__).resolve().parents[3],
        help="Codename Subspace repository root",
    )
    args = parser.parse_args()
    source = args.root / "tools/control/ProjectOpsStaticCertification.cmake"
    if not source.is_file():
        print("[FAIL] Missing ProjectOpsStaticCertification.cmake:", source)
        return 1
    cmake = shutil.which("cmake")
    if not cmake:
        print("[FAIL] cmake unavailable")
        return 1

    with tempfile.TemporaryDirectory(prefix="subspace_r82r8_static_") as tmp:
        repo = Path(tmp) / "project"
        engine = repo / "engine"
        gates = repo / "tools/control/static-gates"
        engine.mkdir(parents=True)
        gates.mkdir(parents=True)
        runner = repo / "tools/control/ProjectOpsStaticCertification.cmake"
        shutil.copy2(source, runner)
        (repo / "project.control.json").write_text('{"id":"test","name":"test"}\n', encoding="utf-8")
        (engine / "engine-root.marker").write_text("engine root marker\n", encoding="utf-8")

        (gates / "pass_a_mutates_root.cmake").write_text(
            'get_filename_component(ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)\n'
            'set(PROJECT_ROOT "${ROOT}")\n'
            'message(STATUS "R82R8 synthetic earlier gate mutates both root variables")\n',
            encoding="utf-8",
        )
        (gates / "pass_b_requires_engine_root.cmake").write_text(
            'get_filename_component(PROJECT_ROOT "${ROOT}/.." ABSOLUTE)\n'
            'if(NOT EXISTS "${ROOT}/engine-root.marker")\n'
            '  message(FATAL_ERROR "R82R8 later gate inherited wrong ROOT")\n'
            'endif()\n'
            'if(NOT EXISTS "${PROJECT_ROOT}/project.control.json")\n'
            '  message(FATAL_ERROR "R82R8 later gate inherited wrong PROJECT_ROOT")\n'
            'endif()\n'
            'message(STATUS "R82R8 later gate resolved both roots correctly")\n',
            encoding="utf-8",
        )

        healthy = run_cmake(cmake, runner, engine)
        if healthy.returncode or "ProjectOps static certification PASS" not in healthy.stdout:
            print("[FAIL] Root isolation positive case")
            print(healthy.stdout, healthy.stderr)
            return 1
        print("[PASS] Earlier included gate cannot contaminate later ROOT/PROJECT_ROOT")

        (engine / "engine-root.marker").unlink()
        broken = run_cmake(cmake, runner, engine)
        if broken.returncode == 0 or "R82R8 later gate inherited wrong ROOT" not in broken.stderr:
            print("[FAIL] Real gate failures must propagate (not be swallowed)")
            print(broken.stdout, broken.stderr)
            return 1
        print("[PASS] Missing real prerequisite still fails closed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
