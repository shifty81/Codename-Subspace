# R178 milestone-sized convergence gate: FPS context, strategic fleet orders,
# universal interior kits and donor metadata must remain normalized around one
# current Subspace authority. This gate is intentionally source-only so it is
# picked up by ProjectOpsStaticCertification without extending native CMake.
get_filename_component(R178_PROJECT_ROOT "${ROOT}/.." ABSOLUTE)

function(r178_require FILE TOKEN MESSAGE_TEXT)
  if(NOT EXISTS "${FILE}")
    message(FATAL_ERROR "R178 missing required file: ${FILE}")
  endif()
  file(READ "${FILE}" R178_TEXT)
  string(FIND "${R178_TEXT}" "${TOKEN}" R178_INDEX)
  if(R178_INDEX EQUAL -1)
    message(FATAL_ERROR "R178 ${MESSAGE_TEXT}: missing token '${TOKEN}' in ${FILE}")
  endif()
endfunction()

function(r178_forbid FILE TOKEN MESSAGE_TEXT)
  if(NOT EXISTS "${FILE}")
    message(FATAL_ERROR "R178 missing required file: ${FILE}")
  endif()
  file(READ "${FILE}" R178_TEXT)
  string(FIND "${R178_TEXT}" "${TOKEN}" R178_INDEX)
  if(NOT R178_INDEX EQUAL -1)
    message(FATAL_ERROR "R178 ${MESSAGE_TEXT}: forbidden token '${TOKEN}' in ${FILE}")
  endif()
endfunction()

set(R178_INPUT "${ROOT}/include/input/InputState.h")
set(R178_ROUTER "${ROOT}/include/input/ControlIntentRouterSystem.h")
set(R178_CONTEXT "${ROOT}/src/ui/RuntimeControlContextSystem.cpp")
set(R178_FLEET_H "${ROOT}/include/fleet/FleetCommandSystem.h")
set(R178_FLEET_CPP "${ROOT}/src/fleet/FleetCommandSystem.cpp")
set(R178_STRATEGY "${ROOT}/include/fleet/FleetStrategyControlSystem.h")
set(R178_KIT "${ROOT}/include/interior/ModularInteriorKitSystem.h")
set(R178_REGISTRY "${ROOT}/include/interior/InteriorKitRegistrySystem.h")
set(R178_LINK "${ROOT}/src/interior/ShipModuleInteriorLinkSystem.cpp")
set(R178_INTERACT "${ROOT}/include/interior/InteriorInteractionSystem.h")
set(R178_DONOR "${R178_PROJECT_ROOT}/content/interiors/nullharbor_donor_kitbash_harvest_v1.json")
set(R178_AUTH "${R178_PROJECT_ROOT}/content/architecture/nullharbor_convergence_authority_v1.json")

# Append-only semantic input domains; historical flight actions remain present.
r178_require("${R178_INPUT}" "ThrustForward = 0" "historical input index authority drifted")
r178_require("${R178_INPUT}" "CharacterMoveForward" "first-person semantic input missing")
r178_require("${R178_INPUT}" "FleetCameraForward" "fleet camera semantic input missing")
r178_require("${R178_ROUTER}" "ControlDomain::FleetStrategy" "context router lacks fleet strategy")
r178_require("${R178_ROUTER}" "W/S/A/D are camera pan here, never ship thrust" "fleet-strategy input ownership is ambiguous")
r178_require("${R178_CONTEXT}" "context.controlDomain=ControlDomain::FleetStrategy" "authorized command seat does not own FleetStrategy domain")

# Fleet orders require real domain executors. Timer-completion is retired.
r178_require("${R178_FLEET_H}" "FleetOrderExecutor" "fleet executor contract missing")
r178_require("${R178_FLEET_H}" "Salvage" "expanded fleet order vocabulary missing")
r178_require("${R178_STRATEGY}" "It never mutates ship" "RTS control boundary missing")
r178_require("${R178_FLEET_CPP}" "WAITING FOR DOMAIN EXECUTOR" "missing-executor fail-visible state missing")
r178_forbid("${R178_FLEET_CPP}" "deltaTime / baseOrderTime" "generic fleet timer completion returned")

# Interior content is capability/context resolved, fine-snapped and source-pack neutral.
r178_require("${R178_KIT}" "fineSnapMeters = 0.25" "fine kitbash snap contract missing")
r178_require("${R178_KIT}" "structuralGridMeters = 1.0" "structural grid contract missing")
r178_require("${R178_KIT}" "planningCellMeters = 2.0" "planning grid contract missing")
r178_require("${R178_REGISTRY}" "source-pack names never leak" "universal kit-resolution policy missing")
r178_forbid("${R178_LINK}" "quaternius.ultimate_modular_scifi.2021" "hard-coded interior source kit returned")
r178_require("${R178_INTERACT}" "FleetCommandTerminal" "physical fleet-command fixture missing")
r178_require("${R178_INTERACT}" "RefineryConsole" "physical refinery fixture missing")
r178_require("${R178_INTERACT}" "ManufacturingConsole" "physical manufacturing fixture missing")

# Donor content is metadata-only until governed hydration/certification.
r178_require("${R178_DONOR}" "\"uniqueModuleRecords\": 195" "donor kitbash inventory count drifted")
r178_require("${R178_DONOR}" "\"quaterniusUltimateModularSciFiRecords\": 20" "UMSF normalized donor records missing")
r178_require("${R178_DONOR}" "\"rawDonorMeshesVendored\": false" "raw donor content was incorrectly promoted")
r178_require("${R178_AUTH}" "\"destinationProductIdentity\": \"NullHarbor\"" "destination identity policy missing")
r178_require("${R178_AUTH}" "A fleet order cannot complete solely because elapsed time passed." "fleet strategy invariant missing")

message(STATUS "R178 NullHarbor convergence source gate PASS")
