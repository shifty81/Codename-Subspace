# Pass1048-1097 source-level certification for unified authoring/runtime/UI parity.
file(READ "${ROOT}/src/editor/AuthoringStandardsSystem.cpp" AUTHORING)
file(READ "${ROOT}/src/content/ShipyardModuleSystem.cpp" MODULES)
file(READ "${ROOT}/src/modeling/ShipyardModelingSystem.cpp" MODELING)
file(READ "${ROOT}/src/construction/CohesiveAssemblyBakeSystem.cpp" BAKE)
file(READ "${ROOT}/include/editor/AuthoringStandardsSystem.h" AUTHORING_H)
file(READ "${ROOT}/src/rendering/ConformalShieldSurfaceSystem.cpp" SHIELD_POLICY)
file(READ "${ROOT}/src/application/NativeBattlefieldRenderer.cpp" RENDERER)
file(READ "${ROOT}/src/ui/SubspaceUiFramework.cpp" UI)
file(READ "${ROOT}/src/ui/RuntimeControlContextSystem.cpp" CONTROL)
file(READ "${ROOT}/include/ui/RuntimeControlContextSystem.h" CONTROL_H)
file(READ "${ROOT}/src/generator/GeneratorParitySystem.cpp" GENERATORS)
file(READ "${ROOT}/src/ship_editor/ShipyardWorkspaceSystem.cpp" SHIPYARD_WORKSPACE)
file(READ "${ROOT}/src/ship_editor/ShipyardBuilderSystem.cpp" SHIPYARD_BUILDER)
file(READ "${ROOT}/../tools/blender/SubspaceShipyard/__init__.py" BLENDER)
file(READ "${ROOT}/../content/architecture/generator_parity_registry_v1.json" GENERATOR_CONTRACT)
file(READ "${ROOT}/../content/architecture/unified_ui_authority_v1.json" UI_CONTRACT)
file(READ "${ROOT}/../content/architecture/shipyard_authoring_runtime_authority_v2.json" AUTHORING_CONTRACT)

set(AUTHORING_TOKENS
    "DiscoverFlatSnapSurfaces"
    "supportingArea"
    "minConfidence"
    "maximumInsertionMeters"
    "SemanticObjectPurpose::Seat"
    "SemanticObjectPurpose::Console"
    "referenceDoorHeightMeters")
foreach(TOKEN IN LISTS AUTHORING_TOKENS)
    if(NOT AUTHORING MATCHES "${TOKEN}")
        message(FATAL_ERROR "PASS1048-1097 shared authoring standard missing token: ${TOKEN}")
    endif()
endforeach()

if(NOT MODULES MATCHES "DiscoverFlatSnapSurfaces" OR NOT MODULES MATCHES "surface_" OR NOT MODULES MATCHES "maximumInsertionMeters")
    message(FATAL_ERROR "PASS1048-1097 Shipyard socket generation is not consuming measured flat-surface snap authority")
endif()
if(NOT MODELING MATCHES "AssignSemanticPurpose" OR NOT MODELING MATCHES "PURPOSE=" OR NOT MODELING MATCHES "ValidateObject")
    message(FATAL_ERROR "PASS1048-1097 Add Shape/modeling workflow lacks semantic gameplay-purpose validation")
endif()
if(NOT BAKE MATCHES "_cohesive.obj" OR NOT AUTHORING_H MATCHES "removeOccludedInternalExteriorFaces" OR NOT AUTHORING_H MATCHES "carveWalkableInterior")
    message(FATAL_ERROR "PASS1048-1097 cohesive assembly bake contract is incomplete")
endif()
if(NOT RENDERER MATCHES "PointBuriedInsideAnotherModule" OR NOT RENDERER MATCHES "ConformalShieldSurfaceSystem::Select" OR NOT RENDERER MATCHES "triangleStride")
    message(FATAL_ERROR "PASS1048-1097 live shield renderer does not consume exterior-shell filtering/performance LOD authority")
endif()
if(NOT SHIELD_POLICY MATCHES "visibleShieldCount" OR NOT SHIELD_POLICY MATCHES "StrategicOnly" OR NOT SHIELD_POLICY MATCHES "maxNearRipples")
    message(FATAL_ERROR "PASS1048-1097 conformal shield performance policy missing")
endif()

set(CONTROL_TOKENS
    "CockpitFirstPerson"
    "OnFootFirstPerson"
    "RemoteFleetCommand"
    "AuthoringDev")
foreach(TOKEN IN LISTS CONTROL_TOKENS)
    if(NOT CONTROL MATCHES "${TOKEN}" AND NOT CONTROL_H MATCHES "${TOKEN}")
        message(FATAL_ERROR "PASS1048-1097 runtime view authority missing: ${TOKEN}")
    endif()
endforeach()

set(UI_TOKENS
    "SubspaceDockSystem::CreateMinimalWorkspace"
    "SubspaceDockSystem::ResizeSplit"
    "SubspaceDockSystem::SetPanelOpacity"
    "SubspaceDockSystem::FloatPanel"
    "SubspaceDockSystem::ResizeFloating"
    "panelOpacityMinimum"
    "scrollThumb")
foreach(TOKEN IN LISTS UI_TOKENS)
    if(NOT UI MATCHES "${TOKEN}")
        message(FATAL_ERROR "PASS1048-1097 universal UI/dock authority missing token: ${TOKEN}")
    endif()
endforeach()
if(NOT SHIPYARD_WORKSPACE MATCHES "BuildDefaultDockWorkspace" OR NOT SHIPYARD_BUILDER MATCHES "authoringDomain")
    message(FATAL_ERROR "PASS1048-1097 Shipyard does not consume shared dock/domain authorities")
endif()
if(NOT SHIPYARD_BUILDER MATCHES "ModelPreviousPurpose" OR
   NOT SHIPYARD_BUILDER MATCHES "ModelAssignPurpose" OR
   NOT SHIPYARD_BUILDER MATCHES "ModelNextPurpose" OR
   NOT SHIPYARD_BUILDER MATCHES "Assigned gameplay purpose" OR
   NOT SHIPYARD_BUILDER MATCHES "AuthoringStandardsSystem::PurposeName")
    message(FATAL_ERROR "PASS1048-1097 visible Model/Add Shape workflow lacks semantic purpose selection/assignment controls")
endif()

if(NOT GENERATORS MATCHES "GeneratorDomain::SolarSystem" OR NOT GENERATORS MATCHES "subspace.generator-request.v1" OR NOT GENERATORS MATCHES "GeneratorDomain::Character")
    message(FATAL_ERROR "PASS1048-1097 runtime generator parity registry is incomplete")
endif()
if(NOT BLENDER MATCHES "SSY_OT_export_generator_request" OR NOT BLENDER MATCHES "GENERATOR_DOMAIN_ITEMS" OR NOT BLENDER MATCHES "subspace.generator-request.v1")
    message(FATAL_ERROR "PASS1048-1097 Blender addon does not expose canonical generator parity requests")
endif()
if(NOT GENERATOR_CONTRACT MATCHES "Blender must not fork procedural generation math" OR NOT UI_CONTRACT MATCHES "SubspaceDockSystem" OR NOT AUTHORING_CONTRACT MATCHES "1 world unit is 1 meter")
    message(FATAL_ERROR "PASS1048-1097 architecture contracts are missing parity/scale/UI normalization rules")
endif()

message(STATUS "Pass1048-1097 authoring/runtime/UI/Blender parity source gate PASS")
