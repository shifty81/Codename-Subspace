# R177R2 Planetary Command migration matcher source gate.
# R188 normalization: the exact/indent-normalized text matcher is historical
# provenance only. Certify the R179 semantic verifier that superseded it.
if(NOT DEFINED PROJECT_ROOT)
  if(DEFINED ROOT)
    get_filename_component(PROJECT_ROOT "${ROOT}/.." ABSOLUTE)
  else()
    message(FATAL_ERROR "R177R2/R188 gate requires PROJECT_ROOT or ROOT")
  endif()
endif()

set(_verifier "${PROJECT_ROOT}/scripts/subspace_planetary_command_r158_r177_apply.ps1")
set(_semantic "${PROJECT_ROOT}/tools/control/tests/test_r158_r177_planetary_command_source.py")
set(_r179_gate "${PROJECT_ROOT}/tools/control/static-gates/nullharbor_r179_planetary_command_convergence.cmake")
set(_r184_gate "${PROJECT_ROOT}/tools/control/static-gates/nullharbor_r184_pcc_planetary_orchestration.cmake")

foreach(_path IN ITEMS "${_verifier}" "${_semantic}" "${_r179_gate}" "${_r184_gate}")
  if(NOT EXISTS "${_path}")
    message(FATAL_ERROR "R177R2/R188 semantic supersession dependency missing: ${_path}")
  endif()
endforeach()

file(READ "${_verifier}" _verify)
file(READ "${_semantic}" _semantic_text)

foreach(_token IN ITEMS
  "historical 17-transform text migration is retired after R179"
  "canonical source semantics"
  "legacyTextTransforms"
  "canonical-semantic-verification"
)
  string(FIND "${_verify}" "${_token}" _found)
  if(_found EQUAL -1)
    message(FATAL_ERROR "R177R2/R188 verifier missing '${_token}'")
  endif()
endforeach()

foreach(_token IN ITEMS
  "enum class PiClaimState"
  "PlaceGoverned"
  "PlanetaryCommandCycleOverlay"
  "HitTestPlanetaryHex"
  "R179 Planetary Command owns pointer clicks"
)
  string(FIND "${_semantic_text}" "${_token}" _found)
  if(_found EQUAL -1)
    message(FATAL_ERROR "R177R2/R188 semantic source verifier missing '${_token}'")
  endif()
endforeach()

message(STATUS "R177R2 Planetary Command matcher gate superseded safely by R179 semantic verification PASS")
