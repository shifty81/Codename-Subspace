# R179 Planetary Command convergence gate. The historical R158-R177 text
# migration is retired; current source must expose one semantic planetary
# command authority that coexists with R178's append-only input/control model.
get_filename_component(R179_PROJECT_ROOT "${ROOT}/.." ABSOLUTE)

function(r179_require FILE TOKEN MESSAGE_TEXT)
  if(NOT EXISTS "${FILE}")
    message(FATAL_ERROR "R179 missing required file: ${FILE}")
  endif()
  file(READ "${FILE}" R179_TEXT)
  string(FIND "${R179_TEXT}" "${TOKEN}" R179_INDEX)
  if(R179_INDEX EQUAL -1)
    message(FATAL_ERROR "R179 ${MESSAGE_TEXT}: missing token '${TOKEN}' in ${FILE}")
  endif()
endfunction()

set(R179_PI_H "${ROOT}/include/economy/PlanetaryIndustrySystem.h")
set(R179_PI_CPP "${ROOT}/src/economy/PlanetaryIndustrySystem.cpp")
set(R179_INPUT "${ROOT}/include/input/InputState.h")
set(R179_WINDOW "${ROOT}/src/platform/NativeWindow.cpp")
set(R179_BINDINGS "${ROOT}/src/input/InputBindingProfile.cpp")
set(R179_FACE_H "${ROOT}/include/integration/PlayerFacingIntegrationSystem.h")
set(R179_FACE_CPP "${ROOT}/src/integration/PlayerFacingIntegrationSystem.cpp")
set(R179_APP "${ROOT}/src/application/NativeGameApplication.cpp")
set(R179_RENDER "${ROOT}/src/application/NativeBattlefieldRenderer.cpp")
set(R179_WORKSPACE "${ROOT}/src/ui/SandboxWorkspaceSystem.cpp")
set(R179_MIGRATION "${R179_PROJECT_ROOT}/scripts/subspace_planetary_command_r158_r177_apply.ps1")
set(R179_VERIFY "${R179_PROJECT_ROOT}/tools/control/tests/test_r158_r177_planetary_command_source.py")
set(R179_AUDIT "${R179_PROJECT_ROOT}/docs/audits/NULLHARBOR_R179_PLANETARY_COMMAND_PREIMAGE_RECONCILIATION.md")

# Stable identity and governed state progression.
r179_require("${R179_PI_H}" "enum class PiClaimState" "claim-state authority missing")
r179_require("${R179_PI_H}" "enum class PiOverlayMode" "overlay authority missing")
r179_require("${R179_PI_H}" "enum class PiProjectionMode" "projection authority missing")
r179_require("${R179_PI_H}" "PiSectorIdentity" "stable sector identity missing")
r179_require("${R179_PI_H}" "PlaceGoverned" "governed placement API missing")
r179_require("${R179_PI_CPP}" "CLAIM REQUIRES CONTIGUOUS OWNED BORDER" "contiguous claim policy missing")
r179_require("${R179_PI_CPP}" "ClaimFrontier" "claim frontier derivation missing")

# R178 append-only input authority survives while R179 adds only a new action.
r179_require("${R179_INPUT}" "FleetCommandCancel" "R178 fleet-control action missing")
r179_require("${R179_INPUT}" "PlanetaryCommandCycleOverlay" "R179 overlay action missing")
r179_require("${R179_WINDOW}" "PlanetaryCommandCycleOverlay" "native F5 binding missing")
r179_require("${R179_BINDINGS}" "OpenPlanetaryManufacturing,\"P\"" "P command binding missing")
r179_require("${R179_BINDINGS}" "PlanetaryCommandCycleOverlay,\"F5\"" "F5 overlay binding missing")

# Drawing and hit-testing share the same player-facing projection authority.
r179_require("${R179_FACE_H}" "PlanetaryCommandLayout" "command layout contract missing")
r179_require("${R179_FACE_CPP}" "ProjectPlanetaryHex" "projection implementation missing")
r179_require("${R179_FACE_CPP}" "HitTestPlanetaryHex" "shared hit-test projection missing")
r179_require("${R179_FACE_CPP}" "CyclePlanetaryOverlay" "overlay cycle missing")
r179_require("${R179_FACE_CPP}" "TogglePlanetaryProjection" "globe/sector projection toggle missing")
r179_require("${R179_FACE_CPP}" "PlaceGoverned" "player-facing placement bypasses governed industry")

# Live application owns command-view pointer input and prevents flight-context leakage.
r179_require("${R179_APP}" "R179 Planetary Command owns pointer clicks" "command pointer ownership missing")
r179_require("${R179_APP}" "input.WasPressed(InputAction::PlanetaryCommandCycleOverlay)" "live F5 route missing")
r179_require("${R179_APP}" "_window.IsShiftDown()" "Shift+Enter deployment route missing")
r179_require("${R179_RENDER}" "PLANETARY COMMAND - " "command renderer title missing")
r179_require("${R179_RENDER}" "SECTOR INSPECTOR" "sector inspector missing")
r179_require("${R179_RENDER}" "CLAIM FRONTIER" "claim frontier visualization missing")
r179_require("${R179_WORKSPACE}" "Claim contiguous sector" "workspace action vocabulary stale")

# Historical source transformer is now a semantic verifier, so later milestones
# can extend InputState without needing a growing list of certified preimages.
r179_require("${R179_MIGRATION}" "historical 17-transform text migration is retired" "legacy migration replay not retired")
r179_require("${R179_MIGRATION}" "canonical-semantic-verification" "semantic migration record missing")
r179_require("${R179_VERIFY}" "R158-R177 -> R179 Planetary Command source verification PASS" "source verifier missing")
r179_require("${R179_AUDIT}" "expected one certified preimage" "preimage reconciliation audit missing")

message(STATUS "R179 NullHarbor Planetary Command convergence source gate PASS")
