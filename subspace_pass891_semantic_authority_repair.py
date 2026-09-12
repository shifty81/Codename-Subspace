#!/usr/bin/env python3
from __future__ import annotations

import argparse
import datetime as dt
import pathlib
import shutil
import subprocess


FUNCTION_REPAIRS = {
    pathlib.Path("engine/src/ships/ShipClassRoleSystem.cpp"): {
        "ShipClassRoleSystem::Envelope": r'''ShipClassEnvelope ShipClassRoleSystem::Envelope(ShipClass c){
    switch(c){
    case ShipClass::Frigate:return {c,UniversalSizeClass::XS,40.0f,90.0f,65.0f};
    case ShipClass::Destroyer:return {c,UniversalSizeClass::S,90.0f,160.0f,125.0f};
    case ShipClass::Cruiser:return {c,UniversalSizeClass::M,160.0f,280.0f,220.0f};
    case ShipClass::Battlecruiser:return {c,UniversalSizeClass::L,280.0f,450.0f,365.0f};
    case ShipClass::Battleship:return {c,UniversalSizeClass::XL,450.0f,750.0f,600.0f};
    case ShipClass::Capital:return {c,UniversalSizeClass::XL,750.0f,1800.0f,1050.0f};
    }
    return {};
}''',
    },
    pathlib.Path("engine/src/ships/ShipPcgRuntimeClosureSystem.cpp"): {
        "ShipPcgRuntimeClosureSystem::ApplyLineage": r'''void ShipPcgRuntimeClosureSystem::ApplyLineage(ProceduralShipVisualRecipe&recipe,const HullFamilyRuntimeProfile&family,ShipRole role,const std::string&exemplarId){
    recipe.factionId=family.factionId;
    recipe.shipClassId=ShipClassRoleSystem::ClassName(family.shipClass);
    recipe.hullFamilyId=family.familyId;
    recipe.roleVariantId=ShipClassRoleSystem::RoleName(role);
    recipe.exemplarId=exemplarId;
    recipe.lineageAuthority="FACTION_CLASS_HULL_ROLE_V1";
    recipe.manufacturerFamily=family.factionId.empty()?recipe.manufacturerFamily:family.factionId;
}''',
    },
    pathlib.Path("engine/src/ships/FactionShipDesignSystem.cpp"): {
        "FactionShipDesignSystem::VariantPreservesLineage": r'''bool FactionShipDesignSystem::VariantPreservesLineage(const ProceduralShipVisualRecipe&recipe,const HullFamilyCompiledGrammar&family,ShipRole role){
    if(recipe.factionId!=family.runtime.factionId)return false;
    if(recipe.hullFamilyId!=family.runtime.familyId)return false;
    if(recipe.shipClassId!=ShipClassRoleSystem::ClassName(family.runtime.shipClass))return false;
    if(recipe.roleVariantId!=ShipClassRoleSystem::RoleName(role))return false;
    if(!family.runtime.allowedRoles.empty()&&std::find(family.runtime.allowedRoles.begin(),family.runtime.allowedRoles.end(),role)==family.runtime.allowedRoles.end())return false;
    return recipe.lineageAuthority=="FACTION_CLASS_HULL_ROLE_V1";
}''',
    },
}


def locate_function(text: str, qualified_name: str) -> tuple[int, int]:
    start = text.find(qualified_name)
    if start < 0:
        raise RuntimeError(f"Function not found: {qualified_name}")
    line_start = text.rfind("\n", 0, start) + 1
    brace = text.find("{", start)
    if brace < 0:
        raise RuntimeError(f"Opening brace not found for: {qualified_name}")

    depth = 0
    i = brace
    in_string = in_char = in_line_comment = in_block_comment = False
    escape = False

    while i < len(text):
        ch = text[i]
        nx = text[i + 1] if i + 1 < len(text) else ""

        if in_line_comment:
            if ch == "\n":
                in_line_comment = False
            i += 1
            continue
        if in_block_comment:
            if ch == "*" and nx == "/":
                in_block_comment = False
                i += 2
                continue
            i += 1
            continue
        if in_string:
            if escape:
                escape = False
            elif ch == "\\":
                escape = True
            elif ch == '"':
                in_string = False
            i += 1
            continue
        if in_char:
            if escape:
                escape = False
            elif ch == "\\":
                escape = True
            elif ch == "'":
                in_char = False
            i += 1
            continue

        if ch == "/" and nx == "/":
            in_line_comment = True
            i += 2
            continue
        if ch == "/" and nx == "*":
            in_block_comment = True
            i += 2
            continue
        if ch == '"':
            in_string = True
            i += 1
            continue
        if ch == "'":
            in_char = True
            i += 1
            continue
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return line_start, i + 1
        i += 1

    raise RuntimeError(f"Closing brace not found for: {qualified_name}")


def replace_function(text: str, qualified_name: str, replacement: str) -> tuple[str, str]:
    start, end = locate_function(text, qualified_name)
    old = text[start:end]
    newline = "\r\n" if "\r\n" in text else "\n"
    replacement = replacement.replace("\n", newline)
    return text[:start] + replacement + text[end:], old


def run(cmd: list[str], cwd: pathlib.Path) -> int:
    print("\n[RUN]", " ".join(f'"{x}"' if " " in x else x for x in cmd))
    proc = subprocess.run(cmd, cwd=str(cwd))
    print("[EXIT]", proc.returncode)
    return proc.returncode


def find_powershell() -> str | None:
    for name in ("pwsh", "powershell"):
        p = shutil.which(name)
        if p:
            return p
    return None


