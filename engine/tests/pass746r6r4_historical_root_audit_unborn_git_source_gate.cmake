if(NOT DEFINED ROOT)
  message(FATAL_ERROR "Pass746R6R4 source gate requires -DROOT=<engine-root>")
endif()

get_filename_component(PROJECT_ROOT "${ROOT}/.." ABSOLUTE)
set(HISTORICAL "${ROOT}/tests/pass735_744_shipyard_dev_pcc_source_gate.cmake")
set(ROOT_WRAPPER "${PROJECT_ROOT}/scripts/subspace_root_cleanliness_audit.ps1")
set(ROOT_AUTHORITY "${PROJECT_ROOT}/tools/control/ProjectOpsRootAudit.ps1")
set(COMMON "${PROJECT_ROOT}/tools/control/ProjectOpsCommon.psm1")
set(COMPAT "${PROJECT_ROOT}/tools/control/ControlCenterCommon.ps1")
set(QG "${PROJECT_ROOT}/tools/control/WriteQualityGateRecord.ps1")
set(SCC "${PROJECT_ROOT}/tools/control/SubspaceControlCenter.ps1")
set(PROJECT_CONTROL "${PROJECT_ROOT}/project.control.json")

foreach(PATH IN ITEMS "${HISTORICAL}" "${ROOT_WRAPPER}" "${ROOT_AUTHORITY}" "${COMMON}" "${COMPAT}" "${QG}" "${SCC}" "${PROJECT_CONTROL}")
  if(NOT EXISTS "${PATH}")
    message(FATAL_ERROR "Pass746R6R4 missing file: ${PATH}")
  endif()
endforeach()

file(READ "${HISTORICAL}" HISTORICAL_TEXT)
file(READ "${ROOT_WRAPPER}" WRAPPER_TEXT)
file(READ "${ROOT_AUTHORITY}" ROOT_AUTH_TEXT)
file(READ "${COMMON}" COMMON_TEXT)
file(READ "${COMPAT}" COMPAT_TEXT)
file(READ "${QG}" QG_TEXT)
file(READ "${SCC}" SCC_TEXT)
file(READ "${PROJECT_CONTROL}" PROJECT_TEXT)

function(require_text TEXT_VALUE NEEDLE)
  string(FIND "${TEXT_VALUE}" "${NEEDLE}" FOUND_AT)
  if(FOUND_AT EQUAL -1)
    message(FATAL_ERROR "Pass746R6R4 source gate missing: ${NEEDLE}")
  endif()
endfunction()

require_text("${WRAPPER_TEXT}" "ProjectOpsRootAudit.ps1")
require_text("${ROOT_AUTH_TEXT}" "projectOps.rootPolicy")
require_text("${ROOT_AUTH_TEXT}" "forbiddenGeneratedRoots")
require_text("${PROJECT_TEXT}" "\"rootPolicy\"")
require_text("${HISTORICAL_TEXT}" "delegated root audit wrapper")
require_text("${HISTORICAL_TEXT}" "ProjectOps root policy authority")

# Pass746R6R9: R6R7 moved Git probes behind System.Diagnostics.Process so
# ordinary native stderr cannot become PowerShell ErrorRecords. Validate the
# behavior/authority boundary instead of the retired direct-command text.
require_text("${COMMON_TEXT}" "function Invoke-ProjectOpsGitProbe")
require_text("${COMMON_TEXT}" "System.Diagnostics.ProcessStartInfo")
require_text("${COMMON_TEXT}" "$psi.PSObject.Properties['ArgumentList']")
require_text("${COMMON_TEXT}" "$psi.Arguments = ($encodedArguments -join ' ')")
require_text("${COMMON_TEXT}" "RedirectStandardOutput = $true")
require_text("${COMMON_TEXT}" "RedirectStandardError = $true")
require_text("${COMMON_TEXT}" "function Get-ProjectOpsGitRepositoryState")
require_text("${COMMON_TEXT}" "@('rev-parse','--verify','HEAD')")
require_text("${COMMON_TEXT}" "if ($headProbe.Success)")
require_text("${COMMON_TEXT}" "Get-ProjectOpsGitRepositoryState,Get-ProjectOpsCertifiableGitStatusLines")
require_text("${COMPAT_TEXT}" "function Get-ProjectGitRepositoryState")
require_text("${QG_TEXT}" "Get-ProjectGitRepositoryState -Root $Root")
require_text("${QG_TEXT}" "gitInitialized=[bool]$gitState.initialized")
require_text("${QG_TEXT}" "gitHasHead=[bool]$gitState.hasHead")
require_text("${SCC_TEXT}" "Get-ProjectGitRepositoryState -Root $Root")

# Historical text may no longer require the compatibility wrapper to contain a
# literal `.subspace` root-policy marker.
string(FIND "${HISTORICAL_TEXT}" "require_token(\"${ROOTAUDIT}\" \".subspace\" \"root audit\")" OLD_ROOT_ASSERT)
if(NOT OLD_ROOT_ASSERT EQUAL -1)
  message(FATAL_ERROR "Pass746R6R4 rejected stale historical root-audit literal assertion")
endif()

# Direct rev-parse HEAD is unsafe in an initialized repository before the first
# commit. All user-facing gate/audit code must use the shared ProjectOps probe.
string(FIND "${QG_TEXT}" "rev-parse HEAD" QG_DIRECT_HEAD)
string(FIND "${SCC_TEXT}" "rev-parse HEAD" SCC_DIRECT_HEAD)
if(NOT QG_DIRECT_HEAD EQUAL -1 OR NOT SCC_DIRECT_HEAD EQUAL -1)
  message(FATAL_ERROR "Pass746R6R4 rejected direct unborn-unsafe rev-parse HEAD probe")
endif()

# ProjectOps Git state must remain behind the process probe. A direct
# PowerShell-native rev-parse probe would reintroduce stderr/error-stream drift.
string(FIND "${COMMON_TEXT}" "& $git.Source rev-parse" DIRECT_NATIVE_REV_PARSE)
if(NOT DIRECT_NATIVE_REV_PARSE EQUAL -1)
  message(FATAL_ERROR "Pass746R6R4/R6R9 rejected direct PowerShell-native rev-parse probe")
endif()

message(STATUS "Pass746R6R4 historical root audit / unborn Git closure PASS")
