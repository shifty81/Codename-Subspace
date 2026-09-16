"""Standard-library unit tests for PASS1506 read-only inventory (no native build required)."""
import importlib.util
import json
import tempfile
import unittest
from pathlib import Path

SCRIPT = Path(__file__).resolve().parents[1] / 'ShipyardInteractionAudit.py'
spec = importlib.util.spec_from_file_location('shipyard_interaction_audit', SCRIPT)
audit_mod = importlib.util.module_from_spec(spec)
spec.loader.exec_module(audit_mod)


class InventoryTests(unittest.TestCase):
    def make_repo(self, root):
        header = root / audit_mod.ENUM_HEADER
        src = root / 'engine/src/ship_editor/TestBuilder.cpp'
        header.parent.mkdir(parents=True)
        src.parent.mkdir(parents=True)
        header.write_text('''// enum class ShipyardBuilderCommand { Wrong, };
enum class ShipyardBuilderCommand {
  None, Add, Save = 5, Orphan,
};
''', encoding='utf-8')
        src.write_text('''// ShipyardBuilderCommand::Phantom
case ShipyardBuilderCommand::Add: changed = true; return true;
case ShipyardBuilderCommand::Save: model_.status="saved";return true;
auto a=ShipyardBuilderCommand::NotDeclared;
''', encoding='utf-8')
        return header, src

    def test_enum_and_comments(self):
        self.assertEqual(audit_mod.get_enum('enum class ShipyardBuilderCommand{ None, A=2, /* x */ B, };'),
                         ['None', 'A', 'B'])
        with self.assertRaises(ValueError):
            audit_mod.get_enum('enum class ShipyardBuilderCommand{ A, A };')

    def test_references_and_review_candidates(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            self.make_repo(root)
            report = audit_mod.audit(root)
            self.assertEqual(report['commandCount'], 4)
            self.assertEqual(report['summary']['no_source_reference'], 1)
            self.assertEqual(report['summary']['case_present_behavior_unverified'], 2)
            self.assertIn('NotDeclared', report['undeclaredReferences'])
            self.assertNotIn('Phantom', report['undeclaredReferences'])
            self.assertEqual([x['command'] for x in report['statusOnlyCandidates']], ['Save'])
            self.assertIn('Orphan', audit_mod.markdown(report))

    def test_cli_reports_only_and_strict(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            header, src = self.make_repo(root)
            h_before = header.read_bytes()
            s_before = src.read_bytes()
            dest = root / 'artifacts/reports/shipyard-interaction'
            self.assertEqual(audit_mod.main(['--root', str(root), '--out', str(dest)]), 0)
            self.assertEqual(audit_mod.main(['--root', str(root), '--out', str(dest), '--strict']), 2)
            self.assertEqual(header.read_bytes(), h_before)
            self.assertEqual(src.read_bytes(), s_before)
            parsed = json.loads((dest / 'interaction_audit.json').read_text(encoding='utf-8'))
            self.assertEqual(parsed['sourceFileCount'], 1)
            self.assertTrue((dest / 'interaction_audit.md').is_file())

    def test_missing_source_fails_closed(self):
        with tempfile.TemporaryDirectory() as temp:
            self.assertEqual(audit_mod.main(['--root', temp]), 1)


if __name__ == '__main__':
    unittest.main()
