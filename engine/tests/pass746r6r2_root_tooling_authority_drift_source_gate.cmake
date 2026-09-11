if(NOT DEFINED ROOT)
  message(FATAL_ERROR "Pass746R6R2 source gate requires -DROOT=<engine-root>")
endif()

get_filename_component(PROJECT_ROOT "${ROOT}/.." ABSOLUTE)
set(NATIVE_GUARD "${PROJECT_ROOT}/scripts/subspace_native_runtime_guard.ps1")
set(SUPPLY "${PROJECT_ROOT}/scripts/subspace_supply_chain_gate.ps1")
set(CONTINUITY "${PROJECT_ROOT}/scripts/subspace_pass_continuity_audit.ps1")
set(INSTALLER "${PROJECT_ROOT}/scripts/install_subspace_root_utility.ps1")
set(COMPARE "${PROJECT_ROOT}/tools/control/CompareQualityGates.ps1")
set(PROJECT_CONTROL "${PROJECT_ROOT}/project.control.json")

foreach(PATH IN ITEMS "${NATIVE_GUARD}" "${SUPPLY}" "${CONTINUITY}" "${INSTALLER}" "${COMPARE}" "${PROJECT_CONTROL}")
  if(NOT EXISTS "${PATH}")
    message(FATAL_ERROR "Pass746R6R2 source gate missing file: ${PATH}")
  endif()
endforeach()

file(READ "${NATIVE_GUARD}" NATIVE_TEXT)
file(READ "${SUPPLY}" SUPPLY_TEXT)
file(READ "${CONTINUITY}" CONTINUITY_TEXT)
file(READ "${INSTALLER}" INSTALLER_TEXT)
file(READ "${COMPARE}" COMPARE_TEXT)
file(READ "${PROJECT_CONTROL}" PROJECT_TEXT)

function(require_text TEXT_VALUE NEEDLE)
  string(FIND "${TEXT_VALUE}" "${NEEDLE}" FOUND_AT)
  if(FOUND_AT EQUAL -1)
    message(FATAL_ERROR "Pass746R6R2 source gate missing: ${NEEDLE}")
  endif()
endfunction()

require_text("${NATIVE_TEXT}" "engine\\CMakeLists.txt")
require_text("${NATIVE_TEXT}" "engine\\src\\main.cpp")
# PASS_CLEANCLONE_RETIREMENT_LEDGER_GATE_NORMALIZATION_R2
# Historical Pass746 wording required the physical C# archive to remain.
# Current authority permits physical retirement while preserving the durable
# provenance ledger and still classifies any physical C# file that reappears.
require_text("${NATIVE_TEXT}" "PASS_CLEANCLONE_RETIREMENT_LEDGER_POLICY")
require_text("${NATIVE_TEXT}" "rows may legitimately outlive the deleted physical archive")
require_text("${NATIVE_TEXT}" "Legacy C# file is not classified in retirement manifest")
require_text("${SUPPLY_TEXT}" "artifacts\\gates\\certifications\\supply-chain")
require_text("${CONTINUITY_TEXT}" "artifacts\\gates\\certifications\\continuity")
require_text("${COMPARE_TEXT}" "artifacts\\gates\\quality")
require_text("${INSTALLER_TEXT}" "artifacts\\logs\\sessions")
require_text("${INSTALLER_TEXT}" "tools\\control\\ProjectOpsCommon.psm1")
require_text("${INSTALLER_TEXT}" "tools\\control\\ProjectOpsNormalizationAudit.ps1")
require_text("${INSTALLER_TEXT}" "tools\\control\\UniversalTreeStage.psm1")
require_text("${PROJECT_TEXT}" "\"buildAuthority\"")
require_text("${PROJECT_TEXT}" "\"cmake-native-cpp\"")
require_text("${PROJECT_TEXT}" "\"requiredForBuild\": false")

foreach(FORBIDDEN IN ITEMS
    "Missing active solution: AvorionLike.sln"
    "$sln = Join-Path $rootPath \"AvorionLike.sln\""
    "Retirement manifest/source count mismatch"
    "Retirement manifest references missing legacy C# file")
  string(FIND "${NATIVE_TEXT}" "${FORBIDDEN}" BAD_AT)
  if(NOT BAD_AT EQUAL -1)
    message(FATAL_ERROR "Pass746R6R2 rejected stale legacy-solution authority: ${FORBIDDEN}")
  endif()
endforeach()

string(FIND "${COMPARE_TEXT}" ".subspace\\quality-gates" OLD_QG)
if(NOT OLD_QG EQUAL -1)
  message(FATAL_ERROR "Pass746R6R2 rejected legacy quality-gate artifact location")
endif()

string(FIND "${INSTALLER_TEXT}" "\"logs\", \"logs\\sessions\"" OLD_LOG_ROOT)
if(NOT OLD_LOG_ROOT EQUAL -1)
  message(FATAL_ERROR "Pass746R6R2 rejected installer recreation of top-level logs")
endif()

message(STATUS "Pass746R6R2 root-tooling authority drift closure source gate PASS")
