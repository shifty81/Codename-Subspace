from pathlib import Path
import re
import unittest

ROOT = Path(__file__).resolve().parents[3]
SCRIPT = ROOT / 'SubspaceTools.ps1'

class NativeLinkRetryPolicyTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.text = SCRIPT.read_text(encoding='utf-8')

    def test_retry_is_narrow_to_native_output_lock_codes(self):
        self.assertIn('function Test-NativeLinkOutputRaceInCurrentSession', self.text)
        self.assertRegex(self.text, r'LNK1104\|LNK1168')
        self.assertIn('subspace_(game|studio)\\.exe', self.text)

    def test_retry_is_bounded_and_rechecks_existing_safety_preflight(self):
        self.assertIn('$maxAttempts = 3', self.text)
        self.assertIn('Assert-NativeLinkOutputsReady -BuildDirectory $BuildDirectory', self.text)
        self.assertNotIn('Stop-Process -Name subspace_', self.text)
        self.assertNotIn('taskkill', self.text.lower())

    def test_main_build_routes_through_retry_wrapper(self):
        block = self.text[self.text.index('function Invoke-CMakeBuild {'):]
        self.assertIn('Invoke-CMakeBuildWithNativeLinkRetry -BuildDirectory $buildDir -ParallelJobs $parallelJobs', block)
        self.assertIn('Invoke-CTestWithoutCrashDialogs', block)

    def test_attempt_scoping_prevents_stale_link_error_reclassification(self):
        self.assertIn("$marker = 'RUN: ' + $BuildLabel", self.text)
        self.assertIn('$tail.LastIndexOf($marker', self.text)

if __name__ == '__main__':
    unittest.main()
