cmake_minimum_required(VERSION 3.20)
get_filename_component(ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
file(READ "${ROOT}/tools/studio/studio_r180_overlay_repair.py" REPAIR)
file(READ "${ROOT}/tools/studio/studio_verified_normalized_state.py" PROOF)
foreach(TOKEN
    "R186 descendant policy"
    "current tree remains a semantically valid descendant"
    "recorded afterSha256 remains provenance"
    "validate_text(rel, path.read_bytes())"
    "validate_external_authorities(root)"
    "partial R178/R179 overlay regression detected"
    "certified-8484a08-baseline-plus-r178-r179-forward-port")
  string(FIND "${REPAIR}" "${TOKEN}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "R186 descendant-compatible recovery missing token: ${TOKEN}")
  endif()
endforeach()
# The old permanent exact-current-hash receipt lock is the regression R186 removes.
string(FIND "${REPAIR}" "hashlib.sha256(path.read_bytes()).hexdigest() != row.get(\"afterSha256\")" FROZEN)
if(NOT FROZEN EQUAL -1)
  message(FATAL_ERROR "R186 still freezes recovered files to the original R182 after-image hashes")
endif()
string(FIND "${PROOF}" "later governed descendants are accepted" PROOF_POLICY)
if(PROOF_POLICY EQUAL -1)
  message(FATAL_ERROR "R186 normalized-state proof does not document descendant-compatible recovery authority")
endif()
message(STATUS "R186 one-time recovery / governed-descendant compatibility gate PASS")
