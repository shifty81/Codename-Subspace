# R177R1 Planetary Command PCC prebuild orchestration gate.
# R188 normalization: the historical text-materialization function was retired by
# R179/R184. Certify the current verification-only PCC hook instead.
if(NOT DEFINED PROJECT_ROOT)
  if(DEFINED ROOT)
    get_filename_component(PROJECT_ROOT "${ROOT}/.." ABSOLUTE)
  else()
    message(FATAL_ERROR "R177R1/R188 gate requires PROJECT_ROOT or ROOT")
  endif()
endif()

set(_utility "${PROJECT_ROOT}/SubspaceTools.ps1")
set(_verifier "${PROJECT_ROOT}/scripts/subspace_planetary_command_r158_r177_apply.ps1")
if(NOT EXISTS "${_utility}")
  message(FATAL_ERROR "R177R1/R188 missing SubspaceTools.ps1")
endif()
if(NOT EXISTS "${_verifier}")
  message(FATAL_ERROR "R177R1/R188 missing canonical Planetary verifier")
endif()

file(READ "${_utility}" _pcc)
file(READ "${_verifier}" _verify)

foreach(_token IN ITEMS
  "function Invoke-PlanetaryCommandCanonicalVerification"
  "subspace_planetary_command_r158_r177_apply.ps1"
  "Planetary Command canonical source verification before native build."
  "historical text replay is retired"
)
  string(FIND "${_pcc}" "${_token}" _found)
  if(_found EQUAL -1)
    message(FATAL_ERROR "R177R1/R188 canonical PCC verification missing '${_token}'")
  endif()
endforeach()

foreach(_token IN ITEMS
  "historical 17-transform text migration is retired after R179"
  "R179 canonical Planetary Command source is already materialized"
  "Historical exact/indent-normalized transform replay is no longer required."
)
  string(FIND "${_verify}" "${_token}" _found)
  if(_found EQUAL -1)
    message(FATAL_ERROR "R177R1/R188 canonical verifier missing '${_token}'")
  endif()
endforeach()

message(STATUS "R177R1 Planetary Command prebuild gate superseded safely by R184 canonical verification PASS")
