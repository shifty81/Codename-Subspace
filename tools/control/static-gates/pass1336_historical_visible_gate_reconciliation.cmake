# Pass1336 - reconcile the historical Pass1268-1292 visible-shell gate with
# Pass1335's single-chrome ownership and Pass1338's Blender-derived split
# OUTLINER/PROPERTIES authority. This prevents stale historical assertions from
# requiring duplicated or retired composite panel controls.
get_filename_component(PROJECT_ROOT "${ROOT}/.." ABSOLUTE)
set(HISTORICAL "${PROJECT_ROOT}/tools/control/static-gates/pass1268_1292_visible_professional_shipyard_cutover.cmake")

if(NOT EXISTS "${HISTORICAL}")
  message(FATAL_ERROR "Pass1336 historical visible gate missing: ${HISTORICAL}")
endif()

file(READ "${HISTORICAL}" GATE)

# Do not search for the full `file(READ "${ROOT}/...")` line here. CMake expands
# ${ROOT} inside quoted foreach items before string(FIND), while the file text
# correctly contains the literal source token `${ROOT}`. Use stable semantic
# substrings instead so certification checks intent rather than CMake expansion.
foreach(TOKEN IN ITEMS
  "NativeBattlefieldRenderer.cpp"
  "OUTLINER"
  "PROPERTIES"
  "build_identity::kShipyardUiProfile"
  "duplicate right-panel projection returned")
  string(FIND "${GATE}" "${TOKEN}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass1336 historical-gate reconciliation missing: ${TOKEN}")
  endif()
endforeach()

# The historical gate may mention these strings only as forbidden duplicate
# projections; it must no longer require them in the CUTOVER required-token list.
string(FIND "${GATE}" "visible shell missing OUTLINER / SHIP HIERARCHY" BAD_OUTLINER)
string(FIND "${GATE}" "visible shell missing PROPERTIES / INSTANCE + DEFINITION" BAD_PROPERTIES)
if(NOT BAD_OUTLINER EQUAL -1 OR NOT BAD_PROPERTIES EQUAL -1)
  message(FATAL_ERROR "Pass1336 stale Pass1268 required duplicate panel headers")
endif()

message(STATUS "Pass1336 historical visible-gate reconciliation passed")
