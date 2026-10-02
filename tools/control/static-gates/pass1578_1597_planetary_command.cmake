# R158-R177 historical Planetary Command certification, normalized after R179.
# The original text-transform-era API is superseded by the canonical R179
# sector/claim/projection authority. Keep only stable compatibility aliases and
# certify the current semantic owners instead of requiring retired preimages.

function(r158r177_require FILE TOKEN LABEL)
  if(NOT EXISTS "${FILE}")
    message(FATAL_ERROR "R158-R177 Planetary Command missing required file: ${FILE}")
  endif()
  file(READ "${FILE}" R158R177_TEXT)
  string(FIND "${R158R177_TEXT}" "${TOKEN}" R158R177_INDEX)
  if(R158R177_INDEX EQUAL -1)
    message(FATAL_ERROR "R158-R177 Planetary Command missing '${TOKEN}' in ${FILE} (${LABEL})")
  endif()
endfunction()

set(PI_H "${ROOT}/include/economy/PlanetaryIndustrySystem.h")
set(PI_CPP "${ROOT}/src/economy/PlanetaryIndustrySystem.cpp")
set(FACE_H "${ROOT}/include/integration/PlayerFacingIntegrationSystem.h")
set(FACE_CPP "${ROOT}/src/integration/PlayerFacingIntegrationSystem.cpp")
set(INPUT_H "${ROOT}/include/input/InputState.h")
set(APP_CPP "${ROOT}/src/application/NativeGameApplication.cpp")
set(RENDER_CPP "${ROOT}/src/application/NativeBattlefieldRenderer.cpp")

r158r177_require("${PI_H}" "enum class PiClaimState" "claim state")
r158r177_require("${PI_H}" "enum class PiOverlayMode" "overlay model")
r158r177_require("${PI_H}" "enum class PiProjectionMode" "projection model")
r158r177_require("${PI_H}" "PlaceGoverned" "governed deployment")
r158r177_require("${PI_H}" "AdvanceSector" "survey/claim/develop progression")
r158r177_require("${PI_CPP}" "CLAIM REQUIRES CONTIGUOUS OWNED BORDER" "contiguous territorial claims")

r158r177_require("${FACE_H}" "PlanetaryCommandLayout" "command workspace layout")
r158r177_require("${FACE_H}" "SelectPlanetCommandHex" "historical selection API compatibility")
r158r177_require("${FACE_H}" "HitTestPlanetaryHex" "pointer selection")
r158r177_require("${FACE_H}" "CyclePlanetaryOverlay" "six-overlay command model")
r158r177_require("${FACE_H}" "AdvancePlanetarySector" "command progression")
r158r177_require("${FACE_CPP}" "ProjectPlanetaryHex" "shared renderer/hit-test projection")
r158r177_require("${FACE_CPP}" "SelectPlanetCommandHex" "selection compatibility implementation")
r158r177_require("${FACE_CPP}" "PlaceGoverned" "canonical generated-planet deployment")

r158r177_require("${INPUT_H}" "PlanetaryCommandCycleOverlay" "F5 semantic action")
r158r177_require("${APP_CPP}" "HitTestPlanetaryHex" "live command pointer ownership")
r158r177_require("${APP_CPP}" "AdvancePlanetarySector" "live Enter progression")
r158r177_require("${RENDER_CPP}" "PLANETARY COMMAND" "visible command workspace")
r158r177_require("${RENDER_CPP}" "CLAIM FRONTIER" "territory visualization")

message(STATUS "R158-R177 Planetary Command historical gate superseded safely by R179 semantic authority PASS")
