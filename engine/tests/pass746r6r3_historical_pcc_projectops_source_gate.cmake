if(NOT DEFINED ROOT)
  message(FATAL_ERROR "Pass746R6R3 source gate requires -DROOT=<engine-root>")
endif()

get_filename_component(PROJECT_ROOT "${ROOT}/.." ABSOLUTE)
set(HISTORICAL "${ROOT}/tests/pass735_744_shipyard_dev_pcc_source_gate.cmake")
set(TOOLS "${PROJECT_ROOT}/SubspaceTools.ps1")
set(COMMON "${PROJECT_ROOT}/tools/control/ProjectOpsCommon.psm1")
set(PROJECT_CONTROL "${PROJECT_ROOT}/project.control.json")

foreach(PATH IN ITEMS "${HISTORICAL}" "${TOOLS}" "${COMMON}" "${PROJECT_CONTROL}")
  if(NOT EXISTS "${PATH}")
    message(FATAL_ERROR "Pass746R6R3 missing file: ${PATH}")
  endif()
endforeach()

file(READ "${HISTORICAL}" HISTORICAL_TEXT)
file(READ "${TOOLS}" TOOLS_TEXT)
file(READ "${COMMON}" COMMON_TEXT)
file(READ "${PROJECT_CONTROL}" PROJECT_TEXT)

function(require_text TEXT_VALUE NEEDLE)
  string(FIND "${TEXT_VALUE}" "${NEEDLE}" FOUND_AT)
  if(FOUND_AT EQUAL -1)
    message(FATAL_ERROR "Pass746R6R3 source gate missing: ${NEEDLE}")
  endif()
endfunction()

require_text("${TOOLS_TEXT}" "Invoke-ProjectOpsGovernedTreeStage")
require_text("${COMMON_TEXT}" "function Invoke-ProjectOpsGovernedTreeStage")
require_text("${COMMON_TEXT}" "function Test-ProjectOpsGeneratedRelativePath")
foreach(TOKEN ".subspace\\" "updates\\" "artifacts\\" "dist\\" "logs\\")
  require_text("${COMMON_TEXT}" "${TOKEN}")
endforeach()
require_text("${PROJECT_TEXT}" "\"sourceAuthority\"")
require_text("${PROJECT_TEXT}" "\"independentOfGitIgnore\": true")
require_text("${HISTORICAL_TEXT}" "delegated governed source rollup")
require_text("${HISTORICAL_TEXT}" "ProjectOps generated-state exclusion")

# The old Pass735-744 gate may never force source-rollup policy back into the
# interactive SubspaceTools host.
string(FIND "${HISTORICAL_TEXT}" "(Join-Path $Global:SubspaceRoot \".subspace\")" OLD_LITERAL)
if(NOT OLD_LITERAL EQUAL -1)
  message(FATAL_ERROR "Pass746R6R3 rejected obsolete in-host .subspace source-rollup assertion")
endif()

message(STATUS "Pass746R6R3 historical PCC gate / ProjectOps authority closure PASS")
