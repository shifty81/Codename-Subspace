# Pass1334 - standalone PCC safe branch switching controls.
get_filename_component(PROJECT_ROOT "${ROOT}/.." ABSOLUTE)
set(PCC "${PROJECT_ROOT}/tools/control/StandaloneProjectControlCenter.ps1")
set(BRANCH_MANAGER "${PROJECT_ROOT}/tools/control/GitBranchManager.ps1")
set(TEST_PCC "${PROJECT_ROOT}/tools/control/TestStandaloneProjectControlCenter.ps1")

foreach(P IN ITEMS "${PCC}" "${BRANCH_MANAGER}" "${TEST_PCC}")
  if(NOT EXISTS "${P}")
    message(FATAL_ERROR "Pass1334 required PCC branch-control file missing: ${P}")
  endif()
endforeach()

file(READ "${PCC}" PCC_TEXT)
file(READ "${BRANCH_MANAGER}" BRANCH_TEXT)
file(READ "${TEST_PCC}" TEST_TEXT)

foreach(TOKEN IN ITEMS
  "Branches / switch / create / toggle"
  "GitBranchManager.ps1"
  "RequireBranchGateFresh"
  "branch-switch-state.json"
  "BranchControl")
  string(FIND "${PCC_TEXT}" "${TOKEN}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass1334 standalone PCC branch surface missing: ${TOKEN}")
  endif()
endforeach()

foreach(TOKEN IN ITEMS
  "SAFE BRANCH MANAGER"
  "Assert-WorkingTreeClean"
  "Assert-NoPendingPatch"
  "Switch-RemoteTracking"
  "Create-NewBranch"
  "Toggle-PreviousBranch"
  "last-green-quality-gate.json"
  "branch-switch-state.json"
  "No auto-stash, reset, checkout-force, branch deletion, merge, rebase, or force push.")
  string(FIND "${BRANCH_TEXT}" "${TOKEN}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass1334 branch safety contract missing: ${TOKEN}")
  endif()
endforeach()

foreach(FORBIDDEN IN ITEMS "checkout -f" "reset --hard" "push --force")
  string(FIND "${BRANCH_TEXT}" "${FORBIDDEN}" POS)
  if(NOT POS EQUAL -1)
    message(FATAL_ERROR "Pass1334 branch manager contains forbidden destructive command: ${FORBIDDEN}")
  endif()
endforeach()

foreach(TOKEN IN ITEMS
  "branch contract:"
  "branch manager forbids force checkout/reset"
  "GitBranchManager.ps1")
  string(FIND "${TEST_TEXT}" "${TOKEN}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass1334 PCC self-test coverage missing: ${TOKEN}")
  endif()
endforeach()

message(STATUS "Pass1334 safe branch-switch PCC source gate passed")
