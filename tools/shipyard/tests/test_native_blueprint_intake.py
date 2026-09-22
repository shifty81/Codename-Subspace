import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
import native_blueprint_intake as native

CATALOG = {
    'hull': {'grade': 'A', 'class': 'hull', 'semantic': 'HULL_MID'},
    'command': {'grade': 'A', 'class': 'command', 'semantic': 'COMMAND_COCKPIT'},
    'engine': {'grade': 'A', 'class': 'engine', 'semantic': 'MAIN_ENGINE'},
}


def fixture(extra='', child_gap='0', certified='1'):
    return ("SUBSPACE_SHIP_BLUEPRINT_V1\n"
            "META \"bp_1\" \"Example Frigate\" \"AUTHOR\" 1\n"
            "RECIPE \"recipe_1\" \"MINING\" 41 \"family\" \"yard\" \"decal\" 1 1\n"
            "ORIENTATION 0 \"COCKPIT\" 1\n"
            'MODULE "hull" 0 0 0 1 1 1 0 0 0 0 0 0 0 1\n'
            'MODULE "command" 0 5 0 1 1 1 0 0 0 0 0 0 0 1\n'
            'MODULE "engine" 0 -5 0 1 1 1 0 0 0 0 0 0 0 1\n'
            'ATTACH 0 1 "forward" "aft" {} {}\n'.format(child_gap, certified) +
            'ATTACH 0 2 "aft" "forward" 0 1\n' + extra).encode('utf-8')


class NativeIntakeTests(unittest.TestCase):
    def parse(self, raw=None, cat=None, sha=None):
        return native.parse_native(fixture() if raw is None else raw,
                                    CATALOG if cat is None else cat, 'd'*64 if sha is None else sha)

    def test_native_index_links_and_hash(self):
        candidate, audit = self.parse()
        root = candidate['rootInstanceIds'][0]
        self.assertTrue(root.startswith('native-' + native.hashlib.sha256(fixture()).hexdigest()[:12]))
        self.assertEqual([e['parentInstanceId'] for e in candidate['attachmentEdges']], [root, root])
        self.assertEqual(candidate['source']['schema'], native.MAGIC)
        self.assertEqual(candidate['source']['identityPolicy'], 'DOCUMENT_HASH_AND_ORDINAL_PROVISIONAL')
        self.assertEqual(audit['errorCount'], 0)
        self.assertFalse(audit['promotionAllowed'])

    def test_explicit_identity_blocker(self):
        candidate, audit = self.parse()
        codes = {i['code'] for i in audit['issues']}
        self.assertIn('NATIVE_INSTANCE_ID_NOT_SERIALIZED', codes)
        self.assertGreaterEqual(audit['blockerCount'], 2)
        self.assertFalse(candidate['generatorEligible'])

    def test_native_extra_mirror_axes_preserved_as_evidence(self):
        raw = fixture().replace(b'MODULE "engine" 0 -5 0 1 1 1 0 0 0 0 0 0 0 1',
                                b'MODULE "engine" 0 -5 0 1 1 1 15 30 45 0 1 1 0 0')
        candidate, _ = self.parse(raw)
        evidence = candidate['nativeModuleEvidence'][2]
        self.assertTrue(evidence['mirrorY'])
        self.assertFalse(evidence['mirrorZ'])
        self.assertEqual(evidence['yawPitchRollDegrees'], [15,30,45])
        self.assertFalse(evidence['sourceMaterialsEnabled'])

    def test_missing_module_indices_fail(self):
        raw = fixture().replace(b'ATTACH 0 1', b'ATTACH 22 1')
        with self.assertRaisesRegex(ValueError, 'missing module'):
            self.parse(raw)

    def test_multiple_parent_edges_fail(self):
        raw = fixture('ATTACH 1 2 "a" "b" 0 1\n')
        with self.assertRaisesRegex(ValueError, 'multiple parents'):
            self.parse(raw)

    def test_uncertified_attachment_flag_warns(self):
        _, audit = self.parse(fixture(certified='0'))
        self.assertIn('UNCERTIFIED_NATIVE_ATTACHMENT', {i['code'] for i in audit['issues']})

    def test_native_measured_gap_warns(self):
        _, audit = self.parse(fixture(child_gap='.25'))
        self.assertIn('MEASURED_ATTACHMENT_GAP', {i['code'] for i in audit['issues']})

    def test_malformed_or_extension_row_fails_closed(self):
        with self.assertRaisesRegex(ValueError, 'unsupported row'):
            self.parse(fixture('NEXT_VERSION_ROW some-data\n'))

    def test_invalid_schema_rejected(self):
        with self.assertRaisesRegex(ValueError, 'exact'):
            self.parse(fixture().replace(b'SUBSPACE_SHIP_BLUEPRINT_V1', b'SUBSPACE_SHIP_BLUEPRINT_V2'))

    def test_nonfinite_rejected(self):
        with self.assertRaisesRegex(ValueError, 'nonfinite'):
            self.parse(fixture().replace(b'MODULE "hull" 0 0 0', b'MODULE "hull" nan 0 0'))

    def test_roundtrip_bytes_not_changed_cli(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp)/'original'/'ship.subspace_ship'
            path.parent.mkdir()
            raw=fixture()
            path.write_bytes(raw)
            out = Path(tmp)/'staging'
            self.assertEqual(native.main(['--input',str(path),'--out-dir',str(out)]), 0)
            self.assertEqual(path.read_bytes(), raw)
            result=json.loads((out/'exemplar_candidate.json').read_text())
            self.assertEqual(result['source']['sha256'], native.hashlib.sha256(raw).hexdigest())
            self.assertEqual(native.main(['--input',str(path),'--out-dir',str(out)]), 2)

    def test_catalog_semantic_mismatch_fails_closed(self):
        bad = dict(CATALOG)
        bad['engine'] = {**CATALOG['engine'], 'grade':'B'}
        _, audit = self.parse(cat=bad)
        self.assertIn('MODULE_NOT_GRADE_A', {i['code'] for i in audit['issues']})


if __name__ == '__main__':
    unittest.main()
