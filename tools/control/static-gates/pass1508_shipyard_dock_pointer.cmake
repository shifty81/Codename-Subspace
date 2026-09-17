cmake_minimum_required(VERSION 3.20)
# PASS1508: pointer routing must be live in BOTH standalone and in-game Shipyard.
# PASS1508R1: never overwrite the certification runner's ROOT variable. Every
# static gate is included in the same CMake scope; ROOT is the ENGINE root.
get_filename_component(PASS1508_PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
foreach(FILE IN ITEMS
  "engine/include/ship_editor/ShipyardDockPointerSystem.h"
  "engine/src/application/NativeGameApplication.cpp"
  "engine/include/application/NativeGameApplication.h"
  "engine/src/ship_editor/ShipyardProfessionalVisibleCutover.cpp"
  "engine/CMakeLists.txt"
  "engine/tests/pass1508_dock_pointer_tests.cpp")
  if(NOT EXISTS "${PASS1508_PROJECT_ROOT}/${FILE}")
    message(FATAL_ERROR "PASS1508 missing ${FILE}")
  endif()
endforeach()
file(READ "${PASS1508_PROJECT_ROOT}/engine/src/application/NativeGameApplication.cpp" APP)
foreach(TOKEN IN ITEMS
  "_shipyardDockPointer.Begin"
  "_shipyardDockPointer.Drag"
  "_shipyardDockPointer.End"
  "_shipyardDockSuppressClick=false")
  string(FIND "${APP}" "${TOKEN}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "PASS1508 missing live application wiring: ${TOKEN}")
  endif()
endforeach()
file(READ "${PASS1508_PROJECT_ROOT}/engine/src/ship_editor/ShipyardProfessionalVisibleCutover.cpp" CUTOVER)
string(FIND "${CUTOVER}" "phantom hit targets" POS)
if(POS EQUAL -1)
  message(FATAL_ERROR "PASS1508 inactive-tab controls are not gated by materialized layout")
endif()
file(READ "${PASS1508_PROJECT_ROOT}/engine/CMakeLists.txt" CMAKE_CONTENT)
string(FIND "${CMAKE_CONTENT}" "SubspacePass1508DockPointerTests" POS)
if(POS EQUAL -1)
  message(FATAL_ERROR "PASS1508 native test not registered with CTest")
endif()
message(STATUS "PASS1508 pointer routing source gate PASS")
