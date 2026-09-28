# R82R8: source-level guard against cross-gate CMake ROOT variable leakage.
# Use this gate's own location, not the mutable ROOT variable supplied by
# ProjectOpsStaticCertification's include loop.
get_filename_component(R82R8_REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
set(R82R8_RUNNER "${R82R8_REPO_ROOT}/tools/control/ProjectOpsStaticCertification.cmake")
if(NOT EXISTS "${R82R8_RUNNER}")
  message(FATAL_ERROR "R82R8 missing ProjectOps static certification runner: ${R82R8_RUNNER}")
endif()
file(READ "${R82R8_RUNNER}" R82R8_RUNNER_TEXT)
foreach(R82R8_REQUIRED IN ITEMS
  "get_filename_component(_PROJECTOPS_CERT_ENGINE_ROOT"
  "get_filename_component(_PROJECTOPS_CERT_REPO_ROOT"
  "set(ROOT \"\${_PROJECTOPS_CERT_ENGINE_ROOT}\")"
  "set(PROJECT_ROOT \"\${_PROJECTOPS_CERT_REPO_ROOT}\")"
  "include(\"\${GATE_PATH}\")"
  "file(GLOB STATIC_GATES"
  "message(STATUS \"ProjectOps static certification PASS\")")
  string(FIND "${R82R8_RUNNER_TEXT}" "${R82R8_REQUIRED}" R82R8_AT)
  if(R82R8_AT EQUAL -1)
    message(FATAL_ERROR "R82R8 runner root-isolation contract missing: ${R82R8_REQUIRED}")
  endif()
endforeach()
message(STATUS "R82R8 ProjectOps per-gate root-scope contract PASS")
