import csv
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("exemplar_intake", ROOT / "exemplar_intake.py")
mod = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mod)


def ship():
    def node(i, mid, cls, sem, x, y, parent="", ps="", cs=""):
        return {"instanceId": i, "moduleId": mid, "moduleClass": cls, "semantic": sem, "size": "S",
                "anchor": "", "transform": {"position": [x, y, 0], "rotationEulerDeg": [0, 0, 0],
                                             "scale": [1, 1, 1], "mirrorX": False},
                "connection": {"parentInstanceId": parent, "parentSocket": ps, "childSocket": cs},
                "equipmentSlots": []}
    return {"schema": "subspace.shipyard_design", "version": 1,
            "coordinateSystem": {"right": "+X", "forward": "+Y", "up": "+Z", "units": "meters"},
            "ship": {"name": "Reference Frigate", "role": "MINING", "seed": 4},
            "modules": [node("hull", "hull-a", "hull", "HULL_MID", 0, 0),
                        node("command", "cmd-a", "command", "COMMAND_COCKPIT", 0, 5, "hull", "front", "aft"),
                        node("engine", "drive-a", "engine", "MAIN_ENGINE", 0, -5, "hull", "aft", "front")]}


class ExemplarIntakeTests(unittest.TestCase):
    def compile(self, design=None, catalog=None, catalog_hash=None):
        return mod.compile_candidate(ship() if design is None else design, "a" * 64, catalog, catalog_hash)

    def test_valid_draft_never_promotes(self):
        candidate, audit = self.compile()
        self.assertEqual(len(candidate["nodes"]), 3)
        self.assertEqual(len(candidate["attachmentEdges"]), 2)
        self.assertEqual(candidate["rootInstanceIds"], ["hull"])
        self.assertFalse(candidate["generatorEligible"])
        self.assertFalse(audit["promotionAllowed"])
        self.assertEqual(audit["errorCount"], 0)
        self.assertEqual(audit["blockerCount"], 1)

    def test_determinism_independent_of_module_export_order(self):
        a = ship()
        b = ship()
        b["modules"].reverse()
        c1, d1 = self.compile(a)
        c2, d2 = self.compile(b)
        self.assertEqual(c1, c2)
        self.assertEqual(d1, d2)

    def test_duplicate_instance_rejected(self):
        a = ship()
        a["modules"][1]["instanceId"] = "hull"
        _, audit = self.compile(a)
        self.assertIn("DUPLICATE_INSTANCE_ID", [i["code"] for i in audit["issues"]])

    def test_parent_cycle_detected(self):
        a = ship()
        a["modules"][0]["connection"] = {"parentInstanceId": "engine", "parentSocket": "a", "childSocket": "b"}
        _, audit = self.compile(a)
        codes = [i["code"] for i in audit["issues"]]
        self.assertIn("PARENT_CYCLE", codes)
        self.assertIn("ROOT_COUNT", codes)

    def test_missing_parent_fails_closed(self):
        a = ship()
        a["modules"][1]["connection"]["parentInstanceId"] = "deleted"
        _, audit = self.compile(a)
        self.assertIn("MISSING_PARENT", [i["code"] for i in audit["issues"]])

    def test_unpaired_socket_rejected(self):
        a = ship()
        a["modules"][2]["connection"]["childSocket"] = ""
        _, audit = self.compile(a)
        self.assertIn("SOCKET_PAIR_MISSING", [i["code"] for i in audit["issues"]])

    def test_wrong_axes_rejected(self):
        a = ship()
        a["coordinateSystem"]["forward"] = "+Z"
        with self.assertRaisesRegex(ValueError, "coordinate"):
            self.compile(a)

    def test_nonfinite_transform_rejected(self):
        a = ship()
        a["modules"][0]["transform"]["position"][0] = float("nan")
        with self.assertRaisesRegex(ValueError, "nonfinite"):
            self.compile(a)

    def test_nonuniform_structure_rejected(self):
        a = ship()
        a["modules"][1]["transform"]["scale"] = [1, 2, 1]
        _, audit = self.compile(a)
        self.assertIn("NONUNIFORM_STRUCTURE", [i["code"] for i in audit["issues"]])

    def test_catalog_grade_and_semantic_mismatch(self):
        a = ship()
        catalog = {"hull-a": {"grade": "A", "class": "hull", "semantic": "HULL_MID"},
                   "cmd-a": {"grade": "B", "class": "command", "semantic": "COMMAND_COCKPIT"},
                   "drive-a": {"grade": "A", "class": "engine", "semantic": "ENGINE_HOUSING"}}
        _, audit = self.compile(a, catalog, "b"*64)
        self.assertEqual(audit["errorCount"], 2)
        codes = [i["code"] for i in audit["issues"]]
        self.assertIn("MODULE_NOT_GRADE_A", codes)
        self.assertIn("CATALOG_SEMANTIC_MISMATCH", codes)

    def test_catalog_csv_real_column_contract(self):
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / "catalog.csv"
            path.write_text("module_id,grade,class,semantic,source_obj\nhull-a,A,hull,HULL_MID,hull.obj\n", encoding="utf-8")
            catalog, sha = mod.read_catalog(path)
            self.assertEqual(catalog["hull-a"]["sourceObj"], "hull.obj")
            self.assertEqual(len(sha), 64)

    def test_json_duplicate_key_rejected(self):
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / "design.json"
            path.write_text('{"schema":"one","schema":"two"}', encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "duplicate"):
                mod.load_json(path)

    def test_cli_writes_draft_and_preserves_source(self):
        with tempfile.TemporaryDirectory() as temp:
            base = Path(temp)
            path = base / "source" / "frigate.subspace_shipyard.json"
            path.parent.mkdir()
            path.write_text(json.dumps(ship()), encoding="utf-8")
            orig = path.read_bytes()
            out = base / "staging"
            self.assertEqual(mod.main(["--input", str(path), "--out-dir", str(out)]), 0)
            self.assertEqual(path.read_bytes(), orig)
            candidate = json.loads((out / "exemplar_candidate.json").read_text())
            self.assertEqual(candidate["state"], "DRAFT_UNCERTIFIED")
            self.assertEqual(mod.main(["--input", str(path), "--out-dir", str(out)]), 2)
            self.assertEqual(mod.main(["--input", str(path), "--out-dir", str(out), "--overwrite"]), 0)

    def test_cli_rejects_source_directory_output(self):
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / "ship.json"
            path.write_text(json.dumps(ship()))
            self.assertEqual(mod.main(["--input", str(path), "--out-dir", str(path.parent)]), 2)


if __name__ == "__main__":
    unittest.main()
