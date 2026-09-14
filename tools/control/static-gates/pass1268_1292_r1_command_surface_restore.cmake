file(READ "${ROOT}/include/ship_editor/ShipyardBuilderSystem.h" BUILDER_H)
file(READ "${ROOT}/src/ship_editor/ShipyardBuilderSystem.cpp" BUILDER_CPP)

set(REQUIRED_COMMANDS
    "ModelPreviousPurpose"
    "ModelNextPurpose"
    "ModelAssignPurpose"
    "PreviousGeneratorDomain"
    "NextGeneratorDomain")

foreach(TOKEN IN LISTS REQUIRED_COMMANDS)
    if(BUILDER_CPP MATCHES "ShipyardBuilderCommand::${TOKEN}" AND
       NOT BUILDER_H MATCHES "[\n\r][ \t]*${TOKEN}[,\n\r]")
        message(FATAL_ERROR "Pass1268-1292 R1: ShipyardBuilderSystem.cpp uses ${TOKEN} but the public command enum does not declare it")
    endif()
endforeach()

if(NOT BUILDER_H MATCHES "LegacyActivate" OR
   NOT BUILDER_H MATCHES "LegacyBuildControls" OR
   NOT BUILDER_H MATCHES "developerWorkspacesVisible" OR
   NOT BUILDER_H MATCHES "testWorkspaceActive")
    message(FATAL_ERROR "Pass1268-1292 R1: visible-cutover compatibility declarations were lost while restoring command surface")
endif()

message(STATUS "Pass1268-1292 R1 Shipyard command/header surface restore PASS")
