cmake_minimum_required(VERSION 3.20)
# PASS1508R2: static guard for the exact visual regressions seen in the
# user-reported screenshot. Do not assign to ROOT: all gates share CMake scope.
get_filename_component(PASS1508R2_PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
set(PASS1508R2_RENDER "${PASS1508R2_PROJECT_ROOT}/engine/src/application/NativeBattlefieldRenderer.cpp")
set(PASS1508R2_UI "${PASS1508R2_PROJECT_ROOT}/engine/src/ship_editor/ShipyardProfessionalVisibleCutover.cpp")
set(PASS1508R2_POINTER "${PASS1508R2_PROJECT_ROOT}/engine/include/ship_editor/ShipyardDockPointerSystem.h")
set(PASS1508R2_APP "${PASS1508R2_PROJECT_ROOT}/engine/src/application/NativeGameApplication.cpp")
foreach(PASS1508R2_SOURCE IN ITEMS "${PASS1508R2_RENDER}" "${PASS1508R2_UI}" "${PASS1508R2_POINTER}" "${PASS1508R2_APP}")
  if(NOT EXISTS "${PASS1508R2_SOURCE}")
    message(FATAL_ERROR "PASS1508R2 missing source: ${PASS1508R2_SOURCE}")
  endif()
endforeach()
file(READ "${PASS1508R2_RENDER}" PASS1508R2_RENDER_TEXT)
file(READ "${PASS1508R2_UI}" PASS1508R2_UI_TEXT)
file(READ "${PASS1508R2_POINTER}" PASS1508R2_POINTER_TEXT)
file(READ "${PASS1508R2_APP}" PASS1508R2_APP_TEXT)
foreach(PASS1508R2_REQUIRED IN ITEMS
  "const float right=layout.propertiesX,rightW=layout.propertiesWidth"
  "class ShipyardPanelClip"
  "layout.outlinerX+layout.outlinerWidth,layout.outlinerY+panelHeaderH"
  "layout.propertiesX+layout.propertiesWidth,layout.propertiesY+panelHeaderH"
  "if(showProperties){"
  "propertiesClip(layout.propertiesX,layout.propertiesY"
  "const float validationTop=layout.propertiesY+layout.propertiesHeight"
  "if(!d.floating||!d.visible)continue")
  string(FIND "${PASS1508R2_RENDER_TEXT}" "${PASS1508R2_REQUIRED}" PASS1508R2_POS)
  if(PASS1508R2_POS EQUAL -1)
    message(FATAL_ERROR "PASS1508R2 renderer containment missing ${PASS1508R2_REQUIRED}")
  endif()
endforeach()
foreach(PASS1508R2_FORBIDDEN IN ITEMS
  "const float right=layout.outlinerX,rightW=layout.outlinerWidth"
  "layout.outlinerX,layout.statusY"
  "layout.outlinerX,layout.propertiesY,0,w,layout.propertiesY"
  "layout.outlinerY+panelHeaderH,0,w,layout.outlinerY+panelHeaderH"
  "layout.propertiesY+panelHeaderH,0,w,layout.propertiesY+panelHeaderH")
  string(FIND "${PASS1508R2_RENDER_TEXT}" "${PASS1508R2_FORBIDDEN}" PASS1508R2_POS)
  if(NOT PASS1508R2_POS EQUAL -1)
    message(FATAL_ERROR "PASS1508R2 global-coordinate rendering leak: ${PASS1508R2_FORBIDDEN}")
  endif()
endforeach()
foreach(PASS1508R2_REQUIRED IN ITEMS
  "const float rx=l.propertiesX+7*s,rw=l.propertiesWidth-14*s"
  "clipControls(outlinerControlStart"
  "clipControls(propertiesControlStart")
  string(FIND "${PASS1508R2_UI_TEXT}" "${PASS1508R2_REQUIRED}" PASS1508R2_POS)
  if(PASS1508R2_POS EQUAL -1)
    message(FATAL_ERROR "PASS1508R2 UI control geometry missing ${PASS1508R2_REQUIRED}")
  endif()
endforeach()
string(FIND "${PASS1508R2_POINTER_TEXT}" "CoversFloatingPanel" PASS1508R2_POS)
if(PASS1508R2_POS EQUAL -1)
  message(FATAL_ERROR "PASS1508R2 missing floating-panel occlusion")
endif()
string(FIND "${PASS1508R2_APP_TEXT}" "ShipyardDockPointerSystem::CoversFloatingPanel" PASS1508R2_POS)
if(PASS1508R2_POS EQUAL -1)
  message(FATAL_ERROR "PASS1508R2 floating-panel occlusion not connected to viewport input")
endif()
message(STATUS "PASS1508R2 floating-panel visual containment source gate PASS")
