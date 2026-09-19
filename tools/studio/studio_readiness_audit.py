#!/usr/bin/env python3
"""Read-only source/runtime/PCC and debug-bundle fingerprint for Subspace Studio.

The optional output is written outside source inputs. This does not certify
GREEN, alter a project, ingest third-party kits, or execute user content.
"""
from __future__ import annotations

import argparse
import datetime as dt
import hashlib
import json
import os
import re
import subprocess
import sys
import zipfile
from pathlib import Path
from typing import Any

SOURCE_PATHS = (
    "project.control.json",
    "SubspaceTools.ps1",
    "engine/CMakeLists.txt",
    "engine/src/studio/StudioApplication.cpp",
    "engine/include/studio/StudioApplication.h",
    "engine/include/studio/StudioClosePolicy.h",
    "engine/include/studio/StudioDocumentStore.h",
    "engine/src/studio/StudioNativeCloseGuard.cpp",
    "engine/src/ship_editor/ShipyardWorkspaceSystem.cpp",
    "engine/include/modeling/ShipyardModelingSystem.h",
)
BINARY_CANDIDATES = (
    "dist/subspace_studio.exe",
    "dist/bin/subspace_studio.exe",
    "build/Release/subspace_studio.exe",
    "build/Debug/subspace_studio.exe",
    "engine/build/Release/subspace_studio.exe",
    "engine/build/Debug/subspace_studio.exe",
)
SIGNALS = {
    "verified_blueprint_recovery": "Studio close: verified blueprint-only recovery",
    "unsupported_drafts_warning": "Studio exit WARNING: socket/definition overrides and editable model/interior drafts",
    "studio_exit_7": "subspace_studio exit code: 7",
    "pcc_exit_7_failure": "subspace_studio failed with exit code 7",
    "debug_packaging": "Packaging self-describing debug bundle",
}
MAX_DEBUG_MEMBER = 8 * 1024 * 1024
MAX_HASH_BYTES = 100 * 1024 * 1024


def sha256_file(path: Path, max_bytes: int = MAX_HASH_BYTES) -> dict[str, Any]:
    size = path.stat().st_size
    if size > max_bytes:
        return {"exists": True, "bytes": size, "sha256": None, "note": "size cap; hash skipped"}
    digest = hashlib.sha256()
    with path.open("rb") as src:
        for block in iter(lambda: src.read(1024 * 1024), b""):
            digest.update(block)
    return {"exists": True, "bytes": size, "sha256": digest.hexdigest()}


def git_value(root: Path, *args: str) -> str | None:
    try:
        result = subprocess.run(
            ["git", "-C", str(root), *args], stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL, text=True, encoding="utf-8",
            errors="replace", timeout=12, check=False,
        )
        return result.stdout.strip() if result.returncode == 0 else None
    except (OSError, subprocess.TimeoutExpired):
        return None


def repo_identity(root: Path) -> dict[str, Any]:
    head = git_value(root, "rev-parse", "HEAD")
    branch = git_value(root, "branch", "--show-current")
    status = git_value(root, "status", "--porcelain=v1", "--untracked-files=no")
    return {
        "repo_detected": head is not None,
        "head": head,
        "branch": branch,
        "tracked_changes": None if status is None else len(status.splitlines()),
        "working_tree_clean_tracked_only": status == "" if status is not None else None,
        "note": "Untracked paths intentionally excluded; this is not a certified clean-tree proof.",
    }


def project_contract(root: Path) -> dict[str, Any]:
    path = root / "project.control.json"
    if not path.is_file():
        return {"present": False, "error": "project.control.json missing"}
    try:
        data = json.loads(path.read_text(encoding="utf-8-sig"))
    except (OSError, UnicodeError, ValueError) as exc:
        return {"present": True, "error": type(exc).__name__}
    declared = data.get("project", {})
    if not isinstance(declared, dict):
        declared = {}
    keys = [entry.get("key") for entry in data.get("commands", [])
            if isinstance(entry, dict) and isinstance(entry.get("key"), str)]
    return {
        "present": True, "schema": data.get("schema"),
        "project_id": declared.get("id", data.get("id")),
        "command_count": len(keys),
        "studio_command_keys": sorted(k for k in keys if "studio" in k or "shipyard" in k),
        "hash": sha256_file(path)["sha256"],
    }


