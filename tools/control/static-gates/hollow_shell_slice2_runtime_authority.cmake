cmake_minimum_required(VERSION 3.20)
# Fail-closed source check: runtime renderer, movement and document all consume
# the same cached derived shell. This is not an automated visual or FPS test.
get_filename_component(HH2_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
set(HH2_APP "${HH2_ROOT}/engine/src/application/NativeGameApplication.cpp")
set(HH2_RENDER "${HH2_ROOT}/engine/src/application/NativeBattlefieldRenderer.cpp")
set(HH2_FRAME "${HH2_ROOT}/engine/include/application/NativeBattlefieldRenderer.h")
set(HH2_SOLVER "${HH2_ROOT}/engine/src/interior/ShipInteriorShellTraversalSystem.cpp")
set(HH2_BUILD "${HH2_ROOT}/engine/src/interior/ShipInteriorDerivedShellSystem.cpp")
foreach(HH2_FILE IN ITEMS APP RENDER FRAME SOLVER BUILD)
    if(NOT EXISTS "${HH2_${HH2_FILE}}")
        message(FATAL_ERROR "Hollow Shell Slice2 missing ${HH2_FILE}")
    endif()
    file(READ "${HH2_${HH2_FILE}}" HH2_${HH2_FILE}_TEXT)
endforeach()
foreach(HH2_MARK IN ITEMS
    "_playerInteriorLayout=builder.Materialize"
    "f.playerInteriorShell=_hasPlayerShipRecipe?&_playerInteriorLayout.shell:nullptr"
    "ShipInteriorShellTraversalSystem::Move("
    "if(!_playerInteriorLayout.shell.ready)return")
    string(FIND "${HH2_APP_TEXT}" "${HH2_MARK}" HH2_INDEX)
    if(HH2_INDEX EQUAL -1)
        message(FATAL_ERROR "Hollow Shell Slice2 application authority lost: ${HH2_MARK}")
    endif()
endforeach()
foreach(HH2_MARK IN ITEMS
    "frame.playerInteriorShell->ready"
    "for(const auto& surface:shell.surfaces)"
    "if(surface.axis==2&&surface.direction>0)continue"
    "glVertex3f(v.x,v.y,v.z)"
    "INTERIOR SHELL NOT READY / MOVEMENT DISABLED")
    string(FIND "${HH2_RENDER_TEXT}" "${HH2_MARK}" HH2_INDEX)
    if(HH2_INDEX EQUAL -1)
        message(FATAL_ERROR "Hollow Shell Slice2 visual authority lost: ${HH2_MARK}")
    endif()
endforeach()
string(FIND "${HH2_RENDER_TEXT}" "One compact starter deck" HH2_OBSOLETE)
if(NOT HH2_OBSOLETE EQUAL -1)
    message(FATAL_ERROR "Obsolete synthetic deck restored")
endif()
foreach(HH2_MARK IN ITEMS
    "const InteriorDerivedShell* playerInteriorShell = nullptr"
    "const ShipInteriorCarvePlan* playerInteriorCarve = nullptr")
    string(FIND "${HH2_FRAME_TEXT}" "${HH2_MARK}" HH2_INDEX)
    if(HH2_INDEX EQUAL -1)
        message(FATAL_ERROR "Hollow Shell Slice2 frame authority lost: ${HH2_MARK}")
    endif()
endforeach()
foreach(HH2_MARK IN ITEMS
    "for(const auto& surface:shell.surfaces)"
    "if(!surface.blocksMovement)continue"
    "const int steps=std::clamp"
    "if(!CanOccupy(carve,shell,feet,radius,height))return feet")
    string(FIND "${HH2_SOLVER_TEXT}" "${HH2_MARK}" HH2_INDEX)
    if(HH2_INDEX EQUAL -1)
        message(FATAL_ERROR "Hollow Shell Slice2 collision source lost: ${HH2_MARK}")
    endif()
endforeach()
foreach(HH2_MARK IN ITEMS "const float doorBottom=std::max(floorA,floorB)" "Vertical portal requires authored stairs/elevator geometry")
    string(FIND "${HH2_BUILD_TEXT}" "${HH2_MARK}" HH2_INDEX)
    if(HH2_INDEX EQUAL -1)
        message(FATAL_ERROR "Hollow Shell Slice2 floor-aligned aperture lost: ${HH2_MARK}")
    endif()
endforeach()
message(STATUS "Hollow Shell Slice2 runtime/collision source authority PASS")
