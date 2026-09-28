
#!/usr/bin/env python3
"""R82R9: validate both historical gate repairs independently and as one runner.

Builds a synthetic source tree; never mutates the real project.
"""
from __future__ import annotations

import argparse
import re
import shutil
import subprocess
import tempfile
from collections import defaultdict
from pathlib import Path


GATE_NAMES = (
    "shipyard_overlay_canvas_foundation.cmake",
    "studio_foundation_r3_authority.cmake",
)


def run(cmake: str, repo: Path, gate: str | None = None) -> subprocess.CompletedProcess[str]:
    source = repo / "tools/control/ProjectOpsStaticCertification.cmake"
    if gate is not None:
        source = repo / "tools/control/static-gates" / gate
    return subprocess.run(
        [cmake, "-DROOT=" + (repo / "engine").as_posix(), "-P", source.as_posix()],
        text=True, capture_output=True, check=False
    )


def fixture(source: Path, dest: Path) -> None:
    gates = dest / "tools/control/static-gates"
    gates.mkdir(parents=True)
    (dest / "engine").mkdir(parents=True)
    runner = source / "tools/control/ProjectOpsStaticCertification.cmake"
    if not runner.is_file():
        raise RuntimeError("R82R8 root-isolated ProjectOps runner is missing")
    shutil.copy2(runner, dest / "tools/control/ProjectOpsStaticCertification.cmake")
    for name in GATE_NAMES:
        shutil.copy2(source / "tools/control/static-gates" / name, gates / name)

    # Source content representative of the actual current contracts, including
    # the old unchanged overlay/pointer/persistence requirements.
    snippets: dict[str, list[str]] = defaultdict(list)
    snippets["engine/src/ui/SubspaceUiFramework.cpp"] += [
        "LayoutShipyardOverlays(w,width,height,topInset,out)",
        "if(active.empty())continue; // empty anchor does not leave a viewport hole",
        'if(id=="center")return {0,topInset,x,available}',
    ]
    snippets["engine/src/ship_editor/ShipyardWorkspaceSystem.cpp"].append(
        'add("tool_rail","Tools","tool_left",true,1.0f,48,420,false,false,false)'
    )
    snippets["engine/include/ship_editor/ShipyardDockPointerSystem.h"] += [
        "ShipyardPanelCompositorSystem::Snapshot(w,width,height,topInset)",
        '(panel=="tool_rail"&&target!="tool_left")',
    ]
    snippets["engine/src/application/NativeGameApplication.cpp"] += [
        "ShipyardOverlayLayoutStore::Load(_shipBuilder.MutableDockWorkspace()",
        "ShipyardOverlayLayoutStore::Save(_shipBuilder.Model().dockWorkspace",
    ]
    snippets["engine/CMakeLists.txt"].append("SubspaceShipyardOverlayFoundationTests")
    # Every source path/exact marker originally certified by the R3/R4 gate
    # still needs to exist; the only substituted check is the obsolete title.
    r3_gate = (gates / GATE_NAMES[1]).read_text(encoding="utf-8")
    for path, marker in re.findall(r'required\("([^"]+)" "([^"]+)"\)', r3_gate):
        snippets[path].append(marker)
    snippets["engine/src/studio/StudioApplication.cpp"] += [
        'NativeWindowConfig config; config.title="Subspace Studio - Ship Authoring";',
        "window_.Initialize(config)",
    ]
    for relative, values in snippets.items():
        dest_path = dest / relative
        dest_path.parent.mkdir(parents=True, exist_ok=True)
        dest_path.write_text("\n".join(values) + "\n", encoding="utf-8")
    for relative in (
        "engine/include/ship_editor/ShipyardOverlayLayoutStore.h",
        "engine/src/ship_editor/ShipyardProfessionalVisibleCutover.cpp",
        "engine/tests/shipyard_overlay_foundation_tests.cpp",
    ):
        target = dest / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text("// synthetic prerequisite\n", encoding="utf-8")


def expect(cmake: str, repo: Path, label: str, succeed: bool, error: str = "",
           gate: str | None = None) -> None:
    result = run(cmake, repo, gate)
    if (result.returncode == 0) != succeed or (error and error not in result.stderr):
        raise AssertionError(
            f"{label}: expected success={succeed}, error={error!r}, "
            f"got code={result.returncode}\n{result.stdout}\n{result.stderr}"
        )
    print(f"[PASS] {label}")


def change(path: Path, old: str, new: str) -> None:
    text = path.read_text(encoding="utf-8")
    if text.count(old) != 1:
        raise AssertionError(f"fixture mutation no longer exact: {path}: {old}")
    path.write_text(text.replace(old, new), encoding="utf-8")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[3])
    args = ap.parse_args()
    source = args.root.resolve()
    cmake = shutil.which("cmake")
    if cmake is None:
        raise RuntimeError("cmake executable is required")
    with tempfile.TemporaryDirectory(prefix="subspace_r82r9_") as t:
        repo = Path(t) / "fixture"
        fixture(source, repo)
        expect(cmake, repo, "Both repaired gates pass together under the R82R8 runner", True)
        expect(cmake, repo, "Overlay retains original layout, pointer, persistence and CTest checks", True,
               gate=GATE_NAMES[0])
        expect(cmake, repo, "R3 retains original R3R2/R4 source authority", True, gate=GATE_NAMES[1])

        workspace = repo / "engine/src/ship_editor/ShipyardWorkspaceSystem.cpp"
        fixed = 'add("tool_rail","Tools","tool_left",true,1.0f,48,420,false,false,false)'
        old = 'add("tool_rail","Tools","tool_left",true,1.0f,48,420,true,true,true)'
        change(workspace, fixed, old)
        expect(cmake, repo, "Retired movable tool rail rejected", False,
               "fixed/non-floatable PASS1509 contract", GATE_NAMES[0])
        change(workspace, old, fixed)

        pointer = repo / "engine/include/ship_editor/ShipyardDockPointerSystem.h"
        change(pointer, "ShipyardPanelCompositorSystem::Snapshot", "REMOVED_SNAPSHOT")
        expect(cmake, repo, "Missing actual overlay pointer authority rejected", False,
               "Dock pointer overlay snapshot/rail authority missing", GATE_NAMES[0])
        change(pointer, "REMOVED_SNAPSHOT", "ShipyardPanelCompositorSystem::Snapshot")

        app = repo / "engine/src/studio/StudioApplication.cpp"
        change(app, '"Subspace Studio - Ship Authoring"', '"Null Harbor Studio - Ship Authoring"')
        expect(cmake, repo, "Superseded Null Harbor title rejected", False,
               "retains superseded Null Harbor title", GATE_NAMES[1])
        change(app, '"Null Harbor Studio - Ship Authoring"', '"Subspace Studio - Ship Authoring"')

        change(app, '"Subspace Studio - Ship Authoring"', '""')
        expect(cmake, repo, "Empty Studio title rejected", False,
               "requires a nonempty Studio title assignment", GATE_NAMES[1])
        change(app, '""', '"Subspace Studio - Ship Authoring"')

        change(app, "StudioGizmoProjectionPolicy::ReflowForOcclusion", "REMOVED_REFLOW")
        expect(cmake, repo, "Missing real R4 gizmo reflow rejected", False,
               "Null Harbor R3 Studio foundation authority missing", GATE_NAMES[1])
        change(app, "REMOVED_REFLOW", "StudioGizmoProjectionPolicy::ReflowForOcclusion")

        expect(cmake, repo, "Combined runner still passes after negative-control restoration", True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