def pcc_state(root: Path) -> dict[str, Any]:
    """Observational receipts only. A recorded GREEN may not match this checkout."""
    gate_path = root / ".subspace/last-green-quality-gate.json"
    gate: dict[str, Any] = {"present": gate_path.is_file(), "current_green_verified": False}
    if gate_path.is_file():
        gate["file_sha256"] = sha256_file(gate_path)["sha256"]
        try:
            obj = json.loads(gate_path.read_text(encoding="utf-8-sig"))
            gate["result_recorded"] = obj.get("result")
            gate["gate_id"] = obj.get("gateId")
            gate["git_fingerprint_present"] = bool(obj.get("gitFingerprint"))
        except (OSError, UnicodeError, ValueError) as exc:
            gate["read_error"] = type(exc).__name__
    tx_dir = root / "updates/transactions"
    tx_files = sorted(tx_dir.glob("*.json"), key=lambda x: x.stat().st_mtime, reverse=True) if tx_dir.is_dir() else []
    patch: dict[str, Any] = {"transaction_receipts_count": len(tx_files), "latest": None}
    if tx_files:
        newest = tx_files[0]
        patch["latest"] = {"file_sha256": sha256_file(newest)["sha256"]}
        try:
            obj = json.loads(newest.read_text(encoding="utf-8-sig"))
            patch["latest"]["patch_id"] = obj.get("patchId")
            patch["latest"]["result_recorded"] = obj.get("result")
        except (OSError, UnicodeError, ValueError) as exc:
            patch["latest"]["read_error"] = type(exc).__name__
    return {
        "last_green_gate": gate,
        "patch_receipts": patch,
        "pending_root_patch_count": sum(1 for _ in root.glob("*.patch")),
        "note": "Receipts are historical; neither current GREEN nor patch applicability is asserted.",
    }


def source_signals(root: Path) -> dict[str, Any]:
    path = root / "engine/src/studio/StudioApplication.cpp"
    if not path.is_file():
        return {"available": False, "note": "StudioApplication.cpp unavailable; cannot classify source path."}
    try:
        content = path.read_text(encoding="utf-8-sig")
    except (OSError, UnicodeError):
        return {"available": False, "note": "StudioApplication.cpp could not be decoded."}
    return {
        "available": True,
        "contains_unconditional_unsupported_drafts_exit_7": bool(re.search(
            r"if\s*\(\s*StudioUnsavedWorkPolicy::HasUnsupportedRecovery\s*\(\s*unsaved\s*\)\s*\)"
            r"\s*\{[^}]*exitCode\s*=\s*7\s*;", content, flags=re.S)),
        "native_close_guard_is_installed": "closeGuard_.Install(" in content,
        "blueprint_recovery_path_present": "SaveExitRecovery(" in content,
        "note": "Textual indicators only, not a compiled or behavioral certification.",
    }


def evidence_from_zip(path: Path) -> dict[str, Any]:
    evidence: dict[str, Any] = {"present": True, "archive_name": path.name}
    evidence.update(sha256_file(path, max_bytes=512 * 1024 * 1024))
    try:
        with zipfile.ZipFile(path) as archive:
            unpacked_bytes = sum(i.file_size for i in archive.infolist())
            if unpacked_bytes <= 128 * 1024 * 1024 and len(archive.infolist()) <= 1000:
                corrupt_member = archive.testzip()
                evidence["zip_integrity"] = "PASS" if corrupt_member is None else "FAIL"
            else:
                corrupt_member = None
                evidence["zip_integrity"] = "NOT_FULLY_TESTED_SIZE_CAP"
            if corrupt_member:
                evidence["corrupt_member"] = corrupt_member
            names = archive.namelist()
            summaries = [n for n in names if n == "DEBUG_SUMMARY.txt"]
            session_logs = sorted(n for n in names
                                  if n.startswith("logs/sessions/") and n.lower().endswith(".log"))
            latest = session_logs[-1] if session_logs else None
            evidence["summary_present"] = bool(summaries)
            evidence["session_logs_count"] = len(session_logs)
            evidence["selected_session_log_name"] = latest
            text = ""
            for name in (summaries[0] if summaries else None, latest):
                if name is None:
                    continue
                info = archive.getinfo(name)
                if info.file_size > MAX_DEBUG_MEMBER:
                    evidence["log_skipped_size_cap"] = True
                    continue
                text += "\n" + archive.read(name).decode("utf-8-sig", "replace")
            evidence["signals"] = {key: token.lower() in text.lower()
                                   for key, token in SIGNALS.items()}
            # A modal choice is not present in these logs. Do not infer it.
            known = evidence["signals"]
            evidence["assessment"] = (
                "CONSISTENT_WITH_FALSE_FAILURE_AFTER_RECOVERY; CLOSE_CHOICE_NOT_LOGGED"
                if all(known[k] for k in ("verified_blueprint_recovery",
                                           "unsupported_drafts_warning",
                                           "studio_exit_7", "pcc_exit_7_failure"))
                else "INSUFFICIENT_EVIDENCE_FOR_KNOWN_EXIT_7_PATTERN"
            )
    except (OSError, zipfile.BadZipFile, RuntimeError, ValueError) as exc:
        evidence["zip_integrity"] = "FAIL"
        evidence["error"] = type(exc).__name__
    return evidence


