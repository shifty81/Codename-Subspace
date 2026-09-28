from pathlib import Path
import unittest

ROOT=Path(__file__).resolve().parents[3]
TEXT=(ROOT/'SubspaceTools.ps1').read_text(encoding='utf-8')
CONTINUITY=(ROOT/'scripts'/'subspace_pass_continuity_audit.ps1').read_text(encoding='utf-8')

class R32PrebuildMigrationPolicyTests(unittest.TestCase):
    def test_full_gate_runs_cutover_before_pass_continuity(self):
        full=TEXT.index('function Invoke-FullGate {')
        cut=TEXT.index('Studio Construct source cutover',full)
        continuity=TEXT.index('Pass/source continuity audit',full)
        build=TEXT.index('Render C++ configure/build/test',full)
        self.assertLess(cut,continuity)
        self.assertLess(cut,build)
    def test_cutover_is_idempotent_and_fail_closed(self):
        self.assertIn("Studio Construct source cutover already present; no migration required.",TEXT)
        self.assertIn("PASS1509 is installed but its required R32 source migration helper is missing.",TEXT)
        self.assertIn("returned success without establishing the required source cutover",TEXT)
    def test_render_build_is_protected(self):
        render=TEXT.index('function Invoke-BuildRender')
        tests=TEXT.index('function Invoke-TestsOnly',render)
        self.assertIn('Studio Construct source cutover',TEXT[render:tests])
    def test_same_process_full_gate_is_protected_by_disk_loaded_continuity_audit(self):
        self.assertIn('Pending approved Studio Construct source cutover detected',CONTINUITY)
        self.assertIn('apply_studio_r23_r32_normalization.py',CONTINUITY)
        self.assertIn('studio_r32_source_assertions.py',CONTINUITY)

if __name__=='__main__': unittest.main()
