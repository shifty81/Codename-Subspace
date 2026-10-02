cmake_minimum_required(VERSION 3.20)
get_filename_component(ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
file(READ "${ROOT}/tools/studio/studio_r180_overlay_repair.py" REPAIR)
file(READ "${ROOT}/tools/studio/studio_verified_normalized_state.py" PROOF)
foreach(TOKEN
    "8484a081f8d60be980d3aac300ba2f1f9397cd75"
    "certified-8484a08-baseline-plus-r178-r179-forward-port"
    "EXTERNAL_AUTHORITIES"
    "engine/src/studio/StudioApplication.cpp"
    "StudioTransformMoveDelta::AssemblyAuthored"
    "StudioTransformMoveDelta::ModelAuthored"
    "TranslateSelectedResolvedParent"
    "ShipyardOverlayLayoutStore::Load"
    "PlanetaryCommandCycleOverlay"
    "validate_external_authorities(root)"
    "unknown working postimage"
    "partial R178/R179 overlay regression detected")
  string(FIND "${REPAIR}" "${TOKEN}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "R182 certified recovery authority missing token: ${TOKEN}")
  endif()
endforeach()
# The false R181 ownership assumption must never return.
string(FIND "${REPAIR}" "\"engine/src/application/NativeGameApplication.cpp\": (\n        \"StudioTransformMoveDelta::AssemblyAuthored\"" FALSE_OWNER)
if(NOT FALSE_OWNER EQUAL -1)
  message(FATAL_ERROR "R182 still assigns standalone Studio transform authority to NativeGameApplication")
endif()
string(FIND "${PROOF}" "repair_known_overlay_regression(root)" HOOK)
if(HOOK EQUAL -1)
  message(FATAL_ERROR "R182 normalized-state proof is not wired to certified recovery")
endif()
message(STATUS "R182 certified-baseline recovery source gate PASS")
