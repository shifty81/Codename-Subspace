import importlib.util, json, pathlib, tempfile, unittest

HERE=pathlib.Path(__file__).resolve()
MOD=HERE.parents[1]/'subspace_kitbash_foundry.py'
spec=importlib.util.spec_from_file_location('foundry',MOD); foundry=importlib.util.module_from_spec(spec);spec.loader.exec_module(foundry)

class FoundryTests(unittest.TestCase):
    def recipe(self):
        return {"schema":foundry.RECIPE_SCHEMA,"assetId":"test.frigate.hull",
                "parts":[{"id":"core","primitive":"HULL_SEGMENT","size":[4,10,3]},
                         {"id":"wing","primitive":"WEDGE","size":[8,4,.5],"position":[3,0,0],"role":"WING"}],
                "sockets":[{"id":"wing.root","partId":"wing","position":[-4,0,0],"forward":[-1,0,0],"up":[0,0,1]}]}
    def test_recipe_is_deterministic_and_forward_contract_is_emitted(self):
        a=foundry.validate_recipe(self.recipe());b=foundry.validate_recipe(self.recipe())
        self.assertEqual(a['recipeSha256'],b['recipeSha256'])
        worker=foundry.emit_worker(self.recipe())
        self.assertIn("SUBSPACE_KITBASH_FOUNDRY_OK",worker)
        self.assertIn("(-x,y,0)",worker) # wedge tip is +Y-forward
        self.assertNotIn("import Nullharbor",worker)
    def test_unknown_primitive_fails_closed(self):
        r=self.recipe();r['parts'][0]['primitive']='MAGIC'
        with self.assertRaises(ValueError):foundry.validate_recipe(r)
    def test_bad_socket_owner_fails_closed(self):
        r=self.recipe();r['sockets'][0]['partId']='missing'
        with self.assertRaises(ValueError):foundry.validate_recipe(r)
    def test_inventory_never_executes_donor(self):
        inv={'donors':[{'sourcePath':'SourceWork/Tools/Blender/NullharborShipFoundry/generator_core.py','sha256':'a'*64,'status':'REVIEW_BEFORE_PORT','category':'ship-foundry'},
                       {'sourcePath':'SourceWork/Tools/Blender/NullharborKitbashGenerator/material_export.py','sha256':'b'*64}]}
        reg=foundry.capability_registry(inv)
        self.assertEqual(reg['authority'],'READ_ONLY_DONOR_EVIDENCE')
        self.assertEqual(reg['executionPolicy'],'DONOR_CODE_NEVER_EXECUTED_DIRECTLY')
        self.assertIn('SHIP_GENERATION',reg['capabilities'])
        self.assertIn('KITBASH_GEOMETRY',reg['capabilities'])
        self.assertIn('MATERIALS',reg['capabilities'])
        self.assertIn('EXPORT',reg['capabilities'])

if __name__=='__main__':unittest.main()
