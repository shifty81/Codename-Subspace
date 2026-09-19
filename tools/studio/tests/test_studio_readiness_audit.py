"""Portable contract tests for G01: python -m unittest discover -s tools/studio/tests."""
import json
import sys
import tempfile
import unittest
import zipfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import studio_readiness_audit as audit


class ReadinessAuditTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        (self.root / "project.control.json").write_text(json.dumps({
            "schema": "forge.project.v1", "project": {"id": "codename-subspace"},
            "commands": [{"key": "run-studio"}, {"key": "build.full"}]
        }), encoding="utf-8")
        source = self.root / "engine/src/studio/StudioApplication.cpp"
        source.parent.mkdir(parents=True)
        source.write_text("if(StudioUnsavedWorkPolicy::HasUnsupportedRecovery(unsaved)){\n"
                          "  exitCode=7;\n}\ncloseGuard_.Install(...); SaveExitRecovery(...);",
                          encoding="utf-8")

    def test_repo_contract(self):
        result = audit.project_contract(self.root)
        self.assertEqual(result["project_id"], "codename-subspace")
        self.assertEqual(result["studio_command_keys"], ["run-studio"])

    def test_pcc_gate_receipt_does_not_assert_current_green(self):
        gate = self.root / ".subspace/last-green-quality-gate.json"
        gate.parent.mkdir(parents=True)
        gate.write_text(json.dumps({"result": "PASS", "gateId": "QG-OLD", "gitFingerprint": "OLD"}),
                        encoding="utf-8")
        tx = self.root / "updates/transactions"
        tx.mkdir(parents=True)
        (tx / "receipt.json").write_text(json.dumps({"patchId": "P1", "result": "PASS"}), encoding="utf-8")
        result = audit.pcc_state(self.root)
        self.assertEqual(result["last_green_gate"]["result_recorded"], "PASS")
        self.assertFalse(result["last_green_gate"]["current_green_verified"])
        self.assertEqual(result["patch_receipts"]["latest"]["patch_id"], "P1")

    def test_source_indicators_are_not_certification(self):
        result = audit.source_signals(self.root)
        self.assertTrue(result["contains_unconditional_unsupported_drafts_exit_7"])
        self.assertTrue(result["native_close_guard_is_installed"])
        self.assertIn("not a compiled", result["note"])

    def test_debug_zip_known_pattern_does_not_infer_click(self):
        archive = self.root / "bundle.zip"
        with zipfile.ZipFile(archive, "w") as out:
            out.writestr("DEBUG_SUMMARY.txt", "Current operation: run-studio")
            out.writestr("logs/sessions/last.log", "\n".join(audit.SIGNALS.values()))
        result = audit.evidence_from_zip(archive)
        self.assertEqual(result["zip_integrity"], "PASS")
        self.assertIn("CLOSE_CHOICE_NOT_LOGGED", result["assessment"])
        self.assertNotIn("click", result)

    def test_incomplete_evidence_is_not_false_failure_confirmation(self):
        archive = self.root / "bundle.zip"
        with zipfile.ZipFile(archive, "w") as out:
            out.writestr("DEBUG_SUMMARY.txt", "crash from unrelated source")
        result = audit.evidence_from_zip(archive)
        self.assertEqual(result["assessment"], "INSUFFICIENT_EVIDENCE_FOR_KNOWN_EXIT_7_PATTERN")

    def test_stdout_does_not_mutate_project(self):
        known = {p.relative_to(self.root) for p in self.root.rglob("*") if p.is_file()}
        self.assertEqual(audit.main(["--repo", str(self.root), "--stdout"]), 0)
        self.assertEqual(known, {p.relative_to(self.root) for p in self.root.rglob("*") if p.is_file()})

    def test_written_report_is_not_green(self):
        out = self.root / ".subspace/reports/studio/audit.json"
        self.assertEqual(audit.main(["--repo", str(self.root), "--out", str(out)]), 0)
        obj = json.loads(out.read_text(encoding="utf-8"))
        self.assertFalse(obj["certifies_green"])
        self.assertFalse(obj["studio_binary_verified_running"])

    def test_missing_repo_fails_without_artifacts(self):
        empty = self.root / "empty"
        empty.mkdir()
        self.assertEqual(audit.main(["--repo", str(empty)]), 2)
        self.assertFalse((empty / ".subspace").exists())

    def test_bad_zip_is_reported_without_extracting(self):
        broken = self.root / "broken.zip"
        broken.write_bytes(b"corrupt ZIP")
        result = audit.evidence_from_zip(broken)
        self.assertEqual(result["zip_integrity"], "FAIL")


if __name__ == "__main__":
    unittest.main()
