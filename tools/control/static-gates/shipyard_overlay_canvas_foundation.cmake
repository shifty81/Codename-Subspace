cmake_minimum_required(VERSION 3.20)
# SHIPYARD-OVERLAY-SLICE1: use uniquely prefixed variables. The ProjectOps
# certification includes every static gate in a shared CMake process.
get_filename_component(SOS1_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
set(SOS1_FILES
    "engine/src/ui/SubspaceUiFramework.cpp"
    "engine/src/ship_editor/ShipyardWorkspaceSystem.cpp"
    "engine/src/ship_editor/ShipyardProfessionalVisibleCutover.cpp"
    "engine/include/ship_editor/ShipyardDockPointerSystem.h"
    "engine/include/ship_editor/ShipyardOverlayLayoutStore.h"
    "engine/src/application/NativeGameApplication.cpp"
    "engine/tests/shipyard_overlay_foundation_tests.cpp"
    "engine/CMakeLists.txt")
foreach(SOS1_REL IN LISTS SOS1_FILES)
    if(NOT EXISTS "${SOS1_ROOT}/${SOS1_REL}")
        message(FATAL_ERROR "Overlay foundation source missing: ${SOS1_REL}")
    endif()
endforeach()
file(READ "${SOS1_ROOT}/engine/src/ui/SubspaceUiFramework.cpp" SOS1_LAYOUT)
file(READ "${SOS1_ROOT}/engine/src/ship_editor/ShipyardWorkspaceSystem.cpp" SOS1_WORKSPACE)
file(READ "${SOS1_ROOT}/engine/include/ship_editor/ShipyardDockPointerSystem.h" SOS1_POINTER)
file(READ "${SOS1_ROOT}/engine/src/application/NativeGameApplication.cpp" SOS1_APP)
file(READ "${SOS1_ROOT}/engine/CMakeLists.txt" SOS1_CMAKE)
foreach(SOS1_ASSERT IN ITEMS
    "LayoutShipyardOverlays(w,width,height,topInset,out)"
    "if(active.empty())continue; // empty anchor does not leave a viewport hole"
    "if(id==\"center\")return {0,topInset,x,available}")
    string(FIND "${SOS1_LAYOUT}" "${SOS1_ASSERT}" SOS1_INDEX)
    if(SOS1_INDEX EQUAL -1)
        message(FATAL_ERROR "Overlay-first canvas contract changed: ${SOS1_ASSERT}")
    endif()
endforeach()
string(FIND "${SOS1_WORKSPACE}" "add(\"tool_rail\",\"Tools\",\"tool_left\",true,1.0f,48,420,true,true,true)" SOS1_INDEX)
if(SOS1_INDEX EQUAL -1)
    message(FATAL_ERROR "Tool rail must remain moveable and floatable")
endif()
foreach(SOS1_ASSERT IN ITEMS
    "ShipyardPanelCompositorSystem::Snapshot(w,width,height,topInset)"
    "(panel==\"tool_rail\"&&target!=\"tool_left\")")
    string(FIND "${SOS1_POINTER}" "${SOS1_ASSERT}" SOS1_INDEX)
    if(SOS1_INDEX EQUAL -1)
        message(FATAL_ERROR "Dock pointer overlay snapshot/rail authority missing: ${SOS1_ASSERT}")
    endif()
endforeach()
foreach(SOS1_ASSERT IN ITEMS
    "ShipyardOverlayLayoutStore::Load(_shipBuilder.MutableDockWorkspace()"
    "ShipyardOverlayLayoutStore::Save(_shipBuilder.Model().dockWorkspace")
    string(FIND "${SOS1_APP}" "${SOS1_ASSERT}" SOS1_INDEX)
    if(SOS1_INDEX EQUAL -1)
        message(FATAL_ERROR "Overlay persistence disconnected: ${SOS1_ASSERT}")
    endif()
endforeach()
string(FIND "${SOS1_CMAKE}" "SubspaceShipyardOverlayFoundationTests" SOS1_INDEX)
if(SOS1_INDEX EQUAL -1)
    message(FATAL_ERROR "Shipyard overlay CTest must be registered")
endif()
message(STATUS "Shipyard overlay-first canvas, movable rail, drag snapshot and layout persistence PASS")
