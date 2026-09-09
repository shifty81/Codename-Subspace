if(NOT DEFINED APP_SOURCE OR NOT EXISTS "${APP_SOURCE}")
  message(FATAL_ERROR "Pass470 APP_SOURCE missing")
endif()
if(NOT DEFINED RENDERER_SOURCE OR NOT EXISTS "${RENDERER_SOURCE}")
  message(FATAL_ERROR "Pass470 RENDERER_SOURCE missing")
endif()
file(READ "${APP_SOURCE}" APP_TEXT)
file(READ "${RENDERER_SOURCE}" RENDERER_TEXT)

foreach(REQUIRED
    "_window.BeginFrame(0.003f, 0.007f, 0.012f, 1.0f)"
    "camera.SetZoomLimits(0.12f,96.0f)"
    "FrameShipyardView(true)")
  string(FIND "${APP_TEXT}" "${REQUIRED}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass470 app contract missing: ${REQUIRED}")
  endif()
endforeach()
# Pass655 semantic successor: the old fixed StrategicCamera zoom/bootstrap and
# wheel-zoom expression were replaced by the normalized 6DOF construction
# camera. Preserve Pass470's actual invariant: standalone Shipyard opens
# framed, supports close inspection, and wheel input changes inspection depth.
string(FIND "${APP_TEXT}" "ConstructionEditorCameraSystem::Reset" RESET_POS)
string(FIND "${APP_TEXT}" "camera.SetEditorView" EDITOR_VIEW_POS)
string(FIND "${APP_TEXT}" "ConstructionEditorCameraSystem::Dolly" DOLLY_POS)
if(RESET_POS EQUAL -1 OR EDITOR_VIEW_POS EQUAL -1 OR DOLLY_POS EQUAL -1)
  message(FATAL_ERROR "Pass470 semantic successor missing normalized construction-camera frame/dolly authority")
endif()

foreach(REQUIRED
    "DrawStandaloneShipyardBackdrop"
    "if(!compressedCruise&&!frame.standaloneShipyard)"
    "WHEEL HULL-CLOSE ZOOM")
  string(FIND "${RENDERER_TEXT}" "${REQUIRED}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass470 renderer contract missing: ${REQUIRED}")
  endif()
endforeach()

message(STATUS "Pass470 standalone Shipyard viewport source gate passed")
