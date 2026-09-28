cmake_minimum_required(VERSION 3.20)
get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
set(TOOLS "${PROJECT_ROOT}/SubspaceTools.ps1")
if(NOT EXISTS "${TOOLS}")
  message(FATAL_ERROR "PASS1510 missing SubspaceTools.ps1")
endif()
file(READ "${TOOLS}" text)
foreach(token IN ITEMS
  "function Invoke-StudioConstructSourceCutoverIfRequired"
  "Studio Construct source cutover"
  "apply_studio_r23_r32_normalization.py"
  "studio_r32_source_assertions.py"
  "PASS1509 requires the pending Studio Construct source cutover")
  string(FIND "${text}" "${token}" pos)
  if(pos EQUAL -1)
    message(FATAL_ERROR "PASS1510 missing mandatory pre-build migration token: ${token}")
  endif()
endforeach()

set(CONTINUITY "${PROJECT_ROOT}/scripts/subspace_pass_continuity_audit.ps1")
if(NOT EXISTS "${CONTINUITY}")
  message(FATAL_ERROR "PASS1510 missing pass continuity audit")
endif()
file(READ "${CONTINUITY}" continuity)
foreach(token IN ITEMS
  "Pending approved Studio Construct source cutover detected"
  "apply_studio_r23_r32_normalization.py"
  "studio_r32_source_assertions.py")
  string(FIND "${continuity}" "${token}" pos)
  if(pos EQUAL -1)
    message(FATAL_ERROR "PASS1510 continuity hook missing token: ${token}")
  endif()
endforeach()

message(STATUS "PASS1510 R32 pre-build migration orchestration PASS")
