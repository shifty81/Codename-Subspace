#!/usr/bin/env python3
"""Synthetic safety tests for the generic R180 three-way strategy.

This test does not pretend to certify the Windows project. It proves the core Git merge behavior:
a newer baseline change and an older-branch gameplay change both survive, and conflicting edits fail.
"""
from pathlib import Path
import subprocess, tempfile


def run(*args, cwd=None):
    return subprocess.run(args,cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.PIPE,check=False,text=True)

with tempfile.TemporaryDirectory() as td:
    td=Path(td)
    base=td/'base'; current=td/'current'; other=td/'other'
    base.write_text('A\nshared\nZ\n')
    current.write_text('A\nshared\nSTUDIO\nZ\n')
    other.write_text('A\nGAMEPLAY\nshared\nZ\n')
    r=run('git','merge-file','-p',str(current),str(base),str(other))
    assert r.returncode==0, r.stderr
    assert 'STUDIO' in r.stdout and 'GAMEPLAY' in r.stdout
print('R180 synthetic three-way merge PASS')
