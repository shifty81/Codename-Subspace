if(NOT DEFINED ROOT)
  message(FATAL_ERROR "ProjectOps static certification requires -DROOT=<engine-root>")
endif()

get_filename_component(PROJECT_ROOT "${ROOT}/.." ABSOLUTE)
set(GATE_DIR "${PROJECT_ROOT}/tools/control/static-gates")

if(NOT EXISTS "${GATE_DIR}")
  message(FATAL_ERROR "ProjectOps static gate directory missing: ${GATE_DIR}")
endif()

file(GLOB STATIC_GATES LIST_DIRECTORIES false "${GATE_DIR}/*.cmake")
list(SORT STATIC_GATES)

list(LENGTH STATIC_GATES GATE_COUNT)
if(GATE_COUNT EQUAL 0)
  message(FATAL_ERROR "ProjectOps static certification has no registered gates.")
endif()

message(STATUS "ProjectOps static certification: ${GATE_COUNT} registered gate(s)")
foreach(GATE_PATH IN LISTS STATIC_GATES)
  get_filename_component(GATE_NAME "${GATE_PATH}" NAME)
  message(STATUS "ProjectOps static gate START: ${GATE_NAME}")
  include("${GATE_PATH}")
  message(STATUS "ProjectOps static gate PASS: ${GATE_NAME}")
endforeach()

message(STATUS "ProjectOps static certification PASS")
