get_filename_component(PROJECT_ROOT "${ROOT}/.." ABSOLUTE)
set(COMMON "${PROJECT_ROOT}/tools/control/ProjectOpsCommon.psm1")
set(PROJECT_CONTROL "${PROJECT_ROOT}/project.control.json")

foreach(PATH IN ITEMS "${COMMON}" "${PROJECT_CONTROL}")
  if(NOT EXISTS "${PATH}")
    message(FATAL_ERROR "R6R10 static gate missing file: ${PATH}")
  endif()
endforeach()

file(READ "${COMMON}" COMMON_TEXT)
file(READ "${PROJECT_CONTROL}" PROJECT_TEXT)

function(r6r10_require TEXT_VALUE NEEDLE)
  string(FIND "${TEXT_VALUE}" "${NEEDLE}" FOUND_AT)
  if(FOUND_AT EQUAL -1)
    message(FATAL_ERROR "R6R10 static gate missing: ${NEEDLE}")
  endif()
endfunction()

r6r10_require("${COMMON_TEXT}" "function ConvertTo-ProjectOpsProcessArgument")
r6r10_require("${COMMON_TEXT}" "$psi.PSObject.Properties['ArgumentList']")
r6r10_require("${COMMON_TEXT}" "$psi.Arguments = ($encodedArguments -join ' ')")
r6r10_require("${COMMON_TEXT}" "Windows PowerShell 5.1 / .NET Framework does not")
r6r10_require("${COMMON_TEXT}" "ArgumentMode = $argumentMode")
r6r10_require("${COMMON_TEXT}" "RedirectStandardOutput = $true")
r6r10_require("${COMMON_TEXT}" "RedirectStandardError = $true")
r6r10_require("${PROJECT_TEXT}" "\"powerShellCompatibility\"")
r6r10_require("${PROJECT_TEXT}" "\"Windows PowerShell 5.1\"")
r6r10_require("${PROJECT_TEXT}" "\"PowerShell 7+\"")

# ArgumentList may be used only behind the runtime capability check. The old
# unconditional loop was the exact Windows PowerShell 5.1 failure.
string(FIND "${COMMON_TEXT}" "foreach ($argument in $GitArgs) {\n        [void]$psi.ArgumentList.Add" UNCONDITIONAL_OLD)
if(NOT UNCONDITIONAL_OLD EQUAL -1)
  message(FATAL_ERROR "R6R10 rejected unconditional ProcessStartInfo.ArgumentList usage")
endif()

message(STATUS "Pass746R6R10 Windows PowerShell Git-probe compatibility static gate PASS")
