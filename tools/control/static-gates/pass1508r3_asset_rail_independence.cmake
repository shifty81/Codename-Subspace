cmake_minimum_required(VERSION 3.20)
# This gate is included into the common ProjectOps certification scope.
# Do not overwrite ROOT or other shared variable names.
get_filename_component(PASS1508R3_PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
set(PASS1508R3_WORKSPACE "${PASS1508R3_PROJECT_ROOT}/engine/src/ship_editor/ShipyardWorkspaceSystem.cpp")
set(PASS1508R3_LAYOUT "${PASS1508R3_PROJECT_ROOT}/engine/src/ship_editor/ShipyardProfessionalVisibleCutover.cpp")
set(PASS1508R3_RENDER "${PASS1508R3_PROJECT_ROOT}/engine/src/application/NativeBattlefieldRenderer.cpp")
set(PASS1508R3_TEST "${PASS1508R3_PROJECT_ROOT}/engine/tests/pass1508r3_dock_region_tests.cpp")
foreach(PASS1508R3_FILE IN ITEMS "${PASS1508R3_WORKSPACE}" "${PASS1508R3_LAYOUT}" "${PASS1508R3_RENDER}" "${PASS1508R3_TEST}")
  if(NOT EXISTS "${PASS1508R3_FILE}")
    message(FATAL_ERROR "PASS1508R3 source missing: ${PASS1508R3_FILE}")
  endif()
endforeach()
file(READ "${PASS1508R3_WORKSPACE}" PASS1508R3_W)
foreach(PASS1508R3_TOKEN IN ITEMS
  [=["root",true,SubspaceDockSplitAxis::Horizontal,.035f,"tool_left","content"]=]
  [=["content",true,SubspaceDockSplitAxis::Vertical,.78f,"upper","bottom"]=])
  string(FIND "${PASS1508R3_W}" "${PASS1508R3_TOKEN}" PASS1508R3_POS)
  if(PASS1508R3_POS EQUAL -1)
    message(FATAL_ERROR "PASS1508R3 expected independent dock topology: ${PASS1508R3_TOKEN}")
  endif()
endforeach()
file(READ "${PASS1508R3_LAYOUT}" PASS1508R3_L)
file(READ "${PASS1508R3_RENDER}" PASS1508R3_R)
foreach(PASS1508R3_TOKEN IN ITEMS
  "l.toolRailHeight=d->rect.height"
  "l.toolRailHeight=0")
  string(FIND "${PASS1508R3_L}" "${PASS1508R3_TOKEN}" PASS1508R3_POS)
  if(PASS1508R3_POS EQUAL -1)
    message(FATAL_ERROR "PASS1508R3 missing independent rail dimensions: ${PASS1508R3_TOKEN}")
  endif()
endforeach()
foreach(PASS1508R3_TOKEN IN ITEMS
  "const float viewportBottom=maximized?layout.statusY-2.0f:layout.viewportBottom"
  "layout.toolRailWidth,layout.toolRailHeight"
  "layout.toolRailY+layout.toolRailHeight")
  string(FIND "${PASS1508R3_R}" "${PASS1508R3_TOKEN}" PASS1508R3_POS)
  if(PASS1508R3_POS EQUAL -1)
    message(FATAL_ERROR "PASS1508R3 missing independent UI geometry: ${PASS1508R3_TOKEN}")
  endif()
endforeach()
foreach(PASS1508R3_FORBIDDEN IN ITEMS
  "viewportBottom-layout.toolRailY"
  "showAssetBrowser?layout.assetShelfY-2.0f")
  string(FIND "${PASS1508R3_R}" "${PASS1508R3_FORBIDDEN}" PASS1508R3_POS)
  if(NOT PASS1508R3_POS EQUAL -1)
    message(FATAL_ERROR "PASS1508R3 legacy coupling remains: ${PASS1508R3_FORBIDDEN}")
  endif()
endforeach()
message(STATUS "PASS1508R3 asset shelf / tool rail independence PASS")
