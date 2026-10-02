#!/usr/bin/env python3
from pathlib import Path
from tempfile import TemporaryDirectory
import importlib.util
import json

script=Path(__file__).resolve().parents[1]/'studio_r180_overlay_repair.py'
spec=importlib.util.spec_from_file_location('r186',script);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)

with TemporaryDirectory() as td:
    root=Path(td)
    # Current files represent governed descendants: their bytes are intentionally
    # not the original R182 after-images, but every semantic postcondition remains.
    rows=[]
    for rel,tokens in m.POSTCONDITIONS.items():
        p=root/rel;p.parent.mkdir(parents=True,exist_ok=True)
        p.write_text('// later governed descendant\n'+'\n'.join(tokens)+'\n',encoding='utf-8')
        rows.append({'path':rel,'beforeSha256':'1'*64,'afterSha256':'2'*64})
    for rel,tokens in m.EXTERNAL_AUTHORITIES.items():
        p=root/rel;p.parent.mkdir(parents=True,exist_ok=True)
        p.write_text('\n'.join(tokens)+'\n',encoding='utf-8')
    receipt=root/m.RECEIPT;receipt.parent.mkdir(parents=True,exist_ok=True)
    receipt.write_text(json.dumps({
        'schema':'subspace.r182-certified-baseline-recovery.v1',
        'baselineCommit':m.BASELINE_COMMIT,
        'strategy':'certified-8484a08-baseline-plus-r178-r179-forward-port',
        'files':rows,
    }),encoding='utf-8')

    assert m.receipt_matches(root), 'valid R182 receipt must authorize semantically valid later descendants'
    assert m.repair_known_overlay_regression(root,dry_run=True) is False, 'completed recovery must not replay after later governed edits'

    # Semantic regression after R182 still invalidates the descendant authority.
    victim=root/'engine/include/interior/ShipEmbodimentSystem.h'
    victim.write_text('InteriorAvatarStance\nplanarVelocity\nConfigureLocomotion\nStopLocomotion\n',encoding='utf-8')
    assert not m.receipt_matches(root), 'missing R182 postcondition must still fail closed'

    # Receipt provenance cannot be silently rebased to another commit.
    record=json.loads(receipt.read_text());record['baselineCommit']='deadbeef';receipt.write_text(json.dumps(record))
    assert not m.receipt_matches(root), 'wrong baseline receipt must not authorize descendants'

print('R186 recovery descendant compatibility tests PASS')
