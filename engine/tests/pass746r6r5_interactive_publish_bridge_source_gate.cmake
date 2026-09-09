if(NOT DEFINED ROOT)
  message(FATAL_ERROR "Pass746R6R5 source gate requires -DROOT=<engine-root>")
endif()

get_filename_component(PROJECT_ROOT "${ROOT}/.." ABSOLUTE)
set(TOOLS "${PROJECT_ROOT}/SubspaceTools.ps1")
set(SCC "${PROJECT_ROOT}/tools/control/SubspaceControlCenter.ps1")
set(NORMALIZE "${PROJECT_ROOT}/tools/control/NormalizeGitHubAuthority.ps1")

foreach(PATH IN ITEMS "${TOOLS}" "${SCC}" "${NORMALIZE}")
  if(NOT EXISTS "${PATH}")
    message(FATAL_ERROR "Pass746R6R5 missing file: ${PATH}")
  endif()
endforeach()

file(READ "${TOOLS}" TOOLS_TEXT)
file(READ "${SCC}" SCC_TEXT)
file(READ "${NORMALIZE}" NORMALIZE_TEXT)

function(require_text TEXT_VALUE NEEDLE)
  string(FIND "${TEXT_VALUE}" "${NEEDLE}" FOUND_AT)
  if(FOUND_AT EQUAL -1)
    message(FATAL_ERROR "Pass746R6R5 source gate missing: ${NEEDLE}")
  endif()
endfunction()

require_text("${TOOLS_TEXT}" "Type PUBLISH to archive the current remote main and replace it with the certified normalized GREEN source")
require_text("${TOOLS_TEXT}" "-Arguments @(\"-PublishConfirmation\",\"PUBLISH\")")
require_text("${TOOLS_TEXT}" "Publication cancelled. No GitHub changes were made.")
require_text("${SCC_TEXT}" "[string]$PublishConfirmation = ''")
require_text("${SCC_TEXT}" "'-PublishConfirmation',$Confirmation")
require_text("${SCC_TEXT}" "Invoke-RepositoryAuthority -Mode 'Publish' -Confirmation $PublishConfirmation")
require_text("${NORMALIZE_TEXT}" "[string]$PublishConfirmation=''")
require_text("${NORMALIZE_TEXT}" "Publish authorization missing. Use the interactive root Control Center")
require_text("${NORMALIZE_TEXT}" "Explicit publication authorization received")

# Lower repository layers must never attempt interactive Read-Host when invoked
# through the logged child-process pipeline.
string(FIND "${NORMALIZE_TEXT}" "Read-Host" NESTED_READ_HOST)
if(NOT NESTED_READ_HOST EQUAL -1)
  message(FATAL_ERROR "Pass746R6R5 rejected nested interactive Read-Host in repository authority layer")
endif()

message(STATUS "Pass746R6R5 interactive publish bridge source gate PASS")
