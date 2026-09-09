if(NOT DEFINED ROOT)
  message(FATAL_ERROR "Pass513-522 source gate requires -DROOT=<engine-root>")
endif()

set(RENDERER "${ROOT}/src/application/NativeBattlefieldRenderer.cpp")
if(NOT EXISTS "${RENDERER}")
  message(FATAL_ERROR "Pass513-522 renderer source missing: ${RENDERER}")
endif()

file(READ "${RENDERER}" RENDERER_TEXT)

function(require_text TEXT_VALUE NEEDLE)
  string(FIND "${TEXT_VALUE}" "${NEEDLE}" FOUND_AT)
  if(FOUND_AT EQUAL -1)
    message(FATAL_ERROR "Pass513-522 source gate missing: ${NEEDLE}")
  endif()
endfunction()

# Pass513-522 historical responsibilities remain source-certified. Pass746
# deliberately replaced the old giant shield envelope with a hull-profile
# surface, so this gate follows the current renderer authority instead of
# requiring a retired implementation name.
require_text("${RENDERER_TEXT}" "DrawShipProfileShield")
require_text("${RENDERER_TEXT}" "frame.commandHud.shieldOnline")
require_text("${RENDERER_TEXT}" "DrawVectorTravelBackdrop")
require_text("${RENDERER_TEXT}" "DrawRuntimeStation3D")
require_text("${RENDERER_TEXT}" "DrawPlayableInterior")
require_text("${RENDERER_TEXT}" "DrawDockingGuidance")
require_text("${RENDERER_TEXT}" "DrawHud")

# Historical shield behavior must remain gated by live reserve/state, while
# regression to the spherical/bubble envelope is explicitly forbidden.
require_text("${RENDERER_TEXT}" "kShieldGapWorld=0.3048f")
require_text("${RENDERER_TEXT}" "ShieldRipplePulse")

string(FIND "${RENDERER_TEXT}" "DrawPlayerShieldEnvelope" LEGACY_SHIELD)
if(NOT LEGACY_SHIELD EQUAL -1)
  message(FATAL_ERROR
    "Pass513-522 compatibility gate rejected retired giant shield envelope authority")
endif()

message(STATUS "Pass513-522 source gate PASS (profile shield compatibility authority)")
