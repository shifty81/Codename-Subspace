# Pass952-956 historical intent: GREEN promotion compares one complete
# certifiable worktree manifest rather than Git staging buckets or dirty-status
# lists. PASS969-972 migrates this gate from retired manifest V2 to current V3.

get_filename_component(PROJECT_ROOT "${ROOT}/.." ABSOLUTE)
set(COMMON "${PROJECT_ROOT}/tools/control/ProjectOpsCommon.psm1")
set(WRITER "${PROJECT_ROOT}/tools/control/WriteQualityGateRecord.ps1")
set(CONTROL "${PROJECT_ROOT}/tools/control/SubspaceControlCenter.ps1")

foreach(PATH IN ITEMS "${COMMON}" "${WRITER}" "${CONTROL}")
  if(NOT EXISTS "${PATH}")
    message(FATAL_ERROR "Pass952-956 required file missing: ${PATH}")
  endif()
endforeach()

file(READ "${COMMON}" COMMON_TEXT)
file(READ "${WRITER}" WRITER_TEXT)
file(READ "${CONTROL}" CONTROL_TEXT)

foreach(TOKEN IN ITEMS
    "function Get-ProjectOpsCertifiableGitManifest"
    "ls-tree','-r','--name-only','HEAD"
    "ls-files','--cached','--others','--exclude-standard"
    "CERTIFIABLE-WORKTREE-MANIFEST-V3"
    "fingerprintScope = 'worktree-path-state-bytes-sha256'"
    "ordering = 'ordinal'"
)
  string(FIND "${COMMON_TEXT}" "${TOKEN}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass952-956 ProjectOps manifest token missing: ${TOKEN}")
  endif()
endforeach()

string(FIND "${COMMON_TEXT}" "CERTIFIABLE-WORKTREE-MANIFEST-V2" RETIRED_V2)
if(NOT RETIRED_V2 EQUAL -1)
  message(FATAL_ERROR "Pass952-956 retired V2 manifest authority remains in ProjectOpsCommon")
endif()

foreach(TOKEN IN ITEMS
    "gitManifestVersion"
    "gitManifestPathCount"
    "gitManifest=$gitManifestEntries"
    "sourceManifest=@($sourceSnapshot.files)"
    "gitHead"
)
  string(FIND "${WRITER_TEXT}" "${TOKEN}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass952-956 quality-gate manifest token missing: ${TOKEN}")
  endif()
endforeach()

foreach(TOKEN IN ITEMS
    "GREEN marker predates deterministic certifiable worktree manifest v3"
    "Get-ManifestDriftRows"
    "Actual governed-source drift"
    "Actual post-GREEN Git worktree drift"
    "Staging changed the certifiable worktree manifest"
    "Git HEAD changed after the GREEN gate"
)
  string(FIND "${CONTROL_TEXT}" "${TOKEN}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass952-956 promotion token missing: ${TOKEN}")
  endif()
endforeach()

message(STATUS "Pass952-956 certified worktree manifest promotion authority PASS on current V3 schema")
