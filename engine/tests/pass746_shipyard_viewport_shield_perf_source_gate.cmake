set(RENDERER "${ROOT}/src/application/NativeBattlefieldRenderer.cpp")
set(CAMERA "${ROOT}/src/rendering/StrategicCamera.cpp")
set(SAFETY "${ROOT}/src/ship_editor/ShipyardBuildSafetySystem.cpp")
foreach(F IN ITEMS "${RENDERER}" "${CAMERA}" "${SAFETY}")
  if(NOT EXISTS "${F}")
    message(FATAL_ERROR "Pass746 source missing: ${F}")
  endif()
endforeach()
file(READ "${RENDERER}" R)
file(READ "${CAMERA}" C)
file(READ "${SAFETY}" S)
foreach(TOKEN IN ITEMS
  "kConstructionMarginWorld=3.048f"
  "BuildRecipeLocalBounds"
  "frameCenterX=(leftX+rightX)*.5f"
  "DrawShipProfileShield"
  "kShieldGapWorld=0.3048f"
  "kShieldCalmWaveHeightWorld=0.0060f"
  "kShieldImpactWaveHeightWorld=0.065f"
  "ShieldRipplePulse"
  "ActiveShieldRipple"
  "still pond"
  "surface.lastShieldFraction"
  "frame.missileSystem->GetDetonations()"
  "CachedShieldTriangles"
  "static std::unordered_map<std::uintptr_t,CachedPreview> cache"
  "record->placementRole"
  "COMPAT "
  "runtimeVisible")
  string(FIND "${R}" "${TOKEN}" P)
  if(P EQUAL -1)
    message(FATAL_ERROR "Pass746 renderer contract missing: ${TOKEN}")
  endif()
endforeach()
string(FIND "${R}" "DrawPlayerShieldEnvelope" OLD_SHIELD)
if(NOT OLD_SHIELD EQUAL -1)
  message(FATAL_ERROR "Pass746 giant sphere shield authority returned")
endif()
foreach(TOKEN IN ITEMS
  "InputAction::ThrustForward"
  "InputAction::ThrustReverse"
  "InputAction::StrafeLeft"
  "InputAction::StrafeRight"
  "InputAction::TurnLeft"
  "InputAction::TurnRight"
  "InputAction::Boost"
  "InputAction::EmergencyBrake"
  "InputAction::FirePrimary"
  "InputAction::FireMiningMissile"
  "input.SetActionValue(action, 0.0f)")
  string(FIND "${S}" "${TOKEN}" P)
  if(P EQUAL -1)
    message(FATAL_ERROR "Pass746 Shipyard flight/weapon suppression missing: ${TOKEN}")
  endif()
endforeach()
string(FIND "${C}" "kMaxScreenLookAhead = 0.08f" P)
if(P EQUAL -1)
  message(FATAL_ERROR "Pass746 screen-locked player follow missing")
endif()
message(STATUS "Pass746 viewport/shield/performance source gate PASS")
