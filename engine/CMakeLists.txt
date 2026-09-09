cmake_minimum_required(VERSION 3.16)
project(SubspaceEngine VERSION 0.1.0 LANGUAGES CXX)

# ---------------------------------------------------------------------------
# Global compiler settings
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------------------
# Put all build outputs directly into the build directory so that the
# executables are at  build/subspace_game(.exe)  regardless of generator
# (Visual Studio, Ninja, Make, etc.).
# ---------------------------------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# Per-configuration overrides keep multi-config generators (Visual Studio,
# Xcode) from appending a Debug/ or Release/ subdirectory.
foreach(_cfg DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    string(TOUPPER "${_cfg}" _cfg_upper)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${_cfg_upper} "${CMAKE_BINARY_DIR}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${_cfg_upper} "${CMAKE_BINARY_DIR}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${_cfg_upper} "${CMAKE_BINARY_DIR}")
endforeach()

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
option(SUBSPACE_BUILD_TESTS  "Build the test suite" ON)
option(SUBSPACE_USE_GLFW     "Find and link GLFW3 (install via your package manager or vcpkg)" OFF)

# Root Project Control passes these values on every configure. They are explicit
# build-contract switches even though the current Windows target remains the
# authoritative native WGL/OpenGL shell in both utility lanes. Declaring them
# prevents fresh CMake configurations from reporting them as unused variables.
option(SUBSPACE_HEADLESS     "Select the root utility headless build lane" OFF)
option(SUBSPACE_BUILD_OPENGL "Select the native OpenGL render build lane" ON)

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------
# OpenGL is a game/platform-shell dependency, not an engine-library dependency.
# The native Windows backend uses WGL/OpenGL directly. Headless engine/tests do
# not require OpenGL development packages.
if(WIN32)
    find_package(OpenGL REQUIRED)
endif()

if(SUBSPACE_USE_GLFW)
    # GLFW3 can be installed via:
    #   apt  : sudo apt install libglfw3-dev
    #   brew : brew install glfw
    #   vcpkg: vcpkg install glfw3
    find_package(glfw3 3.3 REQUIRED)
endif()

# nlohmann/json – header-only, expected at engine/include/nlohmann/json.hpp
# No find_package needed; the include directory below covers it.

