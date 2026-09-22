import copy
import json
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
import exemplar_intake as r10
import exemplar_graph_compiler as compiler
import native_blueprint_intake as native
from test_native_blueprint_intake import fixture, CATALOG


def blender_pair(sha='a'*64):
    def node(i,m,cls,sem,x,y,p='',ps='',cs=''):
        return {'instanceId':i,'moduleId':m,'moduleClass':cls,'semantic':sem,'size':'S','anchor':'',
            'transform':{'position':[x,y,0], 'rotationEulerDeg':[0,0,0],
                         'scale':[1,1,1],'mirrorX':False},
            'connection':{'parentInstanceId':p,'parentSocket':ps,'childSocket':cs},'equipmentSlots':[]}
    obj={'schema':'subspace.shipyard_design','version':1,'coordinateSystem':r10.COORDINATES,
        'ship':{'name':'Frigate A','role':'MINING','seed':4},
        'modules':[node('hull','hull','hull','HULL_MID',0,0),
                   node('cmd','command','command','COMMAND_COCKPIT',0,5,'hull','front','aft'),
                   node('engine','engine','engine','MAIN_ENGINE',0,-5,'hull','back','front')]}
    return r10.compile_candidate(obj,sha,CATALOG,'d'*64)


class GraphCompilerTests(unittest.TestCase):
    def test_real_patterns_and_nonexecutable(self):
        out=compiler.compile_grammar([blender_pair()], 'frigate-mining')
        self.assertFalse(out['generatorEligible'])
        self.assertFalse(out['runtimeExecutable'])
        self.assertEqual(out['sampleCount'],1)
        self.assertEqual(len(out['semanticSocketPatterns']),2)
        self.assertEqual(out['observedSemanticCounts'][0]['meanPerExemplar'],1)
        self.assertIn('NOT',out['generationNote'])

    def test_same_role_two_exemplars_weighted(self):
        out=compiler.compile_grammar([blender_pair('a'*64),blender_pair('b'*64)],'family')
        self.assertEqual(out['sampleCount'],2)
        self.assertTrue(all(p['perExemplarFrequency']==1 for p in out['semanticSocketPatterns']))

    def test_reorder_input_independent(self):
        first,second=blender_pair('a'*64),blender_pair('b'*64)
        self.assertEqual(compiler.compile_grammar([first,second],'family'),
                         compiler.compile_grammar([second,first],'family'))

    def test_native_and_blender_both_accepted(self):
        native_pair=native.parse_native(fixture(),CATALOG,'d'*64)
        out=compiler.compile_grammar([blender_pair(),native_pair],'mining-family')
        self.assertEqual(out['sampleCount'],2)
        self.assertEqual({i['sourceSchema'] for i in out['sourceExemplars']},
                         {'SUBSPACE_SHIP_BLUEPRINT_V1','subspace.shipyard_design'})
        self.assertTrue(out['requirements']['persistentModuleInstanceIds'])

    def test_duplicate_source_not_double_weighted(self):
        with self.assertRaisesRegex(ValueError,'duplicate source'):
            compiler.compile_grammar([blender_pair(),blender_pair()],'family')

    def test_no_physical_blocker_rejected(self):
        a,b=blender_pair()
        b['issues']=[i for i in b['issues'] if i['severity']!='BLOCKER']
        b['blockerCount']=0
        with self.assertRaisesRegex(ValueError,'blocker'):
            compiler.compile_grammar([(a,b)],'family')

    def test_error_fails(self):
        a,b=blender_pair()
        b['errorCount']=1
        with self.assertRaisesRegex(ValueError,'structural errors'):
            compiler.compile_grammar([(a,b)],'family')

    def test_no_unsafe_promotion_flags(self):
        a,b=blender_pair()
        a['generatorEligible']=True
        with self.assertRaisesRegex(ValueError,'unpromoted'):
            compiler.compile_grammar([(a,b)],'family')

    def test_missing_edge_not_faked(self):
        a,b=blender_pair()
        a['attachmentEdges'].pop()
        with self.assertRaisesRegex(ValueError,'N-1'):
            compiler.compile_grammar([(a,b)],'family')

    def test_ghost_edge_rejected(self):
        a,b=blender_pair()
        a['attachmentEdges'][0]['parentModuleId']='other'
        with self.assertRaisesRegex(ValueError,'identity'):
            compiler.compile_grammar([(a,b)],'family')

    def test_mix_roles_rejected(self):
        a,b=blender_pair('a'*64),blender_pair('b'*64)
        b[0]['ship']['role']='COMBAT'
        with self.assertRaisesRegex(ValueError,'different roles'):
            compiler.compile_grammar([a,b],'family')

    def test_native_to_grammar_end_to_end_cli(self):
        with tempfile.TemporaryDirectory() as tmp:
            base=Path(tmp)
            src=base/'source'/'frigate.subspace_ship'
            src.parent.mkdir()
            original=fixture()
            src.write_bytes(original)
            catalog=base/'catalog'/'catalog.csv'
            catalog.parent.mkdir()
            catalog.write_text('module_id,grade,class,semantic,source_obj\n'
                               'hull,A,hull,HULL_MID,hull.obj\n'
                               'command,A,command,COMMAND_COCKPIT,command.obj\n'
                               'engine,A,engine,MAIN_ENGINE,engine.obj\n')
            staging=base/'staging'
            self.assertEqual(native.main(['--input',str(src),'--catalog',str(catalog),
                                          '--out-dir',str(staging)]),0)
            result=base/'grammar'/'result.json'
            self.assertEqual(compiler.main(['--exemplar-dir',str(staging),
                '--grammar-id','frigate-reference','--output',str(result)]),0)
            grammar=json.loads(result.read_text())
            self.assertEqual(grammar['sourceExemplars'][0]['sourceSha256'],
                             native.hashlib.sha256(original).hexdigest())
            self.assertEqual(len(grammar['semanticSocketPatterns']),2)
            self.assertFalse(grammar['generatorEligible'])
            self.assertEqual(src.read_bytes(),original)

    def test_no_candidate_overwrite_cli(self):
        with tempfile.TemporaryDirectory() as tmp:
            base=Path(tmp)
            ex=base/'input'
            ex.mkdir()
            a,b=blender_pair()
            (ex/'exemplar_candidate.json').write_text(json.dumps(a))
            (ex/'exemplar_audit.json').write_text(json.dumps(b))
            out=base/'result'/'grammar.json'
            cmd=['--exemplar-dir',str(ex),'--grammar-id','family','--output',str(out)]
            self.assertEqual(compiler.main(cmd),0)
            self.assertEqual(compiler.main(cmd),2)
            self.assertEqual(compiler.main(cmd+['--overwrite']),0)
            self.assertEqual(compiler.main(['--exemplar-dir',str(ex),'--grammar-id','family',
                                            '--output',str(ex/'grammar.json')]),2)
            self.assertEqual(json.loads(out.read_text())['state'],'PROPOSED_UNCERTIFIED')


if __name__=='__main__':
    unittest.main()
