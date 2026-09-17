cmake_minimum_required(VERSION 3.20)
# PASS1508R4: every gate shares a scope; never write ROOT or generic variables.
get_filename_component(PASS1508R4_PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
set(PASS1508R4_RENDER "${PASS1508R4_PROJECT_ROOT}/engine/src/application/NativeBattlefieldRenderer.cpp")
set(PASS1508R4_SHELL "${PASS1508R4_PROJECT_ROOT}/engine/src/editor/EditorDccShellLayoutSystem.cpp")
set(PASS1508R4_TEST "${PASS1508R4_PROJECT_ROOT}/engine/tests/pass1508r4_shell_geometry_tests.cpp")
foreach(PASS1508R4_PATH IN ITEMS "${PASS1508R4_RENDER}" "${PASS1508R4_SHELL}" "${PASS1508R4_TEST}")
  if(NOT EXISTS "${PASS1508R4_PATH}")
    message(FATAL_ERROR "PASS1508R4 missing: ${PASS1508R4_PATH}")
  endif()
endforeach()
file(READ "${PASS1508R4_RENDER}" PASS1508R4_RENDER_TEXT)
string(FIND "${PASS1508R4_RENDER_TEXT}" "void DrawStandaloneShipyardBackdrop(" PASS1508R4_START)
string(FIND "${PASS1508R4_RENDER_TEXT}" "void DrawVectorTravelBackdrop(" PASS1508R4_END)
if(PASS1508R4_START EQUAL -1 OR PASS1508R4_END LESS_EQUAL PASS1508R4_START)
  message(FATAL_ERROR "PASS1508R4 cannot isolate Shipyard backdrop")
endif()
math(EXPR PASS1508R4_LENGTH "${PASS1508R4_END}-${PASS1508R4_START}")
string(SUBSTRING "${PASS1508R4_RENDER_TEXT}" ${PASS1508R4_START} ${PASS1508R4_LENGTH} PASS1508R4_BACKDROP)
foreach(PASS1508R4_TOKEN IN ITEMS
    "ShipyardBuilderSystem::Layout(*frame.shipBuilder,frame.viewportWidth,frame.viewportHeight)"
    "const float left=layout.viewportLeft"
    "const float right=layout.viewportRight"
    "const float bottom=layout.viewportBottom")
  string(FIND "${PASS1508R4_BACKDROP}" "${PASS1508R4_TOKEN}" PASS1508R4_POSITION)
  if(PASS1508R4_POSITION EQUAL -1)
    message(FATAL_ERROR "PASS1508R4 backdrop must use live viewport: ${PASS1508R4_TOKEN}")
  endif()
endforeach()
foreach(PASS1508R4_FORBIDDEN IN ITEMS
    "shelfVisible?layout.assetShelfY"
    "layout.assetShelfY-2.0f"
    "sidebarVisible?layout.viewportRight")
  string(FIND "${PASS1508R4_BACKDROP}" "${PASS1508R4_FORBIDDEN}" PASS1508R4_POSITION)
  if(NOT PASS1508R4_POSITION EQUAL -1)
    message(FATAL_ERROR "PASS1508R4 obsolete backdrop geometry: ${PASS1508R4_FORBIDDEN}")
  endif()
endforeach()
file(READ "${PASS1508R4_SHELL}" PASS1508R4_SHELL_TEXT)
foreach(PASS1508R4_TOKEN IN ITEMS
    "toolW, contentBottom - contentTop"
    "out.assetShelf = {out.viewport.x, shelfY"
    "out.viewport.width,")
  string(FIND "${PASS1508R4_SHELL_TEXT}" "${PASS1508R4_TOKEN}" PASS1508R4_POSITION)
  if(PASS1508R4_POSITION EQUAL -1)
    message(FATAL_ERROR "PASS1508R4 model-less shell geometry drift: ${PASS1508R4_TOKEN}")
  endif()
endforeach()
string(FIND "${PASS1508R4_RENDER_TEXT}" "// PASS1508R4: draw the Assets header AFTER the floating-panel backing." PASS1508R4_HEADER)
string(FIND "${PASS1508R4_RENDER_TEXT}" "// The ship's screen-space frame is a viewport effect." PASS1508R4_FRAME)
if(PASS1508R4_HEADER LESS_EQUAL PASS1508R4_FRAME)
  message(FATAL_ERROR "PASS1508R4 Assets header painted before floating-panel backing")
endif()
message(STATUS "PASS1508R4 live-backdrop/shell-region source guard PASS")
