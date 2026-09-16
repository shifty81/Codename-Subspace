cmake_minimum_required(VERSION 3.20)
# PASS1505R1: Windows/MSVC renderer lowercase helper repair.
get_filename_component(PASS1505R1_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
set(RENDERER "${PASS1505R1_ROOT}/engine/src/application/NativeBattlefieldRenderer.cpp")
if(NOT EXISTS "${RENDERER}")
  message(FATAL_ERROR "PASS1505R1 renderer missing: ${RENDERER}")
endif()
file(READ "${RENDERER}" TEXT)
string(FIND "${TEXT}" "ToLowerAscii(placement.moduleId)" LEGACY_POS)
if(NOT LEGACY_POS EQUAL -1)
  message(FATAL_ERROR "PASS1505R1 unresolved renderer call to unavailable ToLowerAscii helper")
endif()
foreach(TOKEN IN ITEMS
  "auto moduleLower=placement.moduleId"
  "std::transform(moduleLower.begin(),moduleLower.end(),moduleLower.begin()"
  "moduleLower.find(\"antenna\")"
  "SpaceMaterialKind::StructuralMetal")
  string(FIND "${TEXT}" "${TOKEN}" TOKEN_POS)
  if(TOKEN_POS EQUAL -1)
    message(FATAL_ERROR "PASS1505R1 renderer repair missing token: ${TOKEN}")
  endif()
endforeach()
message(STATUS "PASS1505R1 Windows renderer ASCII lowercase repair PASS")
