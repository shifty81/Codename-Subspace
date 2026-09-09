if(NOT DEFINED ROOT)
  message(FATAL_ERROR "Pass746R2 repository authority source gate requires -DROOT=<engine-root>")
endif()

get_filename_component(REPO "${ROOT}/.." ABSOLUTE)

set(README "${REPO}/README.md")
set(GITIGNORE "${REPO}/.gitignore")
set(PROJECT_CONTROL "${REPO}/project.control.json")
set(ROOT_TOOLS "${REPO}/SubspaceTools.ps1")
set(REPO_TOOL "${REPO}/tools/control/NormalizeGitHubAuthority.ps1")
set(CONTROL_CENTER "${REPO}/tools/control/SubspaceControlCenter.ps1")
set(AUTHORITY_DOC "${REPO}/docs/REPOSITORY_AUTHORITY.md")

foreach(PATH_TO_CHECK IN ITEMS
    "${README}" "${GITIGNORE}" "${PROJECT_CONTROL}" "${ROOT_TOOLS}" "${REPO_TOOL}" "${CONTROL_CENTER}" "${AUTHORITY_DOC}")
  if(NOT EXISTS "${PATH_TO_CHECK}")
    message(FATAL_ERROR "Pass746R2 repository authority file missing: ${PATH_TO_CHECK}")
  endif()
endforeach()

file(READ "${README}" README_TEXT)
file(READ "${GITIGNORE}" GITIGNORE_TEXT)
file(READ "${PROJECT_CONTROL}" PROJECT_CONTROL_TEXT)
file(READ "${ROOT_TOOLS}" ROOT_TOOLS_TEXT)
file(READ "${REPO_TOOL}" REPO_TOOL_TEXT)
file(READ "${CONTROL_CENTER}" CONTROL_CENTER_TEXT)
file(READ "${AUTHORITY_DOC}" AUTHORITY_TEXT)

function(require_text TEXT_VALUE NEEDLE)
  string(FIND "${TEXT_VALUE}" "${NEEDLE}" FOUND_AT)
  if(FOUND_AT EQUAL -1)
    message(FATAL_ERROR "Pass746R2 repository authority source gate missing: ${NEEDLE}")
  endif()
endfunction()

function(forbid_text TEXT_VALUE NEEDLE)
  string(FIND "${TEXT_VALUE}" "${NEEDLE}" FOUND_AT)
  if(NOT FOUND_AT EQUAL -1)
    message(FATAL_ERROR "Pass746R2 repository authority source gate rejected stale text: ${NEEDLE}")
  endif()
endfunction()

require_text("${README_TEXT}" "# Codename Subspace")
require_text("${README_TEXT}" "native C++ only")
require_text("${README_TEXT}" "Prepare normalized GitHub authority")
forbid_text("${README_TEXT}" "Your project description here")
forbid_text("${README_TEXT}" "dotnet run")
forbid_text("${README_TEXT}" "C# Prototype")

require_text("${GITIGNORE_TEXT}" "/AvorionLike/")
require_text("${PROJECT_CONTROL_TEXT}" "\"authorityPolicy\": \"green-gate-normalized\"")
require_text("${PROJECT_CONTROL_TEXT}" "\"key\": \"repo.publish\"")

require_text("${ROOT_TOOLS_TEXT}" "repo-authority-audit")
require_text("${ROOT_TOOLS_TEXT}" "repo-authority-prepare")
require_text("${ROOT_TOOLS_TEXT}" "repo-authority-publish")
require_text("${ROOT_TOOLS_TEXT}" "Join-Path $Global:SubspaceRoot \"AvorionLike\"")

require_text("${REPO_TOOL_TEXT}" "archive/pre-native-normalization-")
require_text("${REPO_TOOL_TEXT}" "--force-with-lease=refs/heads/main:")
require_text("${REPO_TOOL_TEXT}" "Get-CertifiableGitFingerprint")
# Pass746R6R6: publication confirmation moved to the interactive root host.
# The lower repository authority is intentionally non-interactive.
require_text("${ROOT_TOOLS_TEXT}" "Type PUBLISH to archive the current remote main and replace it with the certified normalized GREEN source")
require_text("${ROOT_TOOLS_TEXT}" "-PublishConfirmation")
require_text("${CONTROL_CENTER_TEXT}" "PublishConfirmation")
require_text("${REPO_TOOL_TEXT}" "Publish authorization missing. Use the interactive root Control Center")
forbid_text("${REPO_TOOL_TEXT}" "Read-Host")
require_text("${REPO_TOOL_TEXT}" "No GitHub changes were made")
require_text("${AUTHORITY_TEXT}" "Runtime authority: native C++ only")

message(STATUS "Pass746R2 repository authority source gate PASS")
