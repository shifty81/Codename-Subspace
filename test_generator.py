"""Fixture-based generator tests; do not require or alter the real project."""
import importlib.util
import json
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch
from zipfile import ZipFile

SCRIPT = Path(__file__).with_name('Generate_PASS1507_DockFix.py')
spec = importlib.util.spec_from_file_location('pass1507_generator', SCRIPT)
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


def fixture():
    return ('#include "ui/SubspaceUiFramework.h"\n'
            'void LayoutNode(){\n' + module.LAYOUT_ANCHOR + '        work();\n    }\n'
            + module.CLOSE_ANCHOR + '\n')


class GeneratorTest(unittest.TestCase):
    def test_repairs_only_exact_anchors(self):
        source = fixture()
        result = module.patch_source(source)
        self.assertIn('if(panelId!=active)continue;', result)
        self.assertIn('node.activeTabId=candidate;', result)
        self.assertEqual(result.count('PASS1507: a leaf is a tab stack'), 1)
        self.assertNotEqual(result, source)
        with self.assertRaisesRegex(ValueError, 'absent or ambiguous'):
            module.patch_source(source.replace(module.LAYOUT_ANCHOR, ''))
        with self.assertRaisesRegex(ValueError, 'absent or ambiguous'):
            module.patch_source(source + module.CLOSE_ANCHOR)

    def setup_root(self, base):
        root = Path(base)
        (root / '.git').mkdir()
        source = root / module.DOCK_FILE
        source.parent.mkdir(parents=True)
        source.write_text(fixture(), encoding='utf-8')
        return root, source

    def fake_git(self, root, *args):
        if args[:2] == ('rev-parse', '--show-toplevel'):
            return str(root)
        if args[:2] == ('rev-parse', 'HEAD'):
            return module.BASE_COMMIT
        if args[:2] == ('status', '--porcelain'):
            return ''
        if args[:2] == ('hash-object', '--'):
            return module.BASE_DOCK_BLOB
        raise AssertionError(args)

    def test_package_integrity_and_no_source_mutation(self):
        with tempfile.TemporaryDirectory() as temp:
            root, source = self.setup_root(temp)
            original = source.read_bytes()
            with patch.object(module, 'git', side_effect=self.fake_git):
                generated = module.build(root)
                self.assertTrue(generated.is_file())
                self.assertEqual(source.read_bytes(), original)
                with ZipFile(generated) as package:
                    manifest = json.loads(package.read('PATCH_MANIFEST.json'))
                    self.assertEqual(manifest['baseline']['gitCommit'], module.BASE_COMMIT)
                    self.assertEqual(len(manifest['files']), 5)
                    self.assertEqual(manifest['sourcePreflight']['sha256'], module.sha256(original))
                    for f in manifest['files']:
                        data = package.read(f['path'])
                        self.assertEqual(module.sha256(data), f['sha256'])
                        self.assertEqual(len(data), f['bytes'])
                with self.assertRaisesRegex(ValueError, 'pending root patches'):
                    module.build(root)

    def test_baseline_guard_fails_without_changes(self):
        with tempfile.TemporaryDirectory() as temp:
            root, source = self.setup_root(temp)
            original = source.read_bytes()
            def bad_git(root, *args):
                if args == ('rev-parse', 'HEAD'):
                    return '0000000'
                return self.fake_git(root, *args)
            with patch.object(module, 'git', side_effect=bad_git):
                with self.assertRaisesRegex(ValueError, 'Baseline mismatch'):
                    module.build(root)
            self.assertEqual(source.read_bytes(), original)
            self.assertFalse((root / module.PATCH_NAME).exists())

    def test_rejects_drifted_source(self):
        with tempfile.TemporaryDirectory() as temp:
            root, source = self.setup_root(temp)
            original = source.read_bytes()
            def bad_git(root, *args):
                if args[:2] == ('hash-object', '--'):
                    return 'f' * 40
                return self.fake_git(root, *args)
            with patch.object(module, 'git', side_effect=bad_git):
                with self.assertRaisesRegex(ValueError, 'differs from certified baseline'):
                    module.build(root)
            self.assertEqual(source.read_bytes(), original)
            self.assertFalse((root / module.PATCH_NAME).exists())

if __name__ == '__main__':
    unittest.main(verbosity=2)
