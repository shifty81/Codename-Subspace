#!/usr/bin/env python3
"""Read-only source guards: prevents silently dropping gizmos or shader isolation."""
from pathlib import Path
import argparse

def check(root):
    app=(root/'engine/src/studio/StudioApplication.cpp').read_text(encoding='utf-8')
    gizmo=(root/'engine/src/studio/StudioAxisGizmo.cpp').read_text(encoding='utf-8')
    guard=(root/'engine/include/studio/StudioOverlayProgramScope.h').read_text(encoding='utf-8')
    assert '#include "studio/StudioOverlayProgramScope.h"' in app
    assert 'const StudioOverlayProgramScope overlayProgramScope;' in app
    assert 'StudioGizmoOverlay::Draw(gizmo,' in app
    assert gizmo.count('StudioGizmoProjectionPolicy::Direction(')==2
    assert '(assemblyMode&&model.inspectorTab==ShipyardInspectorTab::Sockets)' in gizmo
    assert 'glGetIntegerv(static_cast<GLenum>(0x8B8D)' in guard
    assert 'use_(0)' in guard and 'use_(static_cast<GLuint>(previous_))' in guard
    print('STUDIO_FOUNDATION_R3_WIDGET_OVERLAY_STATIC PASS')

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--root',default='.')
    a=p.parse_args();check(Path(a.root).resolve())
