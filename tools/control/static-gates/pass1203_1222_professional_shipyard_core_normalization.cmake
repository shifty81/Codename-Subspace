if(NOT DEFINED PROJECT_ROOT)
  get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
endif()
set(E "${PROJECT_ROOT}/engine")

foreach(FILE
    "${E}/include/ship_editor/ShipyardStableIdSystem.h"
    "${E}/include/ship_editor/ShipyardDocumentSystem.h"
    "${E}/include/ship_editor/ShipyardSelectionSystem.h"
    "${E}/include/ship_editor/ShipyardHistorySystem.h"
    "${E}/include/ship_editor/ShipyardSessionSystem.h"
    "${E}/include/ship_editor/ShipyardCommandSystem.h"
    "${E}/include/ship_editor/ShipyardMountProfileSystem.h"
    "${E}/include/ship_editor/ShipyardProfessionalUiSystem.h")
  if(NOT EXISTS "${FILE}")
    message(FATAL_ERROR "Pass1203-1222 missing normalized Shipyard authority: ${FILE}")
  endif()
endforeach()

file(READ "${E}/include/ship_editor/ShipyardDocumentSystem.h" DOCUMENT)
file(READ "${E}/src/ship_editor/ShipyardDocumentSystem.cpp" DOCUMENT_CPP)
file(READ "${E}/include/ship_editor/ShipyardCommandSystem.h" COMMANDS)
file(READ "${E}/src/ship_editor/ShipyardHistorySystem.cpp" HISTORY)
file(READ "${E}/src/ship_editor/ShipyardProfessionalUiSystem.cpp" UI)
file(READ "${E}/src/ship_editor/ShipyardMountProfileSystem.cpp" MOUNT)

foreach(TOKEN "moduleIds" "attachmentIds" "revision" "savedRevision" "ShipyardObjectId")
  if(NOT DOCUMENT MATCHES "${TOKEN}")
    message(FATAL_ERROR "Pass1203-1222 document authority missing token ${TOKEN}")
  endif()
endforeach()
if(NOT DOCUMENT_CPP MATCHES "EraseModule" OR NOT DOCUMENT_CPP MATCHES "Attachment references an invalid module index")
  message(FATAL_ERROR "Pass1203-1222 document mutation/identity validation incomplete")
endif()
if(NOT COMMANDS MATCHES "ShipyardCommandDescriptor" OR NOT COMMANDS MATCHES "ShipyardCommandContext" OR NOT COMMANDS MATCHES "ShipyardCommandResult")
  message(FATAL_ERROR "Pass1203-1222 command registry contract incomplete")
endif()
if(NOT HISTORY MATCHES "startingRevision" OR NOT HISTORY MATCHES "ShipyardDocumentSystem::Capture")
  message(FATAL_ERROR "Pass1203-1222 history is not document-transaction based")
endif()
foreach(TOKEN "ShipyardWorkspaceMode::Build" "ShipyardWorkspaceMode::Interior" "ShipyardWorkspaceMode::Systems" "ShipyardWorkspaceMode::Appearance" "ShipyardWorkspaceMode::Test")
  if(NOT UI MATCHES "${TOKEN}")
    message(FATAL_ERROR "Pass1203-1222 professional workspace strip missing ${TOKEN}")
  endif()
endforeach()
if(NOT UI MATCHES "PCG Lab" OR NOT UI MATCHES "Project Tools" OR NOT UI MATCHES "Raw Authoring")
  message(FATAL_ERROR "Pass1203-1222 advanced workspace progressive disclosure incomplete")
endif()
if(NOT MOUNT MATCHES "enginestrutfoot" OR NOT MOUNT MATCHES "geometry-largest-flat-root" OR NOT MOUNT MATCHES "LargestFlatSurface")
  message(FATAL_ERROR "Pass1203-1222 broad root-surface mounting policy missing")
endif()
message(STATUS "PASS1203-1222 professional Shipyard core normalization certified")
