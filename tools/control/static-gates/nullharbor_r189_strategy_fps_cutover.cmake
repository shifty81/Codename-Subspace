cmake_minimum_required(VERSION 3.20)
get_filename_component(ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
foreach(P IN ITEMS
  "${ROOT}/engine/include/application/NativeGameApplication.h"
  "${ROOT}/engine/src/application/NativeGameApplication.cpp"
  "${ROOT}/engine/include/application/NativeBattlefieldRenderer.h"
  "${ROOT}/engine/src/application/NativeBattlefieldRenderer.cpp"
  "${ROOT}/engine/CMakeLists.txt"
  "${ROOT}/tools/runtime/r189_strategy_fps_cutover.py"
  "${ROOT}/SubspaceTools.ps1")
  if(NOT EXISTS "${P}")
    message(FATAL_ERROR "R189 missing authority file: ${P}")
  endif()
endforeach()
file(READ "${ROOT}/engine/include/application/NativeGameApplication.h" A)
file(READ "${ROOT}/engine/src/application/NativeGameApplication.cpp" B)
file(READ "${ROOT}/engine/include/application/NativeBattlefieldRenderer.h" C)
file(READ "${ROOT}/engine/src/application/NativeBattlefieldRenderer.cpp" D)
file(READ "${ROOT}/engine/CMakeLists.txt" E)
file(READ "${ROOT}/tools/runtime/r189_strategy_fps_cutover.py" T)
file(READ "${ROOT}/SubspaceTools.ps1" P)
function(req V TKN LABEL)
  string(FIND "${${V}}" "${TKN}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "R189 missing ${LABEL}: ${TKN}")
  endif()
endfunction()
req(A "FleetStrategyControlSystem _fleetStrategy" "live FleetStrategy authority")
req(A "PlayerController _playerController" "runtime player context facade")
# R191 intentionally supersedes R189's detached strategy-first boot with the
# embodied OnFoot FPS primary-gameplay boot while retaining FleetStrategy as an
# explicit command mode. Accept either certified lineage.
string(FIND "${B}" "R189_STRATEGY_FIRST_BOOT" R189_LEGACY_BOOT)
if(R189_LEGACY_BOOT EQUAL -1)
  req(B "R191_PRIMARY_FPS_BOOT" "R191 embodied FPS boot supersession")
  req(B "GameplayControlMode::OnFoot" "R191 OnFoot authority")
  req(B "SetGameplayControlMode" "R191 explicit gameplay-mode transition authority")
endif()
req(B "BoardPlayerShipFromStrategy" "strategy-to-FPS bridge")
req(B "_fleetStrategy.TickCamera" "WASD strategy camera")
req(B "FirstPersonViewSystem::BuildOnFootLocal" "FPS eye pose")
req(C "hasFirstPersonPose" "renderer FPS frame contract")
req(D "R189_REAL_FPS_PROJECTION" "real FPS projection")
req(D "FLEET COMMAND" "strategy HUD vocabulary")
req(E "SubspaceR189StrategyFpsCutoverTests" "CTest registration")
req(T "future governed descendants are accepted" "forward-compatible materializer")
req(P "R189 strategy-first runtime / real FPS cutover" "PCC prebuild step")
message(STATUS "R189 strategy-first runtime / real FPS cutover source gate PASS")
