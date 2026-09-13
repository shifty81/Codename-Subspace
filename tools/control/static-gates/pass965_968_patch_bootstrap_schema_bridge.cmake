set(PATCH_ENGINE "${PROJECT_ROOT}/tools/control/StandalonePatchEngine.ps1")
set(PCC "${PROJECT_ROOT}/tools/control/StandaloneProjectControlCenter.ps1")

foreach(REQUIRED_FILE IN ITEMS "${PATCH_ENGINE}" "${PCC}")
  if(NOT EXISTS "${REQUIRED_FILE}")
    message(FATAL_ERROR "PASS965-968 required file missing: ${REQUIRED_FILE}")
  endif()
endforeach()

file(READ "${PATCH_ENGINE}" PATCH_ENGINE_TEXT)
file(READ "${PCC}" PCC_TEXT)

foreach(REQUIRED_FRAGMENT IN ITEMS
    "function ManifestProp"
    "$baseline=ManifestProp $m 'baseline'"
    "ManifestProp $baseline 'gitCommit'"
    "PRECONDITION_CONFLICT")
  string(FIND "${PATCH_ENGINE_TEXT}" "${REQUIRED_FRAGMENT}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "PASS965-968 patch engine contract missing: ${REQUIRED_FRAGMENT}")
  endif()
endforeach()

foreach(REQUIRED_FRAGMENT IN ITEMS
    "[ERROR LOG]"
    "Get-Content -LiteralPath $latestPatchLog.FullName -Tail 12"
    "Press Enter to acknowledge this patch failure and continue")
  string(FIND "${PCC_TEXT}" "${REQUIRED_FRAGMENT}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "PASS965-968 startup failure UX missing: ${REQUIRED_FRAGMENT}")
  endif()
endforeach()

message(STATUS "PASS965-968 bootstrap schema bridge + persistent patch failure UX certified.")
