from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parents[3]
TEXT=(ROOT/'SubspaceTools.ps1').read_text(encoding='utf-8')
class StudioBulkPolicyTests(unittest.TestCase):
    def test_helper_is_registered(self):
        self.assertIn('function Invoke-StudioBulkPolishNormalizationIfRequired',TEXT)
        self.assertIn('apply_studio_r33_r42_bulk_polish.py',TEXT)
    def test_render_build_runs_after_r32_and_before_continuity(self):
        start=TEXT.index('function Invoke-BuildRender')
        end=TEXT.index('function Invoke-TestsOnly',start)
        block=TEXT[start:end]
        self.assertLess(block.index('Studio Construct source cutover'),block.index('Studio bulk polish normalization'))
        self.assertLess(block.index('Studio bulk polish normalization'),block.index('Pass/source continuity audit'))
    def test_full_gate_runs_after_r32_and_before_continuity(self):
        start=TEXT.index('function Invoke-FullGate')
        block=TEXT[start:]
        self.assertLess(block.index('Studio Construct source cutover'),block.index('Studio bulk polish normalization'))
        self.assertLess(block.index('Studio bulk polish normalization'),block.index('Pass/source continuity audit'))
if __name__=='__main__': unittest.main()
