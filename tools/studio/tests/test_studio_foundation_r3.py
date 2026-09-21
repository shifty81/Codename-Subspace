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
    # R3R2 moved the Model/Assembly duplication into one PopulateHandles helper.
    # Require the shared helper AND the complete direction/probe/handle pipeline;
    # rejecting the correct refactor due to a hard-coded occurrence count is a
    # false certification failure, not a useful regression guard.
    assert gizmo.count('StudioGizmoProjectionPolicy::Direction(')==1
    assert 'StudioGizmoProjectionPolicy::ProbeUsable(' in gizmo
    assert 'StudioGizmoProjectionPolicy::BuildHandles(' in gizmo
    assert gizmo.count('PopulateHandles(out,camera,width,height,')==2
    assert '(assemblyMode&&model.inspectorTab==ShipyardInspectorTab::Sockets)' in gizmo
    assert 'glGetIntegerv(static_cast<GLenum>(0x8B8D)' in guard
    assert 'use_(0)' in guard and 'use_(static_cast<GLuint>(previous_))' in guard
    policy=(root/'engine/include/studio/StudioGizmoProjectionPolicy.h').read_text(encoding='utf-8')
    assert 'ReflowForOcclusion(' in policy
    assert 'StudioGizmoProjectionPolicy::ReflowForOcclusion(' in app
    assert 'ShipyardPanelCompositorSystem::Snapshot(' in app
    assert 'ShipyardPanelCompositorSystem::TopFloatingAt(layers,x,y)' in app
    assert 'frame.editorInteriorShell=&interiorPreview_.shell;' in app
    print('STUDIO_R4_WIDGET_OVERLAY_INTERIOR_STATIC PASS')

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--root',default='.')
    a=p.parse_args();check(Path(a.root).resolve())