def main() -> int:
    ap = argparse.ArgumentParser(description="Repair Pass613/Pass638 semantic ship authority regression.")
    ap.add_argument("--root", default=".", help="Codename Subspace repository root.")
    ap.add_argument("--no-full-gate", action="store_true", help="Stop after targeted tests pass.")
    args = ap.parse_args()

    root = pathlib.Path(args.root).resolve()
    required = [
        root / "engine" / "CMakeLists.txt",
        root / "SubspaceTools.ps1",
        root / "engine" / "tests" / "pass595_614_kitbash_construction_upgrade_tests.cpp",
        root / "engine" / "tests" / "pass615_654_kitbash_runtime_closure_tests.cpp",
    ]
    for p in required:
        if not p.is_file():
            print(f"[FAIL] Required repository file missing: {p}")
            return 2

    stamp = dt.datetime.now().strftime("%Y%m%d-%H%M%S")
    recovery = root / "artifacts" / "recovery" / f"pass891-semantic-authority-{stamp}"
    backup = recovery / "pre-repair"
    evidence = recovery / "evidence"
    backup.mkdir(parents=True, exist_ok=True)
    evidence.mkdir(parents=True, exist_ok=True)

    touched = []
    originals = {}

    print("=" * 78)
    print(" CODENAME SUBSPACE PASS891 SEMANTIC AUTHORITY REPAIR")
    print("=" * 78)
    print("Repository :", root)
    print("Recovery   :", recovery)
    print()
    print("Repairs:")
    print("  Pass613: canonical Frigate XS 40-90m / Battleship XL 450-750m")
    print("  Pass638: strict faction -> class -> hull-family -> role lineage round-trip")
    print()
    print("No tests are weakened and no unrelated Pass791-890 systems are reverted.")

    try:
        for rel, repairs in FUNCTION_REPAIRS.items():
            path = root / rel
            if not path.is_file():
                raise RuntimeError(f"Required source file missing: {rel}")
            data = path.read_bytes()
            originals[rel] = data

            target_backup = backup / rel
            target_backup.parent.mkdir(parents=True, exist_ok=True)
            target_backup.write_bytes(data)

            bom = data.startswith(b"\xef\xbb\xbf")
            text = data.decode("utf-8-sig")
            before_record = []
            for qualified_name, replacement in repairs.items():
                text, old = replace_function(text, qualified_name, replacement)
                before_record.append(f"===== BEFORE: {qualified_name} =====\n{old}\n")
                print(f"[PATCH] {rel} :: {qualified_name}")

            encoded = text.encode("utf-8")
            if bom:
                encoded = b"\xef\xbb\xbf" + encoded
            path.write_bytes(encoded)
            touched.append(rel)

            evidence_file = evidence / (rel.name + ".before.txt")
            evidence_file.write_text("\n".join(before_record), encoding="utf-8")

        build = root / "engine" / "build"
        if run([
            "cmake", "-S", str(root / "engine"), "-B", str(build),
            "-DSUBSPACE_HEADLESS=OFF",
            "-DSUBSPACE_BUILD_OPENGL=ON",
            "-DSUBSPACE_BUILD_TESTS=ON",
        ], root) != 0:
            raise RuntimeError("CMake configure failed after semantic repair.")

        if run(["cmake", "--build", str(build), "--config", "Debug", "--parallel", "8"], root) != 0:
            raise RuntimeError("Native build failed after semantic repair.")

        regex = "SubspacePass595To614KitbashConstructionUpgradeTests|SubspacePass615To654KitbashRuntimeClosureTests"
        if run([
            "ctest", "--test-dir", str(build), "-C", "Debug",
            "-R", regex, "--output-on-failure", "--timeout", "120",
        ], root) != 0:
            raise RuntimeError("Targeted Pass595-654 tests still fail after semantic repair.")

        (recovery / "PASS891_TARGETED_GREEN.txt").write_text(
            "PASS891 semantic authority repair\n"
            f"timestamp={dt.datetime.now().isoformat()}\n"
            "targeted_tests=PASS\n"
            "repaired=Pass613 canonical class envelopes; Pass638 lineage round-trip\n",
            encoding="utf-8",
        )
        print("\n[PASS] Targeted Pass595-654 tests are GREEN.")

        if args.no_full_gate:
            print("[INFO] --no-full-gate requested; stopping after targeted GREEN.")
            return 0

        ps = find_powershell()
        if not ps:
            print("[WARN] PowerShell not found. Targeted tests are GREEN; full gate not run.")
            return 0

        full_gate = run([
            ps, "-NoProfile", "-ExecutionPolicy", "Bypass",
            "-File", str(root / "SubspaceTools.ps1"),
            "-Action", "full-gate",
        ], root)
        if full_gate != 0:
            print("\n[WARN] Targeted repair is GREEN, but the full quality gate found another issue.")
            print("[WARN] Repaired source is retained. Use the new debug bundle for the next blocker.")
            return full_gate

        print("\n[PASS] PASS891 semantic authority repair is FULL-GATE GREEN.")
        return 0

    except Exception as exc:
        print(f"\n[FAIL] {exc}")
        print("[ROLLBACK] Restoring pre-repair source because targeted repair did not certify.")
        for rel in touched:
            data = originals.get(rel)
            if data is not None:
                (root / rel).write_bytes(data)
                print("[ROLLBACK]", rel)
        (recovery / "PASS891_REPAIR_FAILED.txt").write_text(
            f"timestamp={dt.datetime.now().isoformat()}\n"
            f"error={exc}\n"
            "rollback=completed\n",
            encoding="utf-8",
        )
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
