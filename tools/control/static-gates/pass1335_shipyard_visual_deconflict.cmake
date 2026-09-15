# Pass1335 - Shipyard visual deconflict / single-shell source gate.
get_filename_component(PROJECT_ROOT "${ROOT}/.." ABSOLUTE)
set(VISIBLE "${ROOT}/src/ship_editor/ShipyardProfessionalVisibleCutover.cpp")
if(NOT EXISTS "${VISIBLE}")
  message(FATAL_ERROR "Pass1335 visible Shipyard source missing: ${VISIBLE}")
endif()
file(READ "${VISIBLE}" UI)

# Pass1340-1439 compacts the same single-shell authority further for a
# viewport-first Blender DCC layout. Certify the modern spacing and retained
# off-screen legacy compatibility projection instead of the retired literals.
foreach(TOKEN IN ITEMS
  "l.top=88.0f*s"
  "l.placedListY=l.contentTopY+42.0f*s"
  "showSidebar"
  "kCompatibilityStride=100000.0f"
  "complete historical command surface")
  string(FIND "${UI}" "${TOKEN}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass1335 deconflict authority missing: ${TOKEN}")
  endif()
endforeach()

foreach(FORBIDDEN IN ITEMS
  "OUTLINER  /  SHIP HIERARCHY"
  "PROPERTIES  /  INSTANCE + DEFINITION"
  "if(c.x<l.right-1.0f)continue;")
  string(FIND "${UI}" "${FORBIDDEN}" POS)
  if(NOT POS EQUAL -1)
    message(FATAL_ERROR "Pass1335 duplicate visible Shipyard projection still present: ${FORBIDDEN}")
  endif()
endforeach()

message(STATUS "Pass1335 Shipyard visual deconflict source gate passed")
