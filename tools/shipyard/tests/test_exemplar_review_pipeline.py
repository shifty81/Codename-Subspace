import json
from pathlib import Path
import tempfile
import unittest
import sys

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT))
import exemplar_review_pipeline as pipe


def fixture():
    return ("SUBSPACE_SHIP_BLUEPRINT_V1\n"
            "META \"bp_1\" \"Example Frigate\" \"AUTHOR\" 1\n"
            "RECIPE \"recipe_1\" \"MINING\" 41 \"family\" \"yard\" \"decal\" 1 1\n"
            "ORIENTATION 0 \"COCKPIT\" 1\n"
            'MODULE "hull" 0 0 0 1 1 1 0 0 0 0 0 0 0 1\n'
            'MODULE "command" 0 5 0 1 1 1 0 0 0 0 0 0 0 1\n'
            'MODULE "engine" 0 -5 0 1 1 1 0 0 0 0 0 0 0 1\n'
            'ATTACH 0 1 "forward" "aft" 0 1\n'
            'ATTACH 0 2 "aft" "forward" 0 1\n').encode()

class PipelineTests(unittest.TestCase):
    def test_bundle_is_review_only_and_source_unchanged(self):
        with tempfile.TemporaryDirectory() as td:
            root=Path(td);src=root/'source'/'ship.subspace_ship';src.parent.mkdir();raw=fixture();src.write_bytes(raw)
            out=root/'review'
            self.assertEqual(pipe.main(['--ship',str(src),'--out-dir',str(out),'--grammar-id','frigate.family.v1']),0)
            self.assertEqual(src.read_bytes(),raw)
            m=json.loads((out/'review_manifest.json').read_text())
            self.assertFalse(m['promotionAllowed']);self.assertFalse(m['runtimeInstallAllowed'])
            self.assertEqual(len(m['artifacts']),3)
            self.assertTrue((out/'grammar'/'family_grammar.json').is_file())

    def test_existing_output_requires_overwrite(self):
        with tempfile.TemporaryDirectory() as td:
            root=Path(td);src=root/'source'/'ship.subspace_ship';src.parent.mkdir();src.write_bytes(fixture())
            out=root/'review';out.mkdir()
            self.assertEqual(pipe.main(['--ship',str(src),'--out-dir',str(out),'--grammar-id','x']),2)

if __name__=='__main__': unittest.main()
