#!/usr/bin/env python3
import importlib.util
import json
import tempfile
from pathlib import Path
base=Path(__file__).resolve().parents[1]/'null_harbor_rebrand_audit.py'
spec=importlib.util.spec_from_file_location('identity_audit',base)
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
with tempfile.TemporaryDirectory() as td:
    root=Path(td)
    (root/'project.control.json').write_text(json.dumps({'id':'codename-subspace','name':'Codename Subspace'}))
    (root/'README.md').write_text('# Codename Subspace\n')
    (root/'engine/include').mkdir(parents=True)
    (root/'engine/include/proto.h').write_text('namespace subspace {}\n')
    (root/'docs/handoffs').mkdir(parents=True)
    (root/'docs/handoffs/history.md').write_text('Codename Subspace used to be called that.\n')
    r=m.scan(root)
    categories={item['classification'] for item in r}
    assert 'active_review' in categories
    assert 'compatibility_migration_required' in categories
    assert 'historical_or_provenance_preserve' in categories
    assert m.main(['--root',str(root)])==0
    assert m.main(['--root',str(root),'--verify'])==1
print('NULL_HARBOR_REBRAND_AUDIT_TEST PASS')