# ---------------------------------------------------------------------------
# Collect source files from every src/ subdirectory
# ---------------------------------------------------------------------------
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/audio/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/appearance/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ai/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/combat/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/celestial/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/character/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/content/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/construction/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/debug_tools/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/developer/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/config/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/ecs/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/events/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/logging/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/persistence/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/physics/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/resources/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/factions/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/formation/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/hangar/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/inventory/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/input/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/navigation/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/networking/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/normalization/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/rendering/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ship_editor/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ships/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/weapons/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/trading/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/trade_route/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/rpg/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/crew/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/power/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/mining/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/modeling/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/procedural/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/quest/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/reputation/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/scripting/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/tutorial/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/achievement/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/crafting/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/diplomacy/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/notification/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/research/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/scanning/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/salvage/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/fleet/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/flight/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/cargo/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/economy/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/effects/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/editor/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/station/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/interior/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/integration/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/runtime/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/world/*.cpp"
)

file(GLOB_RECURSE ENGINE_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

# ---------------------------------------------------------------------------
# Engine library
# ---------------------------------------------------------------------------
add_library(subspace_engine STATIC
    ${ENGINE_SOURCES}
    ${ENGINE_HEADERS}
)

target_include_directories(subspace_engine PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

if(SUBSPACE_USE_GLFW)
    target_link_libraries(subspace_engine PUBLIC glfw)
endif()

# ---------------------------------------------------------------------------
# Game executable
# ---------------------------------------------------------------------------
add_executable(subspace_game
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/application/NativeGameApplication.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/application/NativeBattlefieldRenderer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/NativeWindow.cpp"
)

target_link_libraries(subspace_game PRIVATE
    subspace_engine
)

if(WIN32)
    # Keep the CMake-generated Windows target consistent with the checked-in
    # Visual Studio project, which already uses the Unicode character set.
    # NativeWindow also uses explicit W-suffixed Win32 APIs.
    target_compile_definitions(subspace_game PRIVATE UNICODE _UNICODE)
    target_link_libraries(subspace_game PRIVATE OpenGL::GL windowscodecs ole32)
endif()

# ---------------------------------------------------------------------------
# Shipyard offline certification tool
# ---------------------------------------------------------------------------
add_executable(subspace_shipyard_certifier
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/shipyard_certifier.cpp"
)
target_link_libraries(subspace_shipyard_certifier PRIVATE subspace_engine)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
if(SUBSPACE_BUILD_TESTS)
    enable_testing()

    # Keep the long-running historical regression suite stable and place new
    # milestone acceptance suites in their own executable.  This avoids turning
    # test_main.cpp into an ever-growing compile-time bottleneck while CTest
    # still runs every certified test target.
    add_executable(subspace_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_main.cpp"
    )
    target_link_libraries(subspace_tests PRIVATE subspace_engine)
    add_test(NAME SubspaceTests COMMAND subspace_tests)

    add_executable(subspace_milestone_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/pass246_265_tests.cpp"
    )
    target_link_libraries(subspace_milestone_tests PRIVATE subspace_engine)
    add_test(NAME SubspaceMilestoneTests COMMAND subspace_milestone_tests)

    add_executable(subspace_production_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/pass266_295_tests.cpp"
    )
    target_link_libraries(subspace_production_tests PRIVATE subspace_engine)
    add_test(NAME SubspaceProductionTests COMMAND subspace_production_tests)

    add_executable(subspace_forward_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/pass296_315_tests.cpp"
    )
    target_link_libraries(subspace_forward_tests PRIVATE subspace_engine)
    add_test(NAME SubspaceForwardTests COMMAND subspace_forward_tests)

    add_executable(subspace_stabilization_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/pass316_335_tests.cpp"
    )
    target_link_libraries(subspace_stabilization_tests PRIVATE subspace_engine)
    add_test(NAME SubspaceStabilizationTests COMMAND subspace_stabilization_tests)

    add_executable(subspace_visual_foundation_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/pass336_340_tests.cpp"
    )
    target_link_libraries(subspace_visual_foundation_tests PRIVATE subspace_engine)
    add_test(NAME SubspaceVisualFoundationTests COMMAND subspace_visual_foundation_tests)

    add_executable(subspace_flight_visual_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/pass341_346_tests.cpp"
    )
    target_link_libraries(subspace_flight_visual_tests PRIVATE subspace_engine)
    add_test(NAME SubspaceFlightVisualTests COMMAND subspace_flight_visual_tests)

    add_executable(subspace_ship_visual_reconstruction_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/pass347_352_tests.cpp"
    )
    target_link_libraries(subspace_ship_visual_reconstruction_tests PRIVATE subspace_engine)
    add_test(NAME SubspaceShipVisualReconstructionTests COMMAND subspace_ship_visual_reconstruction_tests)

    add_executable(subspace_cinematic_celestial_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/pass353_360_tests.cpp"
    )
    target_link_libraries(subspace_cinematic_celestial_tests PRIVATE subspace_engine)
    add_test(NAME SubspaceCinematicCelestialTests COMMAND subspace_cinematic_celestial_tests)

    add_executable(subspace_command_galaxy_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/pass361_400_tests.cpp"
    )
    target_link_libraries(subspace_command_galaxy_tests PRIVATE subspace_engine)
    add_test(NAME SubspaceCommandGalaxyTests COMMAND subspace_command_galaxy_tests)

    add_executable(subspace_player_facing_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/pass401_410_tests.cpp"
    )
    target_link_libraries(subspace_player_facing_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePlayerFacingTests COMMAND subspace_player_facing_tests)

    add_executable(subspace_player_closure_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/pass411_420_tests.cpp"
    )
    target_link_libraries(subspace_player_closure_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePlayerClosureTests COMMAND subspace_player_closure_tests)

    add_executable(subspace_shipyard_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/pass421_425_tests.cpp"
    )
    target_link_libraries(subspace_shipyard_tests PRIVATE subspace_engine)
    add_test(NAME SubspaceShipyardTests COMMAND subspace_shipyard_tests)

    add_executable(subspace_shipyard_regression_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/pass428_tests.cpp"
    )
    target_link_libraries(subspace_shipyard_regression_tests PRIVATE subspace_engine)
    add_test(NAME SubspaceShipyardRegressionTests COMMAND subspace_shipyard_regression_tests)

    add_executable(subspace_shipyard_planet_repair_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/pass429_tests.cpp"
    )
    target_link_libraries(subspace_shipyard_planet_repair_tests PRIVATE subspace_engine)
    add_test(NAME SubspaceShipyardPlanetRepairTests COMMAND subspace_shipyard_planet_repair_tests)

    add_executable(subspace_shipyard_r5_authority_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/pass456_tests.cpp"
    )
    target_link_libraries(subspace_shipyard_r5_authority_tests PRIVATE subspace_engine)
    add_test(NAME SubspaceShipyardR5AuthorityTests COMMAND subspace_shipyard_r5_authority_tests)

    add_test(NAME SubspaceShipyardR5SourceGate
        COMMAND ${CMAKE_COMMAND}
            -DRENDERER_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/src/application/NativeBattlefieldRenderer.cpp
            -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass456_source_gate.cmake)

    add_executable(subspace_shipyard_r6_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/pass463_tests.cpp"
    )
    target_link_libraries(subspace_shipyard_r6_tests PRIVATE subspace_engine)
    add_test(NAME SubspaceShipyardR6Tests COMMAND subspace_shipyard_r6_tests)

    add_executable(subspace_shipyard_interaction_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/pass468_tests.cpp"
    )
    target_link_libraries(subspace_shipyard_interaction_tests PRIVATE subspace_engine)
    add_test(NAME SubspaceShipyardInteractionTests COMMAND subspace_shipyard_interaction_tests)

    add_test(NAME SubspaceFrontendInputAuthorityGate
        COMMAND ${CMAKE_COMMAND}
            -DAPP_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/src/application/NativeGameApplication.cpp
            -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass468_input_source_gate.cmake)

    add_test(NAME SubspaceShipyardViewportSourceGate
        COMMAND ${CMAKE_COMMAND}
            -DAPP_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/src/application/NativeGameApplication.cpp
            -DRENDERER_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/src/application/NativeBattlefieldRenderer.cpp
            -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass470_shipyard_view_source_gate.cmake)

    add_executable(subspace_shipyard_attachment_material_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/pass471_shipyard_attachment_material_tests.cpp"
    )
    target_link_libraries(subspace_shipyard_attachment_material_tests PRIVATE subspace_engine)
    add_test(NAME SubspaceShipyardAttachmentMaterialTests COMMAND subspace_shipyard_attachment_material_tests)

    add_test(NAME SubspaceShipyardVisualSourceGate
        COMMAND ${CMAKE_COMMAND}
            -DRENDERER_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/src/application/NativeBattlefieldRenderer.cpp
            -DMATERIAL_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/src/rendering/SpaceMaterialSystem.cpp
            -DMODULE_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/src/content/ShipyardModuleSystem.cpp
            -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass471_shipyard_visual_source_gate.cmake)
endif()

# Pass462 update-order regression: older numbered handoffs may not overwrite a
# newer certified repository state.
if(SUBSPACE_BUILD_TESTS)
    add_test(NAME SubspaceUpdatePassOrderGate
        COMMAND ${CMAKE_COMMAND}
            -DUPDATE_SCRIPT=${CMAKE_CURRENT_SOURCE_DIR}/../scripts/subspace_apply_update_inbox.ps1
            -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass462_update_order_gate.cmake)

    add_executable(subspace_shipyard_pass472_tests tests/pass472_shipyard_authority_tests.cpp)
    target_link_libraries(subspace_shipyard_pass472_tests PRIVATE subspace_engine)
    add_test(NAME SubspaceShipyardPass472Tests COMMAND subspace_shipyard_pass472_tests)
    add_test(NAME SubspaceShipyardPass472SourceGate COMMAND ${CMAKE_COMMAND} -DROOT=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass472_shipyard_authority_source_gate.cmake)

    add_executable(subspace_shipyard_pass473_tests tests/pass473_orientation_symmetry_selection_cloud_tests.cpp)
    target_link_libraries(subspace_shipyard_pass473_tests PRIVATE subspace_engine)
    add_test(NAME SubspaceShipyardPass473Tests COMMAND subspace_shipyard_pass473_tests)
    add_test(NAME SubspaceShipyardPass473SourceGate COMMAND ${CMAKE_COMMAND} -DROOT=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass473_visual_source_gate.cmake)
endif()

# Pass474: authored-ship forward authority + interactive semantic System Map + independent clouds.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_pass474_tests tests/pass474_map_orientation_weather_tests.cpp)
    target_link_libraries(subspace_pass474_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass474Tests COMMAND subspace_pass474_tests)
    add_test(NAME SubspacePass474SourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass474_source_gate.cmake)
endif()

# Pass475: source-faithful planet filtering/LOD telemetry + native Shipyard livery/decal foundation.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_pass475_tests tests/pass475_planet_livery_tests.cpp)
    target_link_libraries(subspace_pass475_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass475Tests COMMAND subspace_pass475_tests)
    add_test(NAME SubspacePass475SourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass475_source_gate.cmake)
endif()

# Pass476-495: physical station docking, lock-first fire control, ship progression,
# spatial Shipyard authoring, propulsion/warp presentation, living weather and
# EVE-familiar Contacts/command-HUD foundations.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_pass476_495_tests tests/pass476_495_systems_tests.cpp)
    target_link_libraries(subspace_pass476_495_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass476To495Tests COMMAND subspace_pass476_495_tests)
    add_test(NAME SubspacePass476To495SourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass476_495_source_gate.cmake)
endif()

# Pass496: canonical solar-system spawn grammar and stellar/celestial exclusion closure.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_pass496_tests tests/pass496_solar_system_placement_tests.cpp)
    target_link_libraries(subspace_pass496_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass496SolarSystemPlacementTests COMMAND subspace_pass496_tests)
    add_test(NAME SubspacePass496SolarSystemPlacementSourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass496_source_gate.cmake)
endif()

# Pass497-500: user-verified wing-root authority, click-vs-drag Shipyard UX,
# synchronized selection, semantic material paint zones and root diagnostics.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_pass497_500_tests tests/pass497_500_shipyard_visual_authority_tests.cpp)
    target_link_libraries(subspace_pass497_500_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass497To500ShipyardVisualAuthorityTests COMMAND subspace_pass497_500_tests)
    add_test(NAME SubspacePass497To500ShipyardVisualAuthoritySourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass497_500_source_gate.cmake)
endif()

# Pass506R3: Shipyard GUI/material/workflow visual-certification closure.
# Verifies non-overlapping hit rectangles, full-axis transform controls,
# Greyoxide Flat/Flat.001 semantic material coverage and the user-certified
# Wing147 source-basis correction.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_pass506r3_tests tests/pass506r3_shipyard_gui_material_tests.cpp)
    target_link_libraries(subspace_pass506r3_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass506R3ShipyardWorkflowClosureTests COMMAND subspace_pass506r3_tests)

    add_executable(subspace_pass506r4_tests tests/pass506r4_shipyard_gui_polish_tests.cpp)
    target_link_libraries(subspace_pass506r4_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass506R4ShipyardGuiPolishTests COMMAND subspace_pass506r4_tests)

    add_executable(subspace_pass506r5_tests tests/pass506r5_shipyard_presentation_tests.cpp)
    target_link_libraries(subspace_pass506r5_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass506R5ShipyardPresentationTests COMMAND subspace_pass506r5_tests)

    add_executable(subspace_pass506r6_tests tests/pass506r6_shipyard_authoring_export_tests.cpp)
    target_link_libraries(subspace_pass506r6_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass506R6ShipyardAuthoringExportTests COMMAND subspace_pass506r6_tests)

    add_executable(subspace_pass506r7_tests tests/pass506r7_universal_build_controls_tests.cpp)
    target_link_libraries(subspace_pass506r7_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass506R7UniversalBuildControlsTests COMMAND subspace_pass506r7_tests)
endif()

# Pass513-522: combat/HUD truth, spawn and travel safety, deep-space presentation,
# station kitbash runtime, explicit interior/on-foot play, docking guidance and camera polish.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_pass513_522_tests tests/pass513_522_project_continuation_tests.cpp)
    target_link_libraries(subspace_pass513_522_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass513To522ProjectContinuationTests COMMAND subspace_pass513_522_tests)
    add_test(NAME SubspacePass513To522SourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass513_522_source_gate.cmake)
endif()

# Pass523-532: station runtime QA, ecology, imported planets, Vector timing,
# authored interiors/interactions, dock services, damage coupling, living traffic,
# and normalized camera/control/UI context.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_pass523_532_tests tests/pass523_532_project_continuation_tests.cpp)
    target_link_libraries(subspace_pass523_532_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass523To532ProjectContinuationTests COMMAND subspace_pass523_532_tests)
    add_test(NAME SubspacePass523To532SourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass523_532_source_gate.cmake)
endif()

# Pass533A: post-audit authority convergence foundation. Hard rear-drive
# propulsion rules, always-visible Shipyard forward authority, and live
# CanonicalAsset identity projection for the certified Shipyard catalog.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_pass533a_tests tests/pass533a_universal_authority_propulsion_tests.cpp)
    target_link_libraries(subspace_pass533a_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass533AUniversalAuthorityPropulsionTests COMMAND subspace_pass533a_tests)
    add_test(NAME SubspacePass533AUniversalAuthorityPropulsionSourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass533a_universal_authority_propulsion_source_gate.cmake)
endif()

# Pass533B: in-game reusable socket authoring. Socket origin/frame/type edits are
# direct-manipulation capable, attachment-aware, undoable, and persisted as
# non-destructive authoring overrides over the certified module catalog.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_pass533b_tests tests/pass533b_socket_authoring_tests.cpp)
    target_link_libraries(subspace_pass533b_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass533BSocketAuthoringTests COMMAND subspace_pass533b_tests)
    add_test(NAME SubspacePass533BSocketAuthoringSourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass533b_socket_authoring_source_gate.cmake)
endif()

# Pass534: live Teach-PCG reusable definition authoring. Semantic classification,
# generator eligibility, pairing intent and mount-face corrections persist as
# non-destructive sidecars and refresh the canonical asset/PCG authority.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_pass534_tests tests/pass534_teach_pcg_definition_authoring_tests.cpp)
    target_link_libraries(subspace_pass534_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass534TeachPcgDefinitionAuthoringTests COMMAND subspace_pass534_tests)
    add_test(NAME SubspacePass534TeachPcgDefinitionAuthoringSourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass534_teach_pcg_definition_authoring_source_gate.cmake)
endif()

# Pass535-554: project-wide editor/UI authority and Shipyard V2 foundation.
if(SUBSPACE_BUILD_TESTS)
 add_executable(subspace_pass535_554_tests tests/pass535_554_editor_shipyard_upgrade_tests.cpp)
 target_link_libraries(subspace_pass535_554_tests PRIVATE subspace_engine)
 add_test(NAME SubspacePass535To554EditorShipyardUpgradeTests COMMAND subspace_pass535_554_tests)
 add_test(NAME SubspacePass535To554EditorShipyardUpgradeSourceGate COMMAND ${CMAKE_COMMAND} -DROOT=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass535_554_editor_shipyard_upgrade_source_gate.cmake)

 add_executable(subspace_pass555_584_tests tests/pass555_584_projectwide_editor_station_upgrade_tests.cpp)
 target_link_libraries(subspace_pass555_584_tests PRIVATE subspace_engine)
 add_test(NAME SubspacePass555To584ProjectWideEditorStationUpgradeTests COMMAND subspace_pass555_584_tests)
 add_test(NAME SubspacePass555To584ProjectWideEditorStationUpgradeSourceGate COMMAND ${CMAKE_COMMAND} -DROOT=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass555_584_projectwide_editor_station_upgrade_source_gate.cmake)
endif()

# Pass585-594: visible Shipyard V2 realization and socket-authoring recovery.
# Restores one-click socket correction, introduces resolution-aware layout,
# real-geometry module cards and floating contextual help without regressing
# the project-wide editor authority established in Pass535-584.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_pass585_594_tests tests/pass585_594_shipyard_visible_ui_socket_recovery_tests.cpp)
    target_link_libraries(subspace_pass585_594_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass585To594ShipyardVisibleUiSocketRecoveryTests COMMAND subspace_pass585_594_tests)
    add_test(NAME SubspacePass585To594ShipyardVisibleUiSocketRecoverySourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass585_594_shipyard_visible_ui_socket_recovery_source_gate.cmake)
endif()

# Pass595-614: universal kitbash construction normalization. True mirroring,
# cursor-carried placement ghosts, free/snap build, XS-XL morph families,
# propulsion/turret taxonomy, whole-ship spatial authority, semantic appearance,
# faction/class hull families, required ship capabilities and governed Full Gate intake.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_pass595_614_tests tests/pass595_614_kitbash_construction_upgrade_tests.cpp)
    target_link_libraries(subspace_pass595_614_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass595To614KitbashConstructionUpgradeTests COMMAND subspace_pass595_614_tests)
    add_test(NAME SubspacePass595To614KitbashConstructionUpgradeSourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass595_614_kitbash_construction_upgrade_source_gate.cmake)
endif()

# Pass615-654: runtime closure for universal kitbash construction. Whole-ship
# spatial/propulsion certification, XS-XL live morph placement, faction hull
# lineage/roles, semantic appearance, universal construction blueprints,
# manufacturing/reverse engineering and project-wide kitbash review catalogs.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_pass615_654_tests tests/pass615_654_kitbash_runtime_closure_tests.cpp)
    target_link_libraries(subspace_pass615_654_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass615To654KitbashRuntimeClosureTests COMMAND subspace_pass615_654_tests)
    add_test(NAME SubspacePass615To654KitbashRuntimeClosureSourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass615_654_kitbash_runtime_closure_source_gate.cmake)
endif()

# Pass655-674: exact universal construction symmetry, centered/6DOF construction
# camera navigation, and normalized canonical-asset thumbnail metadata/cache UX.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_pass655_674_tests tests/pass655_674_editor_symmetry_camera_thumbnail_tests.cpp)
    target_link_libraries(subspace_pass655_674_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass655To674EditorSymmetryCameraThumbnailTests COMMAND subspace_pass655_674_tests)
    add_test(NAME SubspacePass655To674EditorSymmetryCameraThumbnailSourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass655_674_editor_symmetry_camera_thumbnail_source_gate.cmake)
endif()

# Pass735-744: cumulative in-game Shipyard Dev Studio closure (interiors,
# character/player-scale lane and Dev World) plus Havenwild-derived Root Project
# Control Center v2 custody, gate records, artifact indexing and transactional
# manifest-driven root patch intake/undo.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_pass735_744_tests tests/pass735_744_shipyard_dev_pcc_tests.cpp)
    target_link_libraries(subspace_pass735_744_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass735To744ShipyardDevPccTests COMMAND subspace_pass735_744_tests)
    add_test(NAME SubspacePass735To744ShipyardDevPccSourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass735_744_shipyard_dev_pcc_source_gate.cmake)
endif()

# Pass746: centered/dynamic construction framing, flight/camera decoupling,
# profile-following one-foot pond-surface shield with localized impact ripples,
# semantic kitbash cards and immediate-mode render hot-path reductions.
if(SUBSPACE_BUILD_TESTS)
    add_test(NAME SubspacePass746ShipyardViewportShieldPerfSourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass746_shipyard_viewport_shield_perf_source_gate.cmake)
endif()

# Pass746R2: normalized GitHub/main authority. Current native C++ source is
# published only after GREEN certification; stale remote main is archived
# before a force-with-lease replacement and retired C# remains non-authority.
if(SUBSPACE_BUILD_TESTS)
    add_test(NAME SubspacePass746R2RepositoryAuthoritySourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass746r2_repository_authority_source_gate.cmake)
endif()


# Pass746R3: persistent Control Center palette ownership, ANSI-safe child output,
# stale -Root binding-residue cleanup, and source-rollup operational-root hygiene.
if(SUBSPACE_BUILD_TESTS)
    add_test(NAME SubspacePass746R3ControlCenterPaletteRootCleanupSourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass746r3_control_center_palette_root_cleanup_source_gate.cmake)
endif()

# Pass746R4: universal ProjectOps TreeStage (Robocopy/portable backend),
# repository-authority Git argument repair, first-time Git flow UX, and
# action-aware debug certification status.
if(SUBSPACE_BUILD_TESTS)
    add_test(NAME SubspacePass746R4ProjectOpsTreeStageGitFlowSourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass746r4_projectops_tree_stage_git_flow_source_gate.cmake)
endif()

# Pass746R5: single artifacts/ output authority, legacy output migration,
# failure-time Explorer handoff of the current debug ZIP, and Git stderr repair.
if(SUBSPACE_BUILD_TESTS)
    add_test(NAME SubspacePass746R5ArtifactAuthorityFailureHandoffSourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass746r5_artifact_authority_failure_handoff_source_gate.cmake)
endif()

# Pass746R6: absorb/refactor mature Havenwild root-tooling patterns into a
# project-neutral ProjectOps/Forge machine spine and independent source authority.
if(SUBSPACE_BUILD_TESTS)
    add_test(NAME SubspacePass746R6HavenwildProjectOpsAbsorptionSourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass746r6_havenwild_projectops_absorption_source_gate.cmake)
endif()

# Pass746R6R1: proper Havenwild donor absorption into the shared ProjectOps root
# tooling spine; one source/root authority, no Python caches, no parallel policy.
if(SUBSPACE_BUILD_TESTS)
    add_test(NAME SubspacePass746R6R1HavenwildRootToolingRefactorSourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass746r6r1_havenwild_root_tooling_refactor_source_gate.cmake)
endif()

# Pass746R6R2: close remaining root-tooling authority drift after Havenwild
# absorption: native CMake authority, canonical artifact paths, normalized
# installer payload, and quality-gate history location.
if(SUBSPACE_BUILD_TESTS)
    add_test(NAME SubspacePass746R6R2RootToolingAuthorityDriftSourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass746r6r2_root_tooling_authority_drift_source_gate.cmake)
endif()

# Pass746R6R3: modernize the historical Pass735-744 PCC source gate so
# source-rollup safety follows the shared ProjectOps authority rather than a
# retired literal inside SubspaceTools.ps1.
if(SUBSPACE_BUILD_TESTS)
    add_test(NAME SubspacePass746R6R3HistoricalPccProjectOpsSourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass746r6r3_historical_pcc_projectops_source_gate.cmake)
endif()

# Pass746R6R4: finish historical PCC/root-audit delegation and make initialized
# but unborn Git repositories safe for quality-gate records and control audits.
if(SUBSPACE_BUILD_TESTS)
    add_test(NAME SubspacePass746R6R4HistoricalRootAuditUnbornGitSourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass746r6r4_historical_root_audit_unborn_git_source_gate.cmake)
endif()

# Pass746R6R5: keep destructive publication confirmation in the interactive
# root host and pass explicit authorization through non-interactive child layers.
if(SUBSPACE_BUILD_TESTS)
    add_test(NAME SubspacePass746R6R5InteractivePublishBridgeSourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass746r6r5_interactive_publish_bridge_source_gate.cmake)
endif()

# Pass746R6R6: modernize the original repository-authority source gate after
# publication confirmation moved from the nested publisher to the interactive
# root Control Center.
if(SUBSPACE_BUILD_TESTS)
    add_test(NAME SubspacePass746R6R6HistoricalRepositoryPublishGateSourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass746r6r6_historical_repository_publish_gate_source_gate.cmake)
endif()

# ProjectOps static certification authority. Tooling/root-control validation
# gates live under tools/control/static-gates and are discovered at CTest
# runtime, so adding a new tooling gate no longer invalidates the native CMake
# object graph.
if(SUBSPACE_BUILD_TESTS)
    add_test(NAME SubspaceProjectOpsStaticCertification COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/../tools/control/ProjectOpsStaticCertification.cmake)
endif()

