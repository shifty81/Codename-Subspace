file(READ "${ROOT}/src/procedural/GalaxyGenerator.cpp" GENERATOR)
file(READ "${ROOT}/src/procedural/SolarSystemPlacementSystem.cpp" PLACEMENT)
file(READ "${ROOT}/src/application/NativeBattlefieldRenderer.cpp" RENDERER)
file(READ "${ROOT}/src/navigation/SystemMapSystem.cpp" MAP)
file(READ "${ROOT}/src/application/NativeGameApplication.cpp" APP_SOURCE)
foreach(token
    "SolarSystemPlacementSystem placement"
    "placement.Normalize(sector)"
    "Pass496 first-class moon authority")
  string(FIND "${GENERATOR}" "${token}" found)
  if(found LESS 0)
    message(FATAL_ERROR "Pass496 generator source gate missing: ${token}")
  endif()
endforeach()
foreach(token
    "StellarExclusionRadiusSector"
    "PlanetEnvelopeRadiusSector"
    "BELT_REGION"
    "DEEP_SPACE"
    "asteroid has no region authority")
  string(FIND "${PLACEMENT}" "${token}" found)
  if(found LESS 0)
    message(FATAL_ERROR "Pass496 placement source gate missing: ${token}")
  endif()
endforeach()
string(FIND "${RENDERER}" "SystemSpatialScale::SectorToWorld" scaleFound)
if(scaleFound LESS 0)
  message(FATAL_ERROR "Pass496 renderer no longer consumes shared sector/world scale")
endif()
string(FIND "${MAP}" "sector.moons" moonFound)
if(moonFound LESS 0)
  message(FATAL_ERROR "Pass496 System Map does not consume first-class moon records")
endif()
message(STATUS "Pass496 solar-system placement source gate passed")

foreach(token
    "Pass496 solar placement certification"
    "ecologyOrbitBase"
    "PlanetEnvelopeRadiusSector")
  string(FIND "${APP_SOURCE}" "${token}" found)
  if(found LESS 0)
    message(FATAL_ERROR "Pass496 runtime integration source gate missing: ${token}")
  endif()
endforeach()
