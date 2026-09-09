file(READ "${ROOT}/src/application/NativeGameApplication.cpp" APP)
file(READ "${ROOT}/src/application/NativeBattlefieldRenderer.cpp" RENDERER)
file(READ "${ROOT}/src/ships/ShipyardAuthoredShipSystem.cpp" AUTHORED)

foreach(REQUIRED
    "HitTestSystemMapNode"
    "systemMapTarget"
    "SystemMapNodeKind::Moon")
  string(FIND "${APP}" "${REQUIRED}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass474 application authority missing: ${REQUIRED}")
  endif()
endforeach()

foreach(REQUIRED
    "MAP LEGEND"
    "SELECTED OBJECT"
    "LMB SELECT   RMB ACTIONS"
    "CloudRotationPhase"
    "DrawSystemMapMarker")
  string(FIND "${RENDERER}" "${REQUIRED}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass474 renderer authority missing: ${REQUIRED}")
  endif()
endforeach()

string(FIND "${AUTHORED}" "forwardVisualYawDegrees=180.0f" YAW_POS)
if(YAW_POS EQUAL -1)
  message(FATAL_ERROR "Pass474 authored reference ships do not preserve forward normalization")
endif()
message(STATUS "Pass474 source gate passed")
