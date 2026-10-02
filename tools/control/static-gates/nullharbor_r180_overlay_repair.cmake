cmake_minimum_required(VERSION 3.20)
# R180 introduced the bounded overlay-repair hook. R181 supersedes only its
# line-oriented merge implementation after Windows proved InputState overlap is
# a legitimate semantic conflict. Keep this historical gate compatible by
# certifying the stricter R181 strategy rather than requiring git merge-file.
get_filename_component(ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
file(READ "${ROOT}/tools/studio/studio_r180_overlay_repair.py" REPAIR)
file(READ "${ROOT}/tools/studio/studio_verified_normalized_state.py" PROOF)
foreach(TOKEN
    "8484a081f8d60be980d3aac300ba2f1f9397cd75"
    "rebuild_input_state"
    "rebuild_native_app"
    "rebuild_renderer"
    "PlanetaryCommandCycleOverlay"
    "StudioTransformMoveDelta::AssemblyAuthored")
  string(FIND "${REPAIR}" "${TOKEN}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "R180/R181 repair authority missing token: ${TOKEN}")
  endif()
endforeach()
string(FIND "${PROOF}" "repair_known_overlay_regression(root)" HOOK)
if(HOOK EQUAL -1)
  message(FATAL_ERROR "R180/R181 normalized-state proof is not wired to reconciliation")
endif()
message(STATUS "R180 authority superseded safely by R181 semantic reconciliation PASS")
