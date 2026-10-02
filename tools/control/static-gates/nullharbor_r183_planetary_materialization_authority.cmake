cmake_minimum_required(VERSION 3.20)
get_filename_component(ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)

set(LEGACY_ENTRY_PATH "${ROOT}/scripts/subspace_planetary_command_r158_r177_apply.ps1")
set(VERIFY_TEST_PATH "${ROOT}/tools/control/tests/test_r158_r177_planetary_command_source.py")
set(R179_GATE_PATH "${ROOT}/tools/control/static-gates/nullharbor_r179_planetary_command_convergence.cmake")
set(R184_GATE_PATH "${ROOT}/tools/control/static-gates/nullharbor_r184_pcc_planetary_orchestration.cmake")

foreach(REQUIRED_FILE IN ITEMS
    "${LEGACY_ENTRY_PATH}"
    "${VERIFY_TEST_PATH}"
    "${R179_GATE_PATH}"
    "${R184_GATE_PATH}")
  if(NOT EXISTS "${REQUIRED_FILE}")
    message(FATAL_ERROR "R183 semantic materialization authority missing required file: ${REQUIRED_FILE}")
  endif()
endforeach()

file(READ "${LEGACY_ENTRY_PATH}" LEGACY_ENTRY)
file(READ "${VERIFY_TEST_PATH}" VERIFY_TEST)
file(READ "${R179_GATE_PATH}" R179_GATE)
file(READ "${R184_GATE_PATH}" R184_GATE)

function(r183_require TEXT_VAR TOKEN LABEL)
  string(FIND "${${TEXT_VAR}}" "${TOKEN}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "R183 semantic materialization authority missing ${LABEL}: ${TOKEN}")
  endif()
endfunction()

function(r183_forbid TEXT_VAR TOKEN LABEL)
  string(FIND "${${TEXT_VAR}}" "${TOKEN}" POS)
  if(NOT POS EQUAL -1)
    message(FATAL_ERROR "R183 retired ${LABEL} returned: ${TOKEN}")
  endif()
endfunction()

# R183's durable contract is the verification-only compatibility entrypoint and
# its materialization receipt. Internal Python variable names are not authority.
r183_require(LEGACY_ENTRY "historical 17-transform text migration is retired after R179" "replay-retirement contract")
r183_require(LEGACY_ENTRY "canonical-semantic-verification" "semantic receipt mode")
r183_require(LEGACY_ENTRY "legacyTextTransforms" "retired-transform receipt field")
r183_require(LEGACY_ENTRY "R158_R177_PLANETARY_COMMAND_MATERIALIZED.json" "materialization receipt")
r183_require(LEGACY_ENTRY "test_r158_r177_planetary_command_source.py" "canonical source verifier invocation")
r183_require(LEGACY_ENTRY "PlanetaryCommandLayout" "current command layout authority")
r183_require(LEGACY_ENTRY "PlanetaryCommandCycleOverlay" "current overlay input authority")

# The semantic verifier protects the actual R179 public/gameplay contract.
r183_require(VERIFY_TEST "R158-R177 -> R179 Planetary Command source verification PASS" "semantic verifier success contract")
r183_require(VERIFY_TEST "PiSectorIdentity" "stable sector identity verification")
r183_require(VERIFY_TEST "PlaceGoverned" "governed placement verification")
r183_require(VERIFY_TEST "PlanetaryCommandCycleOverlay" "append-only input verification")

# R179 and R184 remain the canonical source/PCC owners around this historical shim.
r183_require(R179_GATE "historical R158-R177 text" "R179 migration-retirement lineage")
r183_require(R179_GATE "PlaceGoverned" "R179 governed placement authority")
r183_require(R184_GATE "Invoke-PlanetaryCommandCanonicalVerification" "R184 PCC verification hook")
r183_require(R184_GATE "expected one certified preimage" "R184 stale-preimage prohibition")

# Never regress to the removed text-transform machinery.
r183_forbid(LEGACY_ENTRY "apply_planetary_command_r158_r177.py" "Python text-transform replay")
r183_forbid(LEGACY_ENTRY "expected one certified preimage" "certified-preimage matcher")
r183_forbid(LEGACY_ENTRY "indentation-normalized" "whitespace-normalized matcher")

message(STATUS "R183 Planetary Command historical materializer retirement gate PASS (R187 semantic supersession)")
