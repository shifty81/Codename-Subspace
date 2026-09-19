#!/usr/bin/env python3
"""Explicit second-stage G02 C++ repair. Exact Git-blob preimages, no fuzzy mutation.

The project PCC's forge.patch.v1 archive handler replaces whole files only.
This script is intentionally NOT auto-run by intake or Full Gate; use --check
before --apply, and the normal PCC Full Gate afterward. Source is backed up
before atomic replacements, and all preconditions are validated before writes.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
from datetime import datetime, timezone

PREIMAGES = {
    'engine/src/studio/StudioApplication.cpp': '26cccfaa83dcb550b1e914dbe6a3f0b8eb385228',
    'engine/include/studio/StudioApplication.h': '442e2bbafb354fd7b83a3e35f70fa7b8edf08c1e',
    'engine/tests/studio_close_policy_tests.cpp': 'f055059d4ca8725bb44049ef0099fe37df023d9e',
}
MARKER='StudioExitOutcomePolicy'

def git_blob(data: bytes) -> str:
    return hashlib.sha1(b'blob '+str(len(data)).encode()+b'\0'+data).hexdigest()

def replace_once(src: str, old: str, new: str, label: str) -> str:
    count=src.count(old)
    if count!=1:
        raise ValueError(f'{label}: expected exactly one source anchor, found {count}')
    return src.replace(old,new,1)

def edit_cpp(src: str) -> str:
    src=replace_once(src,
       '#include "studio/StudioClosePolicy.h"\n',
       '#include "studio/StudioClosePolicy.h"\n#include "studio/StudioExitOutcomePolicy.h"\n', 'include')
    src=replace_once(src,
       '    if(!closeRecoveryPrepared_ && StudioRecoveryPathPolicy::NeedsRecovery(unsaved.blueprint)){',
       '    if(!StudioExitOutcomePolicy::IsUserConfirmed(closeOutcome_) &&\n'
       '       !closeRecoveryPrepared_ && StudioRecoveryPathPolicy::NeedsRecovery(unsaved.blueprint)){',
       'unconfirmed recovery')
    src=replace_once(src,
       '    if(StudioUnsavedWorkPolicy::HasUnsupportedRecovery(unsaved)){\n'
       '        std::cerr<<"Studio exit WARNING: socket/definition overrides and editable model/interior drafts are NOT in blueprint recovery\\n";\n'
       '        exitCode=7;\n'
       '    }\n'
       '    closeGuard_.Detach();',
       '    const bool unsupported=StudioUnsavedWorkPolicy::HasUnsupportedRecovery(unsaved);\n'
       '    if(unsupported){\n'
       '        std::cerr<<"Studio exit WARNING: socket/definition overrides and editable model/interior drafts are NOT in blueprint recovery\\n";\n'
       '    }\n'
       '    exitCode=StudioExitOutcomePolicy::ExitCode(closeOutcome_,exitCode==6,unsupported);\n'
       '    std::cerr<<"STUDIO_EXIT_RESULT schema=subspace.studio-exit.v1 outcome="\n'
       '             <<StudioExitOutcomePolicy::Name(closeOutcome_)\n'
       '             <<" blueprint_recovery_verified="<<(closeRecoveryPrepared_?"true":"false")\n'
       '             <<" unsupported_drafts="<<(unsupported?"true":"false")\n'
       '             <<" process_exit="<<exitCode<<\'\\n\';\n'
       '    closeGuard_.Detach();', 'exit classification')
    src=replace_once(src,
       '    if(!StudioClosePolicy::NeedsPrompt(before))return true;',
       '    if(!StudioClosePolicy::NeedsPrompt(before)){\n'
       '        closeOutcome_=StudioExitOutcome::Clean;return true;\n'
       '    }', 'clean close')
    src=replace_once(src,
       '        if(!StudioClosePolicy::MayCloseAfterSave(CloseState())){\n'
       '            StudioFileDialog::ShowError("Studio remains open: the document has unsaved changes.");\n'
       '            return false;\n'
       '        }\n'
       '        return true;',
       '        if(!StudioClosePolicy::MayCloseAfterSave(CloseState())){\n'
       '            StudioFileDialog::ShowError("Studio remains open: the document has unsaved changes.");\n'
       '            return false;\n'
       '        }\n'
       '        closeOutcome_=StudioExitOutcome::Saved;\n'
       '        return true;', 'saved close')
    src=replace_once(src,
       '    return StudioClosePolicy::MayCloseWithRecovery(before,recovered,acknowledged);',
       '    if(!StudioClosePolicy::MayCloseWithRecovery(before,recovered,acknowledged))return false;\n'
       '    // The user explicitly approved partial recovery / loss of draft-only data.\n'
       '    // This is normal app termination, NOT proof of a complete Studio save.\n'
       '    closeOutcome_=StudioExitOutcome::UserConfirmedPartialRecovery;\n'
       '    return true;', 'consented close')
    return src

def edit_header(src: str) -> str:
    src=replace_once(src,
       '#include "studio/StudioClosePolicy.h"\n',
       '#include "studio/StudioClosePolicy.h"\n#include "studio/StudioExitOutcomePolicy.h"\n','header include')
    return replace_once(src,
       '    bool closePromptActive_=false;\n',
       '    bool closePromptActive_=false;\n'
       '    StudioExitOutcome closeOutcome_=StudioExitOutcome::Unconfirmed;\n', 'outcome field')

def edit_test(src: str) -> str:
    src=replace_once(src,
       '#include "studio/StudioClosePolicy.h"\n',
       '#include "studio/StudioClosePolicy.h"\n#include "studio/StudioExitOutcomePolicy.h"\n','test include')
    return replace_once(src,
       '    std::cout<<"Studio close policy: "',
       '    expect(StudioExitOutcomePolicy::ExitCode(StudioExitOutcome::Clean,false,false)==0,"clean close succeeds");\n'
       '    expect(StudioExitOutcomePolicy::ExitCode(StudioExitOutcome::Saved,false,false)==0,"saved close succeeds");\n'
       '    expect(StudioExitOutcomePolicy::ExitCode(StudioExitOutcome::UserConfirmedPartialRecovery,false,true)==0,"acknowledged partial recovery is normal exit");\n'
       '    expect(StudioExitOutcomePolicy::ExitCode(StudioExitOutcome::Unconfirmed,false,true)==7,"unconfirmed unsupported draft is failure");\n'
       '    expect(StudioExitOutcomePolicy::ExitCode(StudioExitOutcome::UserConfirmedPartialRecovery,true,true)==6,"failed recovery remains an error");\n'
       '    expect(!StudioExitOutcomePolicy::IsUserConfirmed(StudioExitOutcome::Unconfirmed),"frame limit is not user consent");\n'
       '    std::cout<<"Studio close policy: "','test assertions')

EDITORS={
    'engine/src/studio/StudioApplication.cpp':edit_cpp,
    'engine/include/studio/StudioApplication.h':edit_header,
    'engine/tests/studio_close_policy_tests.cpp':edit_test,
}

def stage(root: Path):
    pending={}
    status={}
    for rel,editor in EDITORS.items():
        path=root/rel
        if not path.is_file():raise ValueError(f'Missing source: {rel}')
        original=path.read_bytes()
        if b'\r\n' in original:raise ValueError(f'Unexpected line endings: {rel}')
        h=git_blob(original)
        if h!=PREIMAGES[rel]:
            if MARKER.encode() in original:
                raise ValueError(f'{rel}: already modified; inspect receipt, do not reapply')
            raise ValueError(f'{rel}: PREIMAGE_CONFLICT expected {PREIMAGES[rel]}, got {h}; no writes')
        edited=editor(original.decode('utf-8')).encode('utf-8')
        if edited==original:raise ValueError(f'{rel}: no change')
        pending[rel]=(original,edited)
        status[rel]={'preimageBlob':h,'oldSha256':hashlib.sha256(original).hexdigest(),
                     'newSha256':hashlib.sha256(edited).hexdigest(), 'oldBytes':len(original),
                     'newBytes':len(edited)}
    header=root/'engine/include/studio/StudioExitOutcomePolicy.h'
    if not header.is_file():raise ValueError('G02 policy header absent; apply G02 PCC package first')
    if 'StudioExitOutcomePolicy' not in header.read_text('utf-8'):
        raise ValueError('G02 policy header is not valid')
    return pending,status

def apply(root:Path,pending,status):
    # Backup all files before first write. Leave immutable backup and a receipt.
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%SZ')
    backup=root/'.subspace'/'recovery'/'studio-g02'/stamp
    backup.mkdir(parents=True,exist_ok=False)
    for rel,(original,_) in pending.items():
        dest=backup/rel;dest.parent.mkdir(parents=True,exist_ok=True);dest.write_bytes(original)
    receipt=backup/'RECEIPT.json'
    meta={'schema':'subspace.studio-g02-transaction.v1','utc':datetime.now(timezone.utc).isoformat(),
          'state':'BACKED_UP', 'files':status,'backupRoot':str(backup)}
    receipt.write_text(json.dumps(meta,indent=2)+'\n',encoding='utf-8')
    written=[]
    try:
        for rel,(original,edited) in pending.items():
            dest=root/rel
            if dest.read_bytes()!=original:raise RuntimeError(f'CONCURRENT_EDIT: {rel}')
            fd,tmp=tempfile.mkstemp(prefix='.studio-g02-',dir=dest.parent)
            try:
                with os.fdopen(fd,'wb') as f:
                    f.write(edited); f.flush();os.fsync(f.fileno())
                os.replace(tmp,dest)
            finally:
                if os.path.exists(tmp):os.unlink(tmp)
            written.append(rel)
            if hashlib.sha256(dest.read_bytes()).hexdigest()!=status[rel]['newSha256']:
                raise RuntimeError(f'Verification failed: {rel}')
    except Exception:
        for rel in written:
            (root/rel).write_bytes((backup/rel).read_bytes())
        meta['state']='ROLLED_BACK';receipt.write_text(json.dumps(meta,indent=2)+'\n',encoding='utf-8')
        raise
    meta['state']='APPLIED';receipt.write_text(json.dumps(meta,indent=2)+'\n',encoding='utf-8')
    return receipt

def main(argv=None):
    p=argparse.ArgumentParser(description='Explicit exact-preimage Studio G02 close-outcome repair')
    mode=p.add_mutually_exclusive_group(required=True)
    mode.add_argument('--check',action='store_true',help='validate exact source without any changes')
    mode.add_argument('--apply',action='store_true',help='back up and apply guarded code changes')
    p.add_argument('--root',type=Path,default=Path(__file__).resolve().parents[2]);args=p.parse_args(argv)
    root=args.root.resolve()
    try:
        pending,status=stage(root)
        if args.check:
            print('G02 PREIMAGE_CHECK PASS (read-only)')
            for rel,metadata in status.items():print(rel,metadata['preimageBlob'],metadata['newSha256'])
        else:
            receipt=apply(root,pending,status)
            print('G02 SOURCE_REPAIR APPLIED; backup and receipt:',receipt)
            print('NEXT: run existing project-owned PCC Full Quality Gate and Windows Studio close tests.')
    except (OSError,UnicodeError,ValueError,RuntimeError) as e:
        print('G02 BLOCKED:',str(e),file=sys.stderr);return 1
    return 0

if __name__=='__main__':raise SystemExit(main())
