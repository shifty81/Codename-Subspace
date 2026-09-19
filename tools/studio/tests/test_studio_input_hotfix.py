"""Focused patcher checks; these are NOT Windows build or GUI acceptance."""
import importlib.util
from pathlib import Path
import tempfile
import unittest
from unittest import mock

SCRIPT=Path(__file__).resolve().parents[1]/'apply_g03_input.py'
spec=importlib.util.spec_from_file_location('studio_hotfix',SCRIPT)
h=importlib.util.module_from_spec(spec);spec.loader.exec_module(h)

STUDIO='''#include "studio/StudioGizmoMath.h"
    if(input_.WasPressed(InputAction::EditorToolSelect))RouteControl(ShipyardBuilderCommand::ToolSelect);
    if(input_.WasPressed(InputAction::EditorToolMove))RouteControl(ShipyardBuilderCommand::ToolMove);
    if(input_.WasPressed(InputAction::EditorToolRotate))RouteControl(ShipyardBuilderCommand::ToolRotate);
    if(scalePressed&&!window_.IsControlDown())RouteControl(ShipyardBuilderCommand::ToolScale);
    if(input_.WasPressed(InputAction::DccCommandSearch))RouteControl(ShipyardBuilderCommand::DccToggleCommandPalette);
    if(input_.WasPressed(InputAction::DccWorkspaceNext))RouteControl(ShipyardBuilderCommand::DccWorkspaceNext);
    if(input_.WasPressed(InputAction::DccWorkspacePrevious))RouteControl(ShipyardBuilderCommand::DccWorkspacePrevious);
    if(window_.ConsumePrimaryPress(pressX,pressY)){
        const auto layout=ShipyardBuilderSystem::Layout(model);
                if(picked>=0){
                    RouteControl(ShipyardBuilderCommand::SelectPlaced,picked);
                    if(builder_.Model().transformTool!=ShipyardTransformTool::Select){
                        pointerTransform_=builder_.Model().inspectorTab==ShipyardInspectorTab::Sockets?
                            builder_.BeginSelectedSocketTransform():builder_.BeginSelectedTransform();
                    }
                }
'''
NATIVE='''        case WM_KILLFOCUS:
            _inputState.Clear();
            _primaryButtonDown=false; _primaryPressPending=false; _primaryReleasePending=false; _primaryDragDeltaX=0.0f; _primaryDragDeltaY=0.0f; _cameraOrbitDragging=false; _cameraPanDragging=false; _altDown=false;
            return 0;
'''

class SourceGuardTests(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root=Path(self.tmp.name)
        (self.root/'.git').mkdir()
        (self.root/'engine/include/studio').mkdir(parents=True)
        (self.root/'engine/include/studio/StudioToolInteractionPolicy.h').write_text('policy')
        self.parts={
            'engine/src/studio/StudioApplication.cpp':STUDIO.encode(),
            'engine/src/platform/NativeWindow.cpp':NATIVE.encode(),
        }
        for rel,data in self.parts.items():
            p=self.root/rel;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(data)
        # Fixture preimages are derived from fixture bytes, unlike the actual
        # installer whose pinned GitHub blob SHAs are immutable and literal.
        p=mock.patch.dict(h.PREIMAGES,{k:h.git_blob(v) for k,v in self.parts.items()})
        p.start();self.addCleanup(p.stop)

    def test_read_only_check_and_complete_routing(self):
        pending,status=h.stage(self.root)
        self.assertEqual(len(pending),2)
        self.assertTrue(all((self.root/k).read_bytes()==v for k,v in self.parts.items()))
        changed=pending['engine/src/studio/StudioApplication.cpp'][1].decode()
        native=pending['engine/src/platform/NativeWindow.cpp'][1].decode()
        for symbol in ('DccToggleToolRail','DccToggleSidebar','DccToggleMaximizeViewport',
                       'TransformConstraintX','TransformConstraintY','TransformConstraintZ',
                       'NudgePort','NudgeDorsal','RemoveModule','FrameSelected',
                       'StudioToolInteractionPolicy::AllowsViewportReselection',
                       'selectedPlacedModule','suppressClick_=false;'):
            self.assertIn(symbol,changed)
        self.assertIn('_controlDown=false; _shiftDown=false;',native)
        self.assertIn('if(!typing)',changed)
        self.assertIn('if(!ctrl)',changed)

    def test_conflict_refuses_without_writes(self):
        target=self.root/'engine/src/platform/NativeWindow.cpp'
        target.write_bytes(NATIVE.encode()+b'// user modification\n')
        with self.assertRaisesRegex(ValueError,'PREIMAGE_CONFLICT'):
            h.stage(self.root)
        self.assertEqual((self.root/'engine/src/studio/StudioApplication.cpp').read_bytes(),STUDIO.encode())
        self.assertFalse((self.root/'.subspace').exists())

    def test_missing_anchor_refuses_without_writes(self):
        target=self.root/'engine/src/studio/StudioApplication.cpp'
        bad=STUDIO.replace('    if(input_.WasPressed(InputAction::EditorToolMove))','    if(false && input_.WasPressed(InputAction::EditorToolMove))')
        target.write_text(bad)
        with mock.patch.dict(h.PREIMAGES,{'engine/src/studio/StudioApplication.cpp':h.git_blob(bad.encode())}):
            with self.assertRaisesRegex(ValueError,'studio shortcut router'):
                h.stage(self.root)
        self.assertFalse((self.root/'.subspace').exists())

    def test_apply_receipt_backups_verify_and_refuse_reapply(self):
        pending,status=h.stage(self.root)
        receipt=h.apply(self.root,pending,status)
        self.assertEqual(__import__('json').loads(receipt.read_text())['status'],'APPLIED')
        for rel,(before,after) in pending.items():
            self.assertEqual((self.root/rel).read_bytes(),after)
            self.assertEqual((receipt.parent/rel).read_bytes(),before)
        with self.assertRaisesRegex(ValueError,'PREIMAGE_CONFLICT'):
            h.stage(self.root)

    def test_failure_rolls_back_both_files(self):
        pending,status=h.stage(self.root)
        replace=h.os.replace
        times=[0]
        def fail_second(*args):
            times[0]+=1
            if times[0]==2:raise OSError('injected second write error')
            return replace(*args)
        with mock.patch.object(h.os,'replace',side_effect=fail_second):
            with self.assertRaisesRegex(OSError,'injected'):
                h.apply(self.root,pending,status)
        for rel,before in self.parts.items():self.assertEqual((self.root/rel).read_bytes(),before)
        receipts=list((self.root/'.subspace/recovery/studio-interaction-hotfix').glob('*/RECEIPT.json'))
        self.assertEqual(len(receipts),1)
        self.assertEqual(__import__('json').loads(receipts[0].read_text())['status'],'ROLLED_BACK')

if __name__=='__main__':unittest.main()
