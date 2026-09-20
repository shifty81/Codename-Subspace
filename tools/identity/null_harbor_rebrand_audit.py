#!/usr/bin/env python3
"""Read-only Null Harbor identity inventory; never rewrites project/source/save files."""
import argparse
import json
import re
import sys
from collections import Counter
from pathlib import Path

PATTERNS = {
    'codename_subspace': re.compile(r'codename[ _-]+subspace', re.I),
    'subspace': re.compile(r'subspace', re.I),
    'null_harbor': re.compile(r'null[ _-]?harbor', re.I),
    'nova_forge': re.compile(r'nova[ _-]?forge', re.I),
}
SKIP_PARTS = {'.git', '.subspace', 'build', 'Build', 'artifacts', 'node_modules',
              '__pycache__', '.venv', 'venv', 'dist', 'recovery'}
TEXT_SUFFIX = {'.h', '.hpp', '.c', '.cpp', '.cc', '.py', '.ps1', '.cmd', '.bat',
               '.json', '.jsonl', '.md', '.txt', '.xml', '.yml', '.yaml', '.toml',
               '.cmake', '.in', '.csv', '.glsl', '.vert', '.frag', '.gitignore',
               '.gitattributes', '.editorconfig', '.html', '.ini', '.cfg'}

def classify(path, line):
    low = path.lower()
    text = line.lower()
    if any(x in low for x in ('docs/handoffs/', 'docs/audits/', 'changelog', 'source_rollup',
                              'license', 'credits', 'third_party')):
        return 'historical_or_provenance_preserve'
    if 'github.com/shifty81/codename-subspace' in text:
        return 'remote_change_after_github_rename'
    if any(t in text for t in ('schema', 'namespace subspace', 'subspace::', 'projectid',
                                'project_id', 'project.id', 'patchid', 'patch_id',
                                'stateDirectory'.lower(), '.subspace', 'assetid', 'asset_id')):
        return 'compatibility_migration_required'
    if low.startswith(('readme', 'docs/', 'engine/src/', 'engine/include/', 'tools/',
                       'scripts/', 'content/')):
        return 'active_review'
    return 'unclassified_review'

def scan(root):
    findings=[]
    for p in sorted(root.rglob('*')):
        if p.is_symlink() or not p.is_file():continue
        rel=p.relative_to(root).as_posix()
        if set(p.relative_to(root).parts)&SKIP_PARTS:continue
        if p.suffix.lower() not in TEXT_SUFFIX and p.name not in {'CMakeLists.txt', 'LICENSE'}:continue
        try:
            if p.stat().st_size>2_000_000:continue
            data=p.read_text(encoding='utf-8')
        except (OSError, UnicodeError):continue
        for lineno,line in enumerate(data.splitlines(),1):
            hits=[name for name,regex in PATTERNS.items() if regex.search(line)]
            if hits:
                findings.append({'path':rel,'line':lineno,'tokens':hits,
                                 'classification':classify(rel,line),'excerpt':line.strip()[:220]})
    return findings

def main(argv=None):
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root',default='.',help='Repository root to inspect (read only)')
    parser.add_argument('--json-out',help='Optional explicit JSON receipt path')
    parser.add_argument('--verify',action='store_true',help='Fail if active identity references need review')
    args=parser.parse_args(argv)
    root=Path(args.root).resolve()
    if not (root/'project.control.json').is_file():
        parser.error('Expected project.control.json at --root; no files modified')
    findings=scan(root)
    by_class=Counter(item['classification'] for item in findings)
    by_token=Counter(t for item in findings for t in item['tokens'])
    manifest=json.loads((root/'project.control.json').read_text(encoding='utf-8'))
    identity={k:manifest.get(k) for k in ('schema','id','name','adapter','stateDirectory')}
    identity['project']=manifest.get('project',{})
    result={'audit':'null-harbor-rebrand-inventory.v1','readOnly':True,
            'identity':identity,'countsByClass':dict(sorted(by_class.items())),
            'countsByToken':dict(sorted(by_token.items())),'findings':findings}
    print('NULL HARBOR REBRAND AUDIT — READ ONLY')
    print('Manifest identity:',json.dumps(identity,sort_keys=True))
    print('Findings:',len(findings),'classes:',dict(by_class),'tokens:',dict(by_token))
    if args.json_out:
        target=Path(args.json_out).resolve()
        if target==root/'project.control.json' or target.is_dir():parser.error('Unsafe output target')
        target.parent.mkdir(parents=True,exist_ok=True)
        target.write_text(json.dumps(result,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
        print('Receipt:',target)
    blockers=by_class['active_review']+by_class['unclassified_review']
    if args.verify and blockers:
        print('NOT READY FOR GITHUB RENAME: active references still require audited migration')
        return 1
    return 0
if __name__=='__main__':sys.exit(main())
