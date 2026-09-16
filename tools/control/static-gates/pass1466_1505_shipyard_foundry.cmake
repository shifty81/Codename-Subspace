# PASS1466-1505 — Shipyard Foundry cumulative authoring authority.
get_filename_component(P1505_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)

function(p1505_require rel token)
  set(path "${P1505_ROOT}/${rel}")
  if(NOT EXISTS "${path}")
    message(FATAL_ERROR "PASS1466-1505 missing required file: ${rel}")
  endif()
  file(READ "${path}" body)
  string(FIND "${body}" "${token}" pos)
  if(pos EQUAL -1)
    message(FATAL_ERROR "PASS1466-1505 missing token '${token}' in ${rel}")
  endif()
endfunction()

# Cumulative identity: successor visible identity + retained historical tokens.
p1505_require("engine/include/application/SubspaceBuildIdentity.h" "SHIPYARD-FOUNDRY")
p1505_require("engine/include/application/SubspaceBuildIdentity.h" "PASS1505")
p1505_require("engine/include/application/SubspaceBuildIdentity.h" "PASS1466-1505")
p1505_require("engine/include/application/SubspaceBuildIdentity.h" "FIRST-CLASS-DCC")
p1505_require("engine/include/application/SubspaceBuildIdentity.h" "PASS1465")

# Camera and native input. Pan must move eye + target and X/Y/Z are editor actions.
p1505_require("engine/src/editor/ConstructionEditorCameraSystem.cpp" "s.assemblyCenter=s.assemblyCenter+delta")
p1505_require("engine/src/platform/NativeWindow.cpp" "WM_MBUTTONDOWN")
p1505_require("engine/src/platform/NativeWindow.cpp" "WM_RBUTTONDOWN")
p1505_require("engine/src/platform/NativeWindow.cpp" "InputAction::DccConstraintX")
p1505_require("engine/src/platform/NativeWindow.cpp" "InputAction::DccConstraintY")
p1505_require("engine/src/platform/NativeWindow.cpp" "InputAction::DccConstraintZ")
p1505_require("engine/include/input/InputState.h" "DccConstraintClear")

# Transform/pivot/articulation authoring and runtime consumption.
p1505_require("engine/src/ship_editor/ShipyardBuilderSystem.cpp" "GLOBAL-SHIP (press again for LOCAL)")
p1505_require("engine/src/ship_editor/ShipyardBuilderSystem.cpp" "UseSelectedSocketAsArticulationPivot")
p1505_require("engine/include/rendering/ProceduralVisualVariantSystem.h" "ScanSweep")
p1505_require("engine/src/application/NativeBattlefieldRenderer.cpp" "ShipArticulationSystem::Apply")

# Interior structural authoring including removable/modelable airlocks.
p1505_require("engine/include/interior/ShipInteriorStructureAuthoringSystem.h" "Airlock")
p1505_require("engine/include/interior/ShipInteriorStructureAuthoringSystem.h" "pressureBoundary")
p1505_require("engine/src/interior/ShipInteriorStructureAuthoringSystem.cpp" "removable")
p1505_require("engine/src/ship_editor/ShipyardBuilderSystem.cpp" "InteriorAddAirlock")
p1505_require("engine/src/ship_editor/ShipyardBuilderSystem.cpp" "GenerateInteriorProgram")

# Modeling is native geometry authoring; Boolean/UV backends remain explicit future integrations.
p1505_require("engine/include/modeling/ShipyardModelingSystem.h" "BooleanSubtract")
p1505_require("engine/include/modeling/ShipyardModelingSystem.h" "Extrude")
p1505_require("engine/src/modeling/ShipyardModelingSystem.cpp" "DuplicatePrimitive")
p1505_require("content/architecture/shipyard_foundry_authoring_v1.json" "CONTRACT_READY_BACKEND_PENDING")
p1505_require("content/architecture/shipyard_foundry_authoring_v1.json" "XATLAS_CANDIDATE_NOT_VENDORED")

# Appearance/material workflow.
p1505_require("engine/include/appearance/ShipPaintFinishSystem.h" "iridescence")
p1505_require("engine/src/appearance/ShipPaintFinishSystem.cpp" "Black Chrome")
p1505_require("engine/src/appearance/ShipPaintFinishSystem.cpp" "Pearlescent")
p1505_require("engine/src/appearance/ShipPaintFinishSystem.cpp" "Iridescent")
p1505_require("engine/src/ship_editor/ShipyardBuilderSystem.cpp" "FACTION_CREST")
p1505_require("engine/src/ship_editor/ShipyardBuilderSystem.cpp" "HULL_NUMBER_")
p1505_require("engine/include/rendering/ProceduralVisualVariantSystem.h" "sourceMaterialsEnabled")

# Material health must be corpus-aware and fail honest when hydrated assets are absent.
p1505_require("engine/include/content/ShipyardMaterialAuditSystem.h" "MissingBaseColorTexture")
p1505_require("engine/src/content/ShipyardMaterialAuditSystem.cpp" "NormalizedFallback")
p1505_require("scripts/subspace_shipyard_material_audit.py" "hydrated Shipyard OBJ corpus is not present")

# ForgeGUI-inspired dockability/persistence without embedding Rust/egui.
p1505_require("engine/include/ui/SubspaceUiFramework.h" "FloatPanel")
p1505_require("engine/include/ui/SubspaceUiFramework.h" "TogglePinned")
p1505_require("engine/include/ui/SubspaceUiFramework.h" "SetAutoHide")
p1505_require("engine/include/ui/SubspaceUiFramework.h" "Serialize")
p1505_require("engine/src/ui/SubspaceUiFramework.cpp" "SUBSPACE_DOCK_V1")
p1505_require("engine/src/ship_editor/ShipyardWorkspaceSystem.cpp" "Kitbash Intake")
p1505_require("engine/src/ship_editor/ShipyardWorkspaceSystem.cpp" "Material Health")
p1505_require("engine/src/ship_editor/ShipyardWorkspaceSystem.cpp" "Attachments & Pivots")

# Workflow/navigation hierarchy and global-menu normalization.
p1505_require("engine/src/ship_editor/ShipyardWorkflowSystem.cpp" "3. Author attachments")
p1505_require("engine/src/ship_editor/ShipyardProfessionalVisibleCutover.cpp" "ASSEMBLY")
p1505_require("engine/src/ship_editor/ShipyardProfessionalVisibleCutover.cpp" "MODEL")
p1505_require("engine/src/ship_editor/ShipyardProfessionalUiSystem.cpp" "File")
p1505_require("engine/src/ship_editor/ShipyardProfessionalUiSystem.cpp" "Help")
p1505_require("docs/design/SHIPYARD_FOUNDRY_PASS1466_1505.md" "ASSEMBLY → MODEL → ATTACHMENTS")

file(READ "${P1505_ROOT}/engine/src/ship_editor/ShipyardProfessionalUiSystem.cpp" p1505_ui)
string(FIND "${p1505_ui}" "{\"shipyard\", \"Shipyard\"" stale_shipyard_menu)
if(NOT stale_shipyard_menu EQUAL -1)
  message(FATAL_ERROR "PASS1466-1505 stale global Shipyard/SUBSPACE-style menu label returned")
endif()

message(STATUS "PASS1466-1505 Shipyard Foundry cumulative authoring authority PASS")
