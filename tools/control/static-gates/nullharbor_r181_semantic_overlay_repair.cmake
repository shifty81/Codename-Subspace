cmake_minimum_required(VERSION 3.20)
get_filename_component(ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
file(READ "${ROOT}/tools/studio/studio_r180_overlay_repair.py" REPAIR)
file(READ "${ROOT}/tools/studio/studio_verified_normalized_state.py" PROOF)
foreach(TOKEN
    "8484a081f8d60be980d3aac300ba2f1f9397cd75"
    "certified-baseline-plus-semantic-r178-r179-deltas"
    "rebuild_input_state"
    "rebuild_native_app"
    "rebuild_renderer"
    "PlanetaryCommandCycleOverlay"
    "StudioTransformMoveDelta::AssemblyAuthored"
    "unknown working postimage"
    "partial R178/R179 overlay regression detected")
  string(FIND "${REPAIR}" "${TOKEN}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "R181 semantic repair authority missing token: ${TOKEN}")
  endif()
endforeach()
string(FIND "${REPAIR}" "git merge-file" OLD_MERGE)
if(NOT OLD_MERGE EQUAL -1)
  message(FATAL_ERROR "R181 must not depend on generic git merge-file conflict resolution")
endif()
string(FIND "${PROOF}" "repair_known_overlay_regression(root)" HOOK)
if(HOOK EQUAL -1)
  message(FATAL_ERROR "R181 normalized-state proof is not wired to semantic reconciliation")
endif()
message(STATUS "R181 certified-baseline semantic overlay reconciliation source gate PASS")
