# R185 closes the last native compatibility failures exposed after R184 finally
# reached CTest. It must preserve modern governed gameplay while restoring
# historical public contracts used by current certification suites.
function(r185_require FILE TOKEN LABEL)
  if(NOT EXISTS "${FILE}")
    message(FATAL_ERROR "R185 missing required file: ${FILE}")
  endif()
  file(READ "${FILE}" R185_TEXT)
  string(FIND "${R185_TEXT}" "${TOKEN}" R185_INDEX)
  if(R185_INDEX EQUAL -1)
    message(FATAL_ERROR "R185 ${LABEL}: missing '${TOKEN}' in ${FILE}")
  endif()
endfunction()

set(EMB_H "${ROOT}/include/interior/ShipEmbodimentSystem.h")
set(EMB_CPP "${ROOT}/src/interior/ShipEmbodimentSystem.cpp")
set(FACE_H "${ROOT}/include/integration/PlayerFacingIntegrationSystem.h")
set(FACE_CPP "${ROOT}/src/integration/PlayerFacingIntegrationSystem.cpp")
set(GALAXY_CPP "${ROOT}/src/procedural/GalaxyGenerator.cpp")
set(HIST_GATE "${CMAKE_CURRENT_LIST_DIR}/pass1578_1597_planetary_command.cmake")

# Physical cockpit interaction must not be inferred solely from a tiny accelerated step.
r185_require("${EMB_H}" "commandSeatInteractionReady_" "seat interaction state missing")
r185_require("${EMB_H}" "commandSeatDeparted_" "seat departure state missing")
r185_require("${EMB_CPP}" "RefreshCommandSeatInteraction" "seat re-entry authority missing")
r185_require("${EMB_CPP}" "commandSeatInteractionReady_=false" "movement does not disarm seat interaction")
r185_require("${EMB_CPP}" "commandSeatDeparted_=true" "seat departure is not tracked")

# Legacy PI helper remains compatible only for ad-hoc planets lacking canonical identity.
r185_require("${FACE_H}" "legacyPlacementCompatibility" "legacy PI compatibility discriminator missing")
r185_require("${FACE_H}" "SelectPlanetCommandHex" "historical Planetary Command API alias missing")
r185_require("${FACE_CPP}" "planet.planetId.empty()" "legacy PI compatibility is not identity-scoped")
r185_require("${FACE_CPP}" "pi.Place(model.industry,installation)" "legacy PI direct placement path missing")
r185_require("${FACE_CPP}" "pi.PlaceGoverned(model.industry,installation,model.ownerId)" "generated-planet governed placement missing")
r185_require("${GALAXY_CPP}" "planet.planetId = \"planet_\"" "generated planets must carry canonical IDs")

# The historical gate itself must certify current semantics, not retired text preimages.
r185_require("${HIST_GATE}" "superseded safely by R179 semantic authority PASS" "historical Planetary Command gate not normalized")
r185_require("${HIST_GATE}" "SelectPlanetCommandHex" "historical API compatibility not certified")
r185_require("${HIST_GATE}" "PlaceGoverned" "historical gate no longer protects governed placement")

message(STATUS "R185 native compatibility closure source gate PASS")
