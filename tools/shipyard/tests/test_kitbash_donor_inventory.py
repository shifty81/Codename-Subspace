import importlib.util
import pathlib
import tempfile
import unittest
import zipfile

SRC = pathlib.Path(__file__).resolve().parents[2] / "blender" / "kitbash_donor_inventory.py"
spec = importlib.util.spec_from_file_location("kitbash_donor_inventory", SRC)
mod = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mod)


class DonorInventoryTests(unittest.TestCase):
    def test_only_safely_inventory_blender_donors(self):
        with tempfile.TemporaryDirectory() as temp:
            p = pathlib.Path(temp) / "donor.zip"
            with zipfile.ZipFile(p, "w") as z:
                z.writestr("SourceWork/Tools/Blender/NullharborKitbashGenerator/tool.py", "print('never execute me')")
                z.writestr("SourceWork/Tools/Blender/Archive/old.py", "old")
                z.writestr("SourceWork/Private/secret.py", "secret")
            data = mod.inventory(p)
            self.assertEqual(len(data["donors"]), 2)
            self.assertEqual(data["donors"][0]["category"], "archived-version-not-authority")
            self.assertEqual(data["donors"][1]["category"], "geometry-material-kitbash")

    def test_reject_path_traversal(self):
        with tempfile.TemporaryDirectory() as temp:
            p = pathlib.Path(temp) / "bad.zip"
            with zipfile.ZipFile(p, "w") as z:
                z.writestr("SourceWork/Tools/Blender/../../escape.py", "x")
            with self.assertRaisesRegex(ValueError, "unsafe"):
                mod.inventory(p)


if __name__ == "__main__":
    unittest.main()
