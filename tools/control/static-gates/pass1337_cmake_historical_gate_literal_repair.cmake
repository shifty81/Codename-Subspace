# Pass1337 - certify that Pass1336 no longer performs a CMake-expanded
# self-introspection search against a literal `${ROOT}` token.
get_filename_component(PROJECT_ROOT "${ROOT}/.." ABSOLUTE)
set(GATE1336 "${PROJECT_ROOT}/tools/control/static-gates/pass1336_historical_visible_gate_reconciliation.cmake")

if(NOT EXISTS "${GATE1336}")
  message(FATAL_ERROR "Pass1337 required Pass1336 gate missing: ${GATE1336}")
endif()

file(READ "${GATE1336}" GATE_TEXT)

foreach(TOKEN IN ITEMS
  "NativeBattlefieldRenderer.cpp"
  "OUTLINER / PROPERTIES"
  "SELECTED MODULE"
  "duplicate right-panel projection returned"
  "CMake expands")
  string(FIND "${GATE_TEXT}" "${TOKEN}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass1337 repaired Pass1336 contract missing: ${TOKEN}")
  endif()
endforeach()

# The fragile form below expands ROOT while evaluating this gate and therefore
# can never match the literal `${ROOT}` text in another CMake source file.
set(FRAGILE_PREFIX "file(READ \"")
string(FIND "${GATE_TEXT}" "${FRAGILE_PREFIX}${ROOT}/src/application/NativeBattlefieldRenderer.cpp\" RENDERER)" FRAGILE)
if(NOT FRAGILE EQUAL -1)
  message(FATAL_ERROR "Pass1337 detected the fragile expanded-ROOT self-search form")
endif()

message(STATUS "Pass1337 CMake historical-gate literal repair passed")
