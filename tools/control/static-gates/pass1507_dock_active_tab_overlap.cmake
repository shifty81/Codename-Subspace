cmake_minimum_required(VERSION 3.20)
# PASS1507: runtime regression is registered in engine/CMakeLists.txt.
get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
set(DOCK "${PROJECT_ROOT}/engine/src/ui/SubspaceUiFramework.cpp")
set(CMAKE_ROOT "${PROJECT_ROOT}/engine/CMakeLists.txt")
set(TEST_CPP "${PROJECT_ROOT}/engine/tests/pass1507_dock_active_tab_tests.cpp")
foreach(REQUIRED_FILE IN ITEMS "${DOCK}" "${CMAKE_ROOT}" "${TEST_CPP}")
  if(NOT EXISTS "${REQUIRED_FILE}")
    message(FATAL_ERROR "PASS1507 missing source/test: ${REQUIRED_FILE}")
  endif()
endforeach()
file(READ "${DOCK}" DOCK_SOURCE)
foreach(TOKEN IN ITEMS
    "PASS1507: a leaf is a tab stack"
    "if(panelId!=active)continue;"
    "node.activeTabId.clear();"
    "node.activeTabId=candidate;")
  string(FIND "${DOCK_SOURCE}" "${TOKEN}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "PASS1507 dock repair missing: ${TOKEN}")
  endif()
endforeach()
file(READ "${CMAKE_ROOT}" ENGINE_CMAKE)
foreach(TOKEN IN ITEMS
    "add_executable(subspace_pass1507_dock_tests"
    "SubspacePass1507DockActiveTabTests"
    "tests/pass1507_dock_active_tab_tests.cpp")
  string(FIND "${ENGINE_CMAKE}" "${TOKEN}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "PASS1507 native regression not wired into Full Gate: ${TOKEN}")
  endif()
endforeach()
message(STATUS "PASS1507 static source/CTest registration gate PASS")
