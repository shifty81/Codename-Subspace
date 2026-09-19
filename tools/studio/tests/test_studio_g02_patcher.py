#!/usr/bin/env python3
"""Source-anchor fixtures for G02 (not an actual Windows build)."""
import importlib.util
import pathlib
import tempfile
import unittest

MODULE=pathlib.Path(__file__).resolve().parents[1]/'apply_g02.py'
spec=importlib.util.spec_from_file_location('apply_g02',MODULE)
mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod)

class G02PatcherTests(unittest.TestCase):
    def test_exit_anchor(self):
        before="""    if(StudioUnsavedWorkPolicy::HasUnsupportedRecovery(unsaved)){
        std::cerr<<"Studio exit WARNING: socket/definition overrides and editable model/interior drafts are NOT in blueprint recovery\\n";
        exitCode=7;
    }
    closeGuard_.Detach();"""
        # Build a focused synthetic source that contains every exact required anchor.
        src="""#include "studio/StudioClosePolicy.h"
    if(!closeRecoveryPrepared_ && StudioRecoveryPathPolicy::NeedsRecovery(unsaved.blueprint)){
"""+before+"""
    if(!StudioClosePolicy::NeedsPrompt(before))return true;
        if(!StudioClosePolicy::MayCloseAfterSave(CloseState())){
            StudioFileDialog::ShowError("Studio remains open: the document has unsaved changes.");
            return false;
        }
        return true;
    return StudioClosePolicy::MayCloseWithRecovery(before,recovered,acknowledged);"""
        out=mod.edit_cpp(src)
        self.assertIn('StudioExitOutcomePolicy::ExitCode(',out)
        self.assertIn('closeOutcome_=StudioExitOutcome::Clean;',out)
        self.assertIn('closeOutcome_=StudioExitOutcome::Saved;',out)
        self.assertIn('closeOutcome_=StudioExitOutcome::UserConfirmedPartialRecovery;',out)
        self.assertNotIn('        exitCode=7;',out)
    def test_header_and_ctest(self):
        header="""#include "studio/StudioClosePolicy.h"
    bool closePromptActive_=false;
"""
        result=mod.edit_header(header)
        self.assertIn('StudioExitOutcome closeOutcome_',result)
        t='#include "studio/StudioClosePolicy.h"\n    std::cout<<"Studio close policy: "'
        self.assertIn('acknowledged partial recovery',mod.edit_test(t))
    def test_duplicate_anchor_fails_closed(self):
        with self.assertRaises(ValueError):mod.replace_once('AB AB','AB','CD','ambiguous')
    def test_git_blob_is_real_git_format(self):
        self.assertEqual(mod.git_blob(b'test\n'),'9daeafb9864cf43055ae93beb0afd6c7d144bfa4')
    def test_preimage_conflict_writes_nothing(self):
        with tempfile.TemporaryDirectory() as d:
            root=pathlib.Path(d)
            for rel in mod.PREIMAGES:
                p=root/rel;p.parent.mkdir(parents=True,exist_ok=True);p.write_text('modified',encoding='utf-8')
            header=root/'engine/include/studio/StudioExitOutcomePolicy.h'
            header.write_text('StudioExitOutcomePolicy',encoding='utf-8')
            with self.assertRaisesRegex(ValueError,'PREIMAGE_CONFLICT'):mod.stage(root)
            self.assertEqual((root/next(iter(mod.PREIMAGES))).read_text(),'modified')
if __name__=='__main__':unittest.main()