def report(root: Path, binary: Path | None, debug_bundle: Path | None) -> dict[str, Any]:
    source = {}
    for relative in SOURCE_PATHS:
        path = root / relative
        source[relative] = sha256_file(path) if path.is_file() else {"exists": False}
    candidates = ([binary] if binary is not None else []) + [root / rel for rel in BINARY_CANDIDATES]
    seen = set()
    available = []
    for path in candidates:
        resolved = path.resolve()
        if str(resolved).lower() in seen or not resolved.is_file():
            continue
        seen.add(str(resolved).lower())
        result = {"name": resolved.name, "from_explicit_argument": binary is not None and resolved == binary.resolve()}
        result.update(sha256_file(resolved, max_bytes=512 * 1024 * 1024))
        available.append(result)
    debug = evidence_from_zip(debug_bundle) if debug_bundle and debug_bundle.is_file() else {
        "present": False, "note": "Optional debug bundle not supplied or not found."
    }
    return {
        "schema": "subspace.studio-readiness-audit.v1",
        "timestamp_utc": dt.datetime.now(dt.timezone.utc).isoformat(),
        "read_only_source": True,
        "certifies_green": False,
        "project": repo_identity(root),
        "contract": project_contract(root),
        "pcc_state": pcc_state(root),
        "source_files": source,
        "source_indicators": source_signals(root),
        "studio_executables": available,
        "studio_binary_verified_running": False,
        "debug_bundle": debug,
        "next_steps": [
            "Confirm local source HEAD, tracked modifications and patch receipts in PCC.",
            "Record running Studio executable path/hash from your actual Windows invocation.",
            "G02: fix close intent/exit semantics without suppressing genuine recovery failures.",
            "G03/G04: render, select, save and reopen a model-only BOX and complete editor drafts.",
        ],
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Subspace Studio read-only source/executable/exit baseline")
    parser.add_argument("--repo", type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument("--binary", type=Path, help="Actual Studio executable; improves identity evidence")
    parser.add_argument("--debug-bundle", type=Path, help="Existing Subspace_DebugBundle ZIP; never extracted")
    parser.add_argument("--out", type=Path, help="Report path (default is .subspace/reports/studio/)")
    parser.add_argument("--stdout", action="store_true", help="Print JSON without writing report")
    args = parser.parse_args(argv)
    root = args.repo.resolve()
    if not (root / "project.control.json").is_file():
        print("[ERROR] Expected project.control.json in repository root: " + str(root), file=sys.stderr)
        return 2
    data = report(root, args.binary, args.debug_bundle)
    if args.stdout:
        print(json.dumps(data, indent=2, ensure_ascii=False))
        return 0
    out = args.out or root / ".subspace/reports/studio" / "studio_readiness_latest.json"
    out = out.resolve()
    try:
        out.parent.mkdir(parents=True, exist_ok=True)
        tmp = out.with_name(out.name + ".tmp")
        tmp.write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
        os.replace(tmp, out)
    except (OSError, UnicodeError) as exc:
        print("[ERROR] Cannot write audit output: " + str(exc), file=sys.stderr)
        return 3
    print("[PASS] Read-only audit report: " + str(out))
    print("[INFO] Studio source condition: " +
          str(data["source_indicators"].get("contains_unconditional_unsupported_drafts_exit_7", "UNKNOWN")))
    print("[INFO] Debug evidence: " + data["debug_bundle"].get("assessment", "NOT_SUPPLIED"))
    print("[INFO] This report does not certify GREEN or repair source.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
