import importlib.util
from pathlib import Path
import unittest

HERE=Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('r32',HERE/'apply_studio_r23_r32_normalization.py')
r32=importlib.util.module_from_spec(spec);spec.loader.exec_module(r32)

class R32MigrationTests(unittest.TestCase):
    def test_generic_space_replacement(self):
        old='model.transformSpace==ShipyardTransformSpace::View?"CAMERA":(model.transformSpace==ShipyardTransformSpace::Ship?"SHIP":"LOCAL")'
        status='model_.transformSpace==ShipyardTransformSpace::View?\"TRANSFORM SPACE: CAMERA\":(model_.transformSpace==ShipyardTransformSpace::Ship?\"TRANSFORM SPACE: SHIP\":\"TRANSFORM SPACE: LOCAL\")'
        text='A '+old+' B '+old+' C '+status
        out,log=r32.extra_builder(text)
        self.assertEqual(out.count('"VIEW"'),2)
        self.assertIn('"PARENT"',out);self.assertIn('"OBJECT"',out)
        self.assertEqual(log[0]['state'],'APPLY')

    def test_workspace_developer_list(self):
        old='ShipyardWorkspaceSystem::DeveloperWorkspaces(){return {ShipyardWorkspaceMode::Model,ShipyardWorkspaceMode::Character,ShipyardWorkspaceMode::Pcg,ShipyardWorkspaceMode::World,ShipyardWorkspaceMode::DevWorld,ShipyardWorkspaceMode::ProjectTools,ShipyardWorkspaceMode::Authoring};}'
        help_old='"View-space movement is default; Ship/Local are explicit alternatives."'
        out,_=r32.extra_workspace(old+'\n'+help_old)
        self.assertNotIn('DeveloperWorkspaces(){return {ShipyardWorkspaceMode::Model,',out)
        self.assertIn('Parent/Object',out)

    def test_replace_once_is_fail_closed(self):
        with self.assertRaises(RuntimeError):
            r32.replace_once('x x','x','y','ambiguous')

if __name__=='__main__': unittest.main()
