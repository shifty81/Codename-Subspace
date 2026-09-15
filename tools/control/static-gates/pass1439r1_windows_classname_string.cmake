# Pass1439R1 - Windows/MSVC ship class label concatenation repair.
set(_header "${CMAKE_CURRENT_LIST_DIR}/../../../engine/include/ships/ShipClassRoleSystem.h")
set(_source "${CMAKE_CURRENT_LIST_DIR}/../../../engine/src/ships/ShipClassRoleSystem.cpp")
set(_renderer "${CMAKE_CURRENT_LIST_DIR}/../../../engine/src/application/NativeBattlefieldRenderer.cpp")

foreach(_f IN ITEMS "${_header}" "${_source}" "${_renderer}")
  if(NOT EXISTS "${_f}")
    message(FATAL_ERROR "Pass1439R1 missing required source: ${_f}")
  endif()
endforeach()

file(READ "${_header}" _h)
file(READ "${_source}" _s)
file(READ "${_renderer}" _r)

if(NOT _h MATCHES "static std::string ClassName\\(ShipClass shipClass\\);")
  message(FATAL_ERROR "Pass1439R1: ShipClassRoleSystem::ClassName must return std::string.")
endif()
if(NOT _s MATCHES "std::string ShipClassRoleSystem::ClassName\\(ShipClass c\\)")
  message(FATAL_ERROR "Pass1439R1: ClassName implementation signature drift.")
endif()
if(NOT _r MATCHES "v  SHIP  /  ")
  message(FATAL_ERROR "Pass1439R1: Blender Outliner ship label is missing.")
endif()

message(STATUS "PASS1439R1 Windows/MSVC class-name string gate: PASS")
