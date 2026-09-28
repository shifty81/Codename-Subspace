cmake_minimum_required(VERSION 3.20)
get_filename_component(ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
function(require_token rel token)
  set(path "${ROOT}/${rel}")
  if(NOT EXISTS "${path}")
    message(FATAL_ERROR "PASS1511-1520 missing ${rel}")
  endif()
  file(READ "${path}" text)
  string(FIND "${text}" "${token}" pos)
  if(pos EQUAL -1)
    message(FATAL_ERROR "PASS1511-1520 ${rel} missing token: ${token}")
  endif()
endfunction()
function(forbid_token rel token)
  set(path "${ROOT}/${rel}")
  file(READ "${path}" text)
  string(FIND "${text}" "${token}" pos)
  if(NOT pos EQUAL -1)
    message(FATAL_ERROR "PASS1511-1520 ${rel} still exposes retired token: ${token}")
  endif()
endfunction()

# R33: measurement HUD reflows rather than painting through floating docks.
require_token("engine/src/studio/StudioApplication.cpp" "StudioOverlayPlacementPolicy::Choose")
require_token("engine/include/studio/StudioAxisGizmo.h" "readoutLeft")
require_token("engine/src/studio/StudioGizmoOverlay.cpp" "snapshot.readoutLeft")
require_token("engine/include/studio/StudioOverlayPlacementPolicy.h" "fail closed")

# R34-R36: visible manipulation language is context-neutral XYZ; semantic ship
# naming remains available in content/socket code but is not the editor ruler.
require_token("engine/src/ship_editor/ShipyardBuilderSystem.cpp" "ShipyardBuilderCommand::NudgePort,\"X -\"")
require_token("engine/src/ship_editor/ShipyardBuilderSystem.cpp" "ShipyardBuilderCommand::NudgeForward,\"Y +\"")
require_token("engine/src/ship_editor/ShipyardBuilderSystem.cpp" "ShipyardBuilderCommand::NudgeDorsal,\"Z +\"")
require_token("engine/src/ship_editor/ShipyardBuilderSystem.cpp" "ShipyardBuilderCommand::SymmetryAxisPortStarboard,\"MIRROR X\"")
require_token("engine/src/ship_editor/ShipyardBuilderSystem.cpp" "Symmetry plane: MIRROR Y / LENGTH")
forbid_token("engine/src/ship_editor/ShipyardBuilderSystem.cpp" "ShipyardBuilderCommand::SymmetryAxisPortStarboard,\"PORT <-> STARBOARD\"")
forbid_token("engine/src/ship_editor/ShipyardBuilderSystem.cpp" "ShipyardBuilderCommand::NudgeStarboard,\"STARBOARD\"")

# R37-R40: R32 public Construct authority remains intact; this pass may not
# revive MODEL as a peer workspace or the movable tool-rail regression.
require_token("engine/src/ship_editor/ShipyardWorkspaceSystem.cpp" "case ShipyardWorkspaceMode::Build:return\"CONSTRUCT\";")
require_token("engine/src/ship_editor/ShipyardWorkspaceSystem.cpp" "case ShipyardWorkspaceMode::Model:return\"GEOMETRY\";")
require_token("engine/src/ship_editor/ShipyardWorkspaceSystem.cpp" "add(\"tool_rail\",\"Tools\",\"tool_left\",true,1.0f,48,420,false,false,false);")
forbid_token("engine/src/ship_editor/ShipyardProfessionalVisibleCutover.cpp" "tab(ShipyardBuilderCommand::WorkspaceModel,\"MODEL\"")

# R41-R42: edits are guarded by PCC pre-build orchestration and remain usable
# through the established undo/redo/delete keyboard surface.
require_token("SubspaceTools.ps1" "Invoke-StudioBulkPolishNormalizationIfRequired")
require_token("engine/src/studio/StudioApplication.cpp" "builder_.UndoAuthoring()")
require_token("engine/src/studio/StudioApplication.cpp" "builder_.RedoAuthoring()")
require_token("engine/src/studio/StudioApplication.cpp" "InputAction::EditorDeleteModule")
require_token("engine/src/studio/StudioApplication.cpp" "assetSearchFocused")
message(STATUS "PASS1511-1520 Studio bulk polish / axis / overlap authority PASS")
