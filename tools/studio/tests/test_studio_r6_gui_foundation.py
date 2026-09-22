from pathlib import Path

root=Path(__file__).resolve().parents[3]
app=(root/'engine/src/studio/StudioApplication.cpp').read_text(encoding='utf-8')
gui=(root/'engine/include/studio/StudioGuiInteractionPolicy.h').read_text(encoding='utf-8')
compositor=(root/'engine/include/ship_editor/ShipyardPanelCompositorSystem.h').read_text(encoding='utf-8')
style=(root/'engine/src/editor/EditorForgeGuiStyleSystem.cpp').read_text(encoding='utf-8')
cmake=(root/'engine/CMakeLists.txt').read_text(encoding='utf-8')
assert 'StudioGizmoDragPolicy::MoveUnits(gizmoStartHandle_' in app, 'R5 movement calibration must survive'
assert 'StudioGuiInteractionPolicy::DragActivated(travel)' in app
assert 'StudioGuiInteractionPolicy::ClearOverlayArea(layers,hud)' in app
assert 'StudioGizmoProjectionPolicy::ReflowForOcclusion' in app
assert 'StudioGizmoOverlay::Draw(gizmo,' in app
assert 'kPointerActivationPixels=3.0f' in gui
assert 'ShipyardPanelCompositorSystem::Intersects(layer.rect,area)' in gui
assert 'static bool Intersects(' in compositor
assert 'p.accent=c(90,170,244)' in style
assert 'SubspaceStudioGuiInteractionR6Tests' in cmake
print('R6 GUI foundation static integration: PASS')
