get_filename_component(PROJECT_ROOT "${ROOT}/.." ABSOLUTE)
set(HISTORICAL "${ROOT}/tests/pass746r6r4_historical_root_audit_unborn_git_source_gate.cmake")
set(COMMON "${PROJECT_ROOT}/tools/control/ProjectOpsCommon.psm1")
set(QG "${PROJECT_ROOT}/tools/control/WriteQualityGateRecord.ps1")
set(SCC "${PROJECT_ROOT}/tools/control/SubspaceControlCenter.ps1")

foreach(PATH IN ITEMS "${HISTORICAL}" "${COMMON}" "${QG}" "${SCC}")
  if(NOT EXISTS "${PATH}")
    message(FATAL_ERROR "R6R9 static gate missing file: ${PATH}")
  endif()
endforeach()

file(READ "${HISTORICAL}" HISTORICAL_TEXT)
file(READ "${COMMON}" COMMON_TEXT)
file(READ "${QG}" QG_TEXT)
file(READ "${SCC}" SCC_TEXT)

function(r6r9_require TEXT_VALUE NEEDLE)
  string(FIND "${TEXT_VALUE}" "${NEEDLE}" FOUND_AT)
  if(FOUND_AT EQUAL -1)
    message(FATAL_ERROR "R6R9 static gate missing: ${NEEDLE}")
  endif()
endfunction()

r6r9_require("${COMMON_TEXT}" "function Invoke-ProjectOpsGitProbe")
r6r9_require("${COMMON_TEXT}" "System.Diagnostics.ProcessStartInfo")
r6r9_require("${COMMON_TEXT}" "$psi.PSObject.Properties['ArgumentList']")
r6r9_require("${COMMON_TEXT}" "$psi.Arguments = ($encodedArguments -join ' ')")
r6r9_require("${COMMON_TEXT}" "RedirectStandardError = $true")
r6r9_require("${COMMON_TEXT}" "@('rev-parse','--verify','HEAD')")
r6r9_require("${COMMON_TEXT}" "if ($headProbe.Success)")
r6r9_require("${HISTORICAL_TEXT}" "Pass746R6R9: R6R7 moved Git probes behind System.Diagnostics.Process")
r6r9_require("${QG_TEXT}" "Get-ProjectGitRepositoryState -Root $Root")
r6r9_require("${SCC_TEXT}" "Get-ProjectGitRepositoryState -Root $Root")

# These exact historical requirements became false when R6R7 introduced the
# structured Git probe and must never return.
string(FIND "${HISTORICAL_TEXT}" "require_text(\"${COMMON_TEXT}\" \"rev-parse --verify HEAD\")" STALE_1)
string(FIND "${HISTORICAL_TEXT}" "initialized-but-unborn repository is valid project state" STALE_2)
if(NOT STALE_1 EQUAL -1 OR NOT STALE_2 EQUAL -1)
  message(FATAL_ERROR "R6R9 rejected stale R6R4 implementation-text assertion")
endif()

string(FIND "${COMMON_TEXT}" "& $git.Source rev-parse" DIRECT_NATIVE)
if(NOT DIRECT_NATIVE EQUAL -1)
  message(FATAL_ERROR "R6R9 rejected direct PowerShell-native rev-parse probe")
endif()

message(STATUS "Pass746R6R9 historical unborn-Git probe migration static gate PASS")
