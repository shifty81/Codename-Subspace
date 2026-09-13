# Pass957-960: project-owned PCC and Forge/Rust-Forge portable intake + certified commit policy.
get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
file(READ "${PROJECT_ROOT}/project.control.json" CONTRACT_TEXT)
file(READ "${PROJECT_ROOT}/tools/control/StandaloneProjectControlCenter.ps1" PCC_TEXT)
file(READ "${PROJECT_ROOT}/tools/control/StandalonePatchEngine.ps1" PATCH_TEXT)
file(READ "${PROJECT_ROOT}/tools/control/SubspaceControlCenter.ps1" SCC_TEXT)
file(READ "${PROJECT_ROOT}/tools/control/ProjectOpsCommon.psm1" COMMON_TEXT)
file(READ "${PROJECT_ROOT}/SubspaceTools.ps1" ROOT_TOOLS_TEXT)

foreach(REQUIRED IN ITEMS
  "\"patchSchema\": \"forge.patch.v1\""
  "\"startupScan\": true"
  "\"explicitApprovalRequired\": true"
  "\"autoApply\": false"
  "\"fullGateAutoApply\": false"
  "\"targetResolution\": \"manifest-identity-before-applicability\""
  "\"portablePolicyId\": \"forge.project-update-policy.v1\""
)
  string(FIND "${CONTRACT_TEXT}" "${REQUIRED}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass957-960 project update contract missing: ${REQUIRED}")
  endif()
endforeach()

foreach(REQUIRED IN ITEMS
  "Assert-StartupIntakePolicy"
  "Apply this patch now? [Y/N]"
  "RequireNoPendingPatch"
  "Full Gate never auto-applies"
)
  string(FIND "${PCC_TEXT}" "${REQUIRED}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass957-960 standalone PCC behavior missing: ${REQUIRED}")
  endif()
endforeach()


foreach(REQUIRED IN ITEMS
  "Portable Forge policy: discovery/queueing never mutates source"
  "Full Gate never auto-applies patches"
  "Pending-update certification guard before full gate"
)
  string(FIND "${ROOT_TOOLS_TEXT}" "${REQUIRED}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass957-960 root ProjectOps patch policy missing: ${REQUIRED}")
  endif()
endforeach()

foreach(REQUIRED IN ITEMS
  "TARGET_MISMATCH"
  "PRECONDITION_CONFLICT"
  "forge.patch.v1"
  "targetProjectId"
)
  string(FIND "${PATCH_TEXT}" "${REQUIRED}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass957-960 patch identity behavior missing: ${REQUIRED}")
  endif()
endforeach()

foreach(REQUIRED IN ITEMS
  "Get-ProjectOpsCertifiableGitChangePaths"
  "Invoke-ProjectOpsStageCertifiableGitChanges"
  "Generated/runtime paths are staged for a certified commit"
)
  string(FIND "${COMMON_TEXT}" "${REQUIRED}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass957-960 shared staging authority missing: ${REQUIRED}")
  endif()
endforeach()

string(FIND "${SCC_TEXT}" "Invoke-ProjectOpsStageCertifiableGitChanges -Root $Root" STAGE_POS)
if(STAGE_POS EQUAL -1)
  message(FATAL_ERROR "Pass957-960 Subspace certified commit is not using shared source-only staging")
endif()
string(FIND "${SCC_TEXT}" "git add -A -- . ':(exclude)" STALE_ADD)
if(NOT STALE_ADD EQUAL -1)
  message(FATAL_ERROR "Pass957-960 stale root-wide git add pathspec remains")
endif()

message(STATUS "Pass957-960 internal PCC / Forge portable policy certification PASS")
