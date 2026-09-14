# Pass1202R1 - certified GREEN commit/push idempotency repair.
get_filename_component(PROJECT_ROOT "${ROOT}/.." ABSOLUTE)
set(ROOT_TOOLS "${PROJECT_ROOT}/SubspaceTools.ps1")
set(CONTROL "${PROJECT_ROOT}/tools/control/SubspaceControlCenter.ps1")
set(STANDALONE "${PROJECT_ROOT}/tools/control/StandaloneProjectControlCenter.ps1")
set(TEST_PCC "${PROJECT_ROOT}/tools/control/TestStandaloneProjectControlCenter.ps1")
foreach(P IN ITEMS "${ROOT_TOOLS}" "${CONTROL}" "${STANDALONE}" "${TEST_PCC}")
  if(NOT EXISTS "${P}")
    message(FATAL_ERROR "Pass1202R1 required PCC file missing: ${P}")
  endif()
endforeach()
file(READ "${ROOT_TOOLS}" ROOT_TEXT)
file(READ "${CONTROL}" CONTROL_TEXT)
file(READ "${STANDALONE}" STANDALONE_TEXT)
file(READ "${TEST_PCC}" TEST_TEXT)
foreach(TOKEN IN ITEMS
  "[STEP] COMMIT: verify or create the exact certified GREEN commit."
  "[STEP] PUSH + REMOTE VERIFY: publish current branch and verify origin matches local HEAD."
  "[PASS] REMOTE VERIFY: certified GREEN source is committed and present on origin.")
  string(FIND "${ROOT_TEXT}" "${TOKEN}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass1202R1 root option-2 stage contract missing: ${TOKEN}")
  endif()
endforeach()
foreach(TOKEN IN ITEMS
  "current HEAD is already the exact certified GREEN commit created from this gate"
  "Get-ProjectOpsCertifiableGitChangePaths -Root $Root"
  "git ls-remote --heads origin"
  "REMOTE VERIFY proves origin already contains this exact HEAD"
  "origin/{0} == local HEAD")
  string(FIND "${CONTROL_TEXT}" "${TOKEN}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass1202R1 idempotent commit/push authority missing: ${TOKEN}")
  endif()
endforeach()
foreach(TOKEN IN ITEMS
  "[STEP] COMMIT: verify or create the exact certified GREEN commit."
  "[STEP] PUSH + REMOTE VERIFY: publish current branch and verify origin matches local HEAD."
  "[PASS] REMOTE VERIFY: certified GREEN source is committed and present on origin.")
  string(FIND "${STANDALONE_TEXT}" "${TOKEN}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass1202R1 standalone PCC reporting contract missing: ${TOKEN}")
  endif()
endforeach()
string(FIND "${TEST_TEXT}" "PCC contract: $s" TEST_POS)
if(TEST_POS EQUAL -1)
  message(FATAL_ERROR "Pass1202R1 standalone PCC test contract missing")
endif()
message(STATUS "Pass1202R1 PCC commit/push idempotency source gate passed")
