cmake_minimum_required(VERSION 3.20)
get_filename_component(ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
file(READ "${ROOT}/tools/control/static-gates/nullharbor_r183_planetary_materialization_authority.cmake" R183_GATE)
file(READ "${ROOT}/tools/control/static-gates/nullharbor_r186_recovery_descendant_compatibility.cmake" R186_GATE)

function(r187_require TEXT_VAR TOKEN LABEL)
  string(FIND "${${TEXT_VAR}}" "${TOKEN}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "R187 missing ${LABEL}: ${TOKEN}")
  endif()
endfunction()
function(r187_forbid TEXT_VAR TOKEN LABEL)
  string(FIND "${${TEXT_VAR}}" "${TOKEN}" POS)
  if(NOT POS EQUAL -1)
    message(FATAL_ERROR "R187 stale ${LABEL} remains: ${TOKEN}")
  endif()
endfunction()

# R183 must certify durable semantic owners, not private implementation names
# from the pre-R186 recovery helper.
r187_require(R183_GATE "test_r158_r177_planetary_command_source.py" "semantic Planetary source verifier")
r187_require(R183_GATE "nullharbor_r179_planetary_command_convergence.cmake" "R179 semantic authority")
r187_require(R183_GATE "nullharbor_r184_pcc_planetary_orchestration.cmake" "R184 PCC authority")
r187_require(R183_GATE "canonical-semantic-verification" "verification-only receipt contract")
r187_forbid(R183_GATE "PLANETARY_AUTHORITIES" "R183 private Python-variable dependency")
r187_forbid(R183_GATE "PLANETARY_MARKERS" "R183 private Python-variable dependency")
r187_forbid(R183_GATE "validate_planetary_authorities" "R183 removed helper dependency")
r187_forbid(R183_GATE "write_planetary_materialization_markers" "R183 removed helper dependency")

# R186 remains the authority that permits governed descendants after the one-time
# R182 recovery. R187 must not reintroduce exact recovered-file freezing.
r187_require(R186_GATE "current tree remains a semantically valid descendant" "R186 descendant policy")
r187_require(R186_GATE "recorded afterSha256 remains provenance" "R186 provenance policy")

message(STATUS "R187 R183 semantic-gate supersession / R186 descendant compatibility PASS")
