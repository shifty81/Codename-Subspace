# Pass1339: standalone PCC retires provably superseded legacy ZIP root drops
# without weakening explicit-approval or fail-closed certification policy.
get_filename_component(P1339_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)

function(p1339_require_text rel token)
  set(path "${P1339_ROOT}/${rel}")
  if(NOT EXISTS "${path}")
    message(FATAL_ERROR "Pass1339 missing required file: ${rel}")
  endif()
  file(READ "${path}" text)
  string(FIND "${text}" "${token}" pos)
  if(pos EQUAL -1)
    message(FATAL_ERROR "Pass1339 token '${token}' missing from ${rel}")
  endif()
endfunction()

p1339_require_text("tools/control/StandaloneProjectControlCenter.ps1" "function PendingLegacyRootDrops")
p1339_require_text("tools/control/StandaloneProjectControlCenter.ps1" "function Archive-SupersededLegacyRootDrops")
p1339_require_text("tools/control/StandaloneProjectControlCenter.ps1" "updates\\superseded")
p1339_require_text("tools/control/StandaloneProjectControlCenter.ps1" "legacy ZIP handoff(s)")
p1339_require_text("tools/control/StandaloneProjectControlCenter.ps1" "Full Gate remains blocked")
p1339_require_text("tools/control/StandaloneProjectControlCenter.ps1" "Archive-SupersededLegacyRootDrops")
p1339_require_text("docs/CURRENT_STATUS.md" "Pass1339 / LEGACY-ROOTDROP-RETIREMENT")

p1339_require_text("tools/control/static-gates/pass1268_1292_visible_professional_shipyard_cutover.cmake" "build_identity::kShipyardUiProfile")
p1339_require_text("tools/control/static-gates/pass1336_historical_visible_gate_reconciliation.cmake" "build_identity::kShipyardUiProfile")
p1339_require_text("tools/control/static-gates/pass1337_cmake_historical_gate_literal_repair.cmake" "build_identity::kShipyardUiProfile")

message(STATUS "Pass1339 standalone PCC legacy root-drop retirement source authority PASS")
