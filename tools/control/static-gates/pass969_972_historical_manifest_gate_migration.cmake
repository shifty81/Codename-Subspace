# PASS969-972 - Historical ProjectOps gate migration certification.
# Historical gates must preserve behavioral invariants while following the
# current manifest schema instead of pinning the project to obsolete V2 tokens.

get_filename_component(PROJECT_ROOT "${ROOT}/.." ABSOLUTE)
set(GATE932 "${PROJECT_ROOT}/tools/control/static-gates/pass932_941_promotion_camera_stability.cmake")
set(GATE952 "${PROJECT_ROOT}/tools/control/static-gates/pass952_956_certified_manifest_promotion_stability.cmake")
set(COMMON "${PROJECT_ROOT}/tools/control/ProjectOpsCommon.psm1")
set(CONTROL "${PROJECT_ROOT}/tools/control/SubspaceControlCenter.ps1")

foreach(PATH IN ITEMS "${GATE932}" "${GATE952}" "${COMMON}" "${CONTROL}")
  if(NOT EXISTS "${PATH}")
    message(FATAL_ERROR "PASS969-972 required file missing: ${PATH}")
  endif()
endforeach()

file(READ "${GATE932}" GATE932_TEXT)
file(READ "${GATE952}" GATE952_TEXT)
file(READ "${COMMON}" COMMON_TEXT)
file(READ "${CONTROL}" CONTROL_TEXT)

foreach(GATE_TEXT IN ITEMS "${GATE932_TEXT}" "${GATE952_TEXT}")
  string(FIND "${GATE_TEXT}" "MANIFEST-V3" CURRENT_POS)
  if(CURRENT_POS EQUAL -1)
    message(FATAL_ERROR "PASS969-972 historical ProjectOps gate does not certify current V3 manifest authority")
  endif()
endforeach()

foreach(GATE_TEXT IN ITEMS "${GATE932_TEXT}" "${GATE952_TEXT}")
  string(FIND "${GATE_TEXT}" "retired" RETIRED_POLICY_POS)
  if(RETIRED_POLICY_POS EQUAL -1)
    message(FATAL_ERROR "PASS969-972 historical ProjectOps gate does not explicitly treat prior schema authority as retired")
  endif()
endforeach()

foreach(TOKEN IN ITEMS
    "---CERTIFIABLE-WORKTREE-MANIFEST-V3---"
    "schemaVersion = 3"
    "fingerprintScope = 'worktree-path-state-bytes-sha256'"
    "ordering = 'ordinal'"
    "System.StringComparer]::Ordinal")
  string(FIND "${COMMON_TEXT}" "${TOKEN}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "PASS969-972 current deterministic manifest authority missing: ${TOKEN}")
  endif()
endforeach()

foreach(TOKEN IN ITEMS
    "gitManifestVersion -lt 3"
    "deterministic certifiable worktree manifest v3"
    "Git HEAD changed after the GREEN gate")
  string(FIND "${CONTROL_TEXT}" "${TOKEN}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "PASS969-972 current promotion authority missing: ${TOKEN}")
  endif()
endforeach()

message(STATUS "PASS969-972 historical ProjectOps gates migrated to current deterministic manifest authority")
