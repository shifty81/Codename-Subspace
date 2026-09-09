file(READ "${ROOT}/src/application/NativeGameApplication.cpp" APP)
file(READ "${ROOT}/src/application/NativeBattlefieldRenderer.cpp" RENDERER)
file(READ "${ROOT}/src/rendering/StrategicCamera.cpp" CAMERA)
file(READ "${ROOT}/src/ship_editor/ShipyardBuilderSystem.cpp" BUILDER)
file(READ "${ROOT}/src/hangar/DockingExperienceSystem.cpp" DOCKING)
file(READ "${ROOT}/src/platform/NativeWindow.cpp" WINDOW)
file(READ "${ROOT}/include/input/InputState.h" INPUT)
file(READ "${ROOT}/src/ui/TacticalContactsSystem.cpp" CONTACTS)
file(READ "${ROOT}/include/ui/TacticalContactsSystem.h" CONTACTS_H)

foreach(REQUIRED
    "PanViewRelative"
    "ShipyardBuildSafetySystem::SuppressFlightAndWeapons"
    "FireControlSystem::Solve"
    "TacticalTargetingSystem::Request"
    "_tacticalContacts"
    "ObservableWarpSystem::EmitStageTransition")
  string(FIND "${APP}" "${REQUIRED}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass476-495 application integration missing: ${REQUIRED}")
  endif()
endforeach()

foreach(REQUIRED
    "DrawDockingGuidance"
    "DrawObservableWarpEvidence"
    "CONTACTS / "
    "SHIP COMMAND"
    "PlanetAtmospherePresentationSystem::Default"
    "PlanetWeatherSystem::Initialize"
    "GasGiantWeatherSystem::Build")
  string(FIND "${RENDERER}" "${REQUIRED}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass476-495 renderer integration missing: ${REQUIRED}")
  endif()
endforeach()

foreach(REQUIRED "ViewRightPlanar" "ViewUpPlanar" "PanViewRelative")
  string(FIND "${CAMERA}" "${REQUIRED}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass483 camera-relative Shipyard pan missing: ${REQUIRED}")
  endif()
endforeach()

foreach(REQUIRED "ShipyardTransformSystem" "ShipyardDragDropSystem" "ShipyardOrientationConstraintSystem")
  string(FIND "${BUILDER}" "${REQUIRED}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass484-487 Shipyard authoring integration missing: ${REQUIRED}")
  endif()
endforeach()

foreach(REQUIRED "StationDockingGeometrySystem" "s.geometry.captureWorld" "s.geometry.undockWorld")
  string(FIND "${DOCKING}" "${REQUIRED}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass476 physical docking integration missing: ${REQUIRED}")
  endif()
endforeach()


foreach(REQUIRED "EditorToolMove" "EditorToolRotate" "EditorFrameSelected" "EditorDeleteModule")
  string(FIND "${INPUT}" "${REQUIRED}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass483-485 keyboard Shipyard action missing: ${REQUIRED}")
  endif()
endforeach()
string(FIND "${WINDOW}" "EditorToolMove" EDITOR_KEY_POS)
if(EDITOR_KEY_POS EQUAL -1)
  message(FATAL_ERROR "Pass483-485 native keyboard mapping missing")
endif()
foreach(REQUIRED "HitTestRow" "HitTestPreset")
  string(FIND "${CONTACTS}" "${REQUIRED}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass495 interactive Contacts authority missing: ${REQUIRED}")
  endif()
endforeach()
string(FIND "${CONTACTS_H}" "sourceId" SOURCE_ID_POS)
if(SOURCE_ID_POS EQUAL -1)
  message(FATAL_ERROR "Pass495 Contacts source identity missing")
endif()

message(STATUS "Pass476-495 source gate passed")
