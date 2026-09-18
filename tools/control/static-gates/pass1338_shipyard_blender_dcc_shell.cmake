# Pass1338: historical Blender-derived Shipyard DCC shell and rendered smoke authority.
# Later DCC tranches may supersede exact coordinates/profile identity while retaining this foundation.
# Pass1444-1453 promotes the shell into EditorDccShellLayoutSystem, so certify
# structural DCC ownership rather than the retired fixed top-coordinate literal.
get_filename_component(P1338_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)

function(p1338_require_text rel token)
  set(path "${P1338_ROOT}/${rel}")
  if(NOT EXISTS "${path}")
    message(FATAL_ERROR "Pass1338 missing required file: ${rel}")
  endif()
  file(READ "${path}" text)
  string(FIND "${text}" "${token}" pos)
  if(pos EQUAL -1)
    message(FATAL_ERROR "Pass1338 token '${token}' missing from ${rel}")
  endif()
endfunction()

p1338_require_text("engine/include/application/SubspaceBuildIdentity.h" "BLENDER-DCC-")
p1338_require_text("engine/include/application/SubspaceBuildIdentity.h" "PASS")
p1338_require_text("engine/src/main.cpp" "--shipyard-smoke")
p1338_require_text("engine/src/application/NativeGameApplication.cpp" "options.shipyardSmoke ? 1280")
# G2 removes historical File/Edit/View/Help TEXT: those labels had no menus.
# A usable application menu will return only when it has real command handlers.
# The old "View Select Add Object" renderer string was NONINTERACTIVE and was
# removed in the Asset Browser recovery: it painted over real ASSETS / RESET UI
# hit rectangles. Certify the current viewport label and reachable command
# controls/handlers instead of requiring a misleading, obsolete text overlay.
# This historical test does NOT certify functional application menus.
p1338_require_text("engine/src/application/NativeBattlefieldRenderer.cpp" "SUBSPACE / SHIP DOCUMENT")
p1338_require_text("engine/src/ship_editor/ShipyardProfessionalVisibleCutover.cpp" "NewEmptyDocument,0,188.0f*s")
p1338_require_text("engine/src/application/NativeBattlefieldRenderer.cpp" "ShipyardText(\"3D VIEW\"")
p1338_require_text("engine/src/ship_editor/ShipyardProfessionalVisibleCutover.cpp" "DccRevealAssetBrowser,0,8.0f*s")
p1338_require_text("engine/src/ship_editor/ShipyardProfessionalVisibleCutover.cpp" "DccResetLayout,0,95.0f*s")
p1338_require_text("engine/src/ship_editor/ShipyardProfessionalVisibleCutover.cpp" "case ShipyardBuilderCommand::DccRevealAssetBrowser:")
p1338_require_text("engine/src/ship_editor/ShipyardProfessionalVisibleCutover.cpp" "SubspaceDockSystem::OpenPanel(model_.dockWorkspace,panel->id)")
p1338_require_text("engine/src/ship_editor/ShipyardProfessionalVisibleCutover.cpp" "case ShipyardBuilderCommand::DccResetLayout:")
p1338_require_text("engine/src/application/NativeBattlefieldRenderer.cpp" "\"OUTLINER\"")
p1338_require_text("engine/src/application/NativeBattlefieldRenderer.cpp" "\"PROPERTIES\"")
p1338_require_text("engine/src/application/NativeBattlefieldRenderer.cpp" "build_identity::kShipyardUiProfile")
p1338_require_text("engine/src/ship_editor/ShipyardProfessionalVisibleCutover.cpp" "EditorDccShellLayoutSystem::Compute")
p1338_require_text("engine/src/editor/EditorDccShellLayoutSystem.cpp" "out.assetShelf")
p1338_require_text("SubspaceTools.ps1" "Native Shipyard rendered smoke")
p1338_require_text("docs/design/SHIPYARD_BLENDER_UI_CONTRACT.md" "Pass1338 acceptance")

message(STATUS "Pass1338 Blender-derived Shipyard DCC shell source authority PASS")
