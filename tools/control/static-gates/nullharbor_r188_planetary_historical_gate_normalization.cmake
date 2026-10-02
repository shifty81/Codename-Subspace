cmake_minimum_required(VERSION 3.20)
get_filename_component(ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)

set(P1598 "${ROOT}/tools/control/static-gates/pass1598_planetary_command_prebuild_repair.cmake")
set(P1599 "${ROOT}/tools/control/static-gates/pass1599_planetary_command_migration_matcher.cmake")
set(R184 "${ROOT}/tools/control/static-gates/nullharbor_r184_pcc_planetary_orchestration.cmake")

foreach(P IN ITEMS "${P1598}" "${P1599}" "${R184}")
  if(NOT EXISTS "${P}")
    message(FATAL_ERROR "R188 missing certification dependency: ${P}")
  endif()
endforeach()

file(READ "${P1598}" A)
file(READ "${P1599}" B)

function(r188_require TEXT_VAR TOKEN LABEL)
  string(FIND "${${TEXT_VAR}}" "${TOKEN}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "R188 missing ${LABEL}: ${TOKEN}")
  endif()
endfunction()

r188_require(A "Invoke-PlanetaryCommandCanonicalVerification" "R184 canonical verification function")
r188_require(A "prebuild gate superseded safely by R184 canonical verification" "R177R1 supersession result")
r188_require(B "test_r158_r177_planetary_command_source.py" "R179 semantic verifier")
r188_require(B "nullharbor_r179_planetary_command_convergence.cmake" "R179 convergence authority")
r188_require(B "nullharbor_r184_pcc_planetary_orchestration.cmake" "R184 PCC authority")
r188_require(B "matcher gate superseded safely by R179 semantic verification" "R177R2 supersession result")

message(STATUS "R188 historical Planetary prebuild/matcher static-gate normalization PASS")
