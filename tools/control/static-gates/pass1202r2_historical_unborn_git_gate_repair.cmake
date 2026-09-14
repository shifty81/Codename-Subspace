# Pass1202R2 - repair R1 historical unborn-Git regression.
get_filename_component(PROJECT_ROOT "${ROOT}/.." ABSOLUTE)
set(CONTROL "${PROJECT_ROOT}/tools/control/SubspaceControlCenter.ps1")
if(NOT EXISTS "${CONTROL}")
  message(FATAL_ERROR "Pass1202R2 control file missing: ${CONTROL}")
endif()
file(READ "${CONTROL}" CONTROL_TEXT)

string(FIND "${CONTROL_TEXT}" "Get-ProjectGitRepositoryState -Root $Root" STATE_POS)
if(STATE_POS EQUAL -1)
  message(FATAL_ERROR "Pass1202R2 shared Git-state authority missing from push verification")
endif()

string(FIND "${CONTROL_TEXT}" "& $git rev-parse HEAD" DIRECT_HEAD_POS)
if(NOT DIRECT_HEAD_POS EQUAL -1)
  message(FATAL_ERROR "Pass1202R2 direct unborn-unsafe HEAD probe still present")
endif()

foreach(TOKEN IN ITEMS
  "[STEP] PUSH: origin/{0} <= {1}"
  "[STEP] REMOTE VERIFY: querying origin branch head."
  "git ls-remote --heads origin"
  "origin/{0} == local HEAD")
  string(FIND "${CONTROL_TEXT}" "${TOKEN}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass1202R2 R1 remote verification contract drifted: ${TOKEN}")
  endif()
endforeach()

message(STATUS "Pass1202R2 historical unborn-Git gate repair certified")
