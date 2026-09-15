cmake_minimum_required(VERSION 3.20)
# Pass1442: reconcile retained PCC history markers after cumulative source overlays.
get_filename_component(P1442_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
set(status "${P1442_ROOT}/docs/CURRENT_STATUS.md")
if(NOT EXISTS "${status}")
  message(FATAL_ERROR "PASS1442 missing docs/CURRENT_STATUS.md")
endif()
file(READ "${status}" status_text)
foreach(token IN ITEMS
  "Pass1339 / LEGACY-ROOTDROP-RETIREMENT"
  "Pass1339R1 / PENDING-NAME-STRICTMODE-FIX"
  "Pass1339R2 / SOURCE-ARTIFACT-HANDOFF-CLASSIFICATION"
  "Pass1440 — single-hull standard ships / multi-hull capitals"
  "Pass1441 / PCC-POST-ROLLUP-RECOVERY"
  "Pass1442 / PCC-STATUS-AUTHORITY-RECONCILIATION")
  string(FIND "${status_text}" "${token}" pos)
  if(pos EQUAL -1)
    message(FATAL_ERROR "PASS1442 status authority missing token: ${token}")
  endif()
endforeach()
message(STATUS "PASS1442 PCC status authority reconciliation PASS")
