import importlib.util, pathlib, tempfile, unittest

HERE=pathlib.Path(__file__).resolve()
MOD=HERE.parent/'apply_studio_r13_r22_normalization.py'
spec=importlib.util.spec_from_file_location('migrate',MOD); migrate=importlib.util.module_from_spec(spec);spec.loader.exec_module(migrate)

class MigrationUtilityTests(unittest.TestCase):
    def test_replace_once_applies_once(self):
        text,state=migrate.replace_once('alpha OLD omega','OLD','NEW','unit')
        self.assertEqual(text,'alpha NEW omega');self.assertEqual(state,'APPLY')
    def test_replace_once_is_idempotent(self):
        text,state=migrate.replace_once('alpha NEW omega','OLD','NEW','unit')
        self.assertEqual(text,'alpha NEW omega');self.assertEqual(state,'PRESENT')
    def test_replace_once_fails_on_ambiguity(self):
        with self.assertRaises(RuntimeError):migrate.replace_once('OLD OLD','OLD','NEW','unit')
        with self.assertRaises(RuntimeError):migrate.replace_once('neither','OLD','NEW','unit')

if __name__=='__main__':unittest.main()
