# Pass456 R5 source-level guard: the normal battlefield renderer must not regain
# the pre-Shipyard synthetic ship/thruster presentation that repeatedly
# resurfaced through later patch overlays.
file(READ "${RENDERER_SOURCE}" _src)
set(_forbidden
    "ThrusterLayoutSystem"
    "DrawFallbackShip"
    "hull_section_small"
    "cockpit_basic"
    "engine_main"
    "engine_small"
    "thruster_small"
)
foreach(_token IN LISTS _forbidden)
    string(FIND "${_src}" "${_token}" _pos)
    if(NOT _pos EQUAL -1)
        message(FATAL_ERROR "R5 visual-authority regression: NativeBattlefieldRenderer.cpp contains forbidden legacy token '${_token}'.")
    endif()
endforeach()
message(STATUS "R5 source gate: normal renderer contains no forbidden legacy ship/thruster visual path.")
