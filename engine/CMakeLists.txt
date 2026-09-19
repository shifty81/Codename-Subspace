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
    "${CMAKE_CURRENT_SOURCE_DIR}/src/generator/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/station/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/interior/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/integration/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/runtime/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/world/*.cpp"
)

# Foundation Convergence P922+: the legacy HomeSystemSaveGame translation unit
# calls detached home/expedition implementation symbols. Keep the source as
# migration reference, but do not place a latent unresolved-object trap in the
# production static library.
list(FILTER ENGINE_SOURCES EXCLUDE REGEX ".*/core/persistence/HomeSystemSaveGame\\.cpp$")

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
# Native application shell: compile the shared C++ window / renderer / app
# implementation once, then link independent game and Studio entry points.
# This is a transitional shared host, NOT a second simulation or an egui fork.
# ---------------------------------------------------------------------------
add_library(subspace_native_host OBJECT
    "${CMAKE_CURRENT_SOURCE_DIR}/src/application/NativeGameApplication.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/application/NativeBattlefieldRenderer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/NativeWindow.cpp"
)
target_link_libraries(subspace_native_host PRIVATE subspace_engine)

add_executable(subspace_game
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
)
target_link_libraries(subspace_game PRIVATE subspace_native_host subspace_engine)

# Dedicated desktop process. Launch defaults are permanently editor-only;
# this entrypoint never accepts the game's gameplay/runtime-smoke switches.
add_executable(subspace_studio
    "${CMAKE_CURRENT_SOURCE_DIR}/src/studio_main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/studio/StudioApplication.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/studio/StudioDocumentStore.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/studio/StudioFileDialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/studio/StudioNativeCloseGuard.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/studio/StudioAxisGizmo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/studio/StudioGizmoOverlay.cpp"
)
target_link_libraries(subspace_studio PRIVATE subspace_native_host subspace_engine)

if(WIN32)
    # The shared host and both executables use the same Unicode Win32 ABI.
    # Keep native OpenGL/WIC/COM confined to executable dependencies.
    target_compile_definitions(subspace_native_host PRIVATE UNICODE _UNICODE)
    foreach(_subspace_native_executable subspace_game subspace_studio)
        target_compile_definitions(${_subspace_native_executable} PRIVATE UNICODE _UNICODE)
        target_link_libraries(${_subspace_native_executable} PRIVATE OpenGL::GL windowscodecs ole32)
    endforeach()
    # Win32 common document dialogs are Studio-owned, not gameplay dependencies.
    target_link_libraries(subspace_studio PRIVATE comdlg32 user32 comctl32)
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

# Pass790R2: governed Shipyard content must load without the retired Pass336
# module-root prerequisite; standalone Shipyard shield preview and XS-XL
# generation authority are source-gated here.
if(SUBSPACE_BUILD_TESTS)
    add_test(NAME SubspacePass790ShipyardAuthorityRecoverySourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass790_shipyard_authority_recovery_source_gate.cmake)
endif()


# Pass892-901: project-wide editor docking normalization. One canonical dock
# tree owns panel visibility, tabbing, splitting, floating and redocking while
# historical fixed-layout systems remain compatibility projections.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_pass892_901_tests tests/pass892_901_editor_dock_normalization_tests.cpp)
    target_link_libraries(subspace_pass892_901_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass892To901EditorDockNormalizationTests COMMAND subspace_pass892_901_tests)
endif()

# Pass912-921: one native Shipyard editor application with Blender-like
# workspaces, canonical editor/client parity, EVE-depth-inspired but original
# character customization contracts, and bounded Carbon open-source adoption.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_pass912_921_tests tests/pass912_921_shipyard_editor_character_parity_tests.cpp)
    target_link_libraries(subspace_pass912_921_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass912To921ShipyardEditorCharacterParityTests COMMAND subspace_pass912_921_tests)
endif()

# Pass922-931: Foundation Convergence A. Full-3D authority, legacy sector-jump
# quarantine, stable persistent identity, SpatialFrame v2, persistence schema
# contract, and P921 performance-reference protection.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_pass922_931_tests tests/pass922_931_foundation_convergence_tests.cpp)
    target_link_libraries(subspace_pass922_931_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass922To931FoundationConvergenceTests COMMAND subspace_pass922_931_tests)
endif()

# Pass932-941: promotion fingerprint stability and editor-camera continuity.
# GREEN promotion fingerprints represent actual working-tree bytes independent
# of staging state; Shipyard wheel dolly preserves the user's panned/orbit pivot.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_pass932_941_tests tests/pass932_941_promotion_camera_stability_tests.cpp)
    target_link_libraries(subspace_pass932_941_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass932To941PromotionCameraStabilityTests COMMAND subspace_pass932_941_tests)

    add_executable(subspace_pass942_951_tests tests/pass942_951_interior_drag_authority_tests.cpp)
    target_link_libraries(subspace_pass942_951_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass942To951InteriorDragAuthorityTests COMMAND subspace_pass942_951_tests)
endif()

# Pass973-997: Runtime Composition + Assembly Authority A. Consolidates
# representation-independent world identity/frame/residency/persistence state
# under NativeRuntimeServices, then establishes the canonical editable assembly,
# BuildCommand journal and persistent-ship Shipyard transaction boundary.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_pass973_997_tests tests/pass973_997_runtime_assembly_authority_tests.cpp)
    target_link_libraries(subspace_pass973_997_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass973To997RuntimeAssemblyAuthorityTests COMMAND subspace_pass973_997_tests)
endif()


# Pass998-1022: canonical Shipyard/runtime product bridge and explicit world
# streaming handoffs. Existing certified recipes import into AssemblyDefinition;
# renderer/collision/interior/nav products project from that one source, while
# destination prefetch becomes an async residency/representation transaction.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_pass998_1022_tests tests/pass998_1022_shipyard_streaming_integration_tests.cpp)
    target_link_libraries(subspace_pass998_1022_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass998To1022ShipyardStreamingIntegrationTests COMMAND subspace_pass998_1022_tests)
endif()

# Pass1023-1047: visible-fidelity recovery plus hard ship class/XS-XL authority
# and Blender-style staged placement. Planet source textures/cloud layers are
# Full-Gate materialized, missing ship textures receive deterministic material
# fallbacks, and palette drags must be transformed/snapped before confirmation.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_pass1023_1047_tests tests/pass1023_1047_visual_class_blender_workflow_tests.cpp)
    target_link_libraries(subspace_pass1023_1047_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass1023To1047VisualClassBlenderWorkflowTests COMMAND subspace_pass1023_1047_tests)
    add_test(NAME SubspacePass1023To1047VisualClassBlenderSourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass1023_1047_visual_class_blender_source_gate.cmake)
endif()

# Pass1048-1097: measured-surface authoring, character-scale semantic objects,
# cohesive assembly/shield authority, FPS/remote-fleet view semantics, universal
# dockable dark UI, and one runtime/Blender generator request registry.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_pass1048_1097_tests tests/pass1048_1097_authoring_runtime_ui_parity_tests.cpp)
    target_link_libraries(subspace_pass1048_1097_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass1048To1097AuthoringRuntimeUiParityTests COMMAND subspace_pass1048_1097_tests)
    add_test(NAME SubspacePass1048To1097AuthoringRuntimeUiParitySourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass1048_1097_authoring_runtime_ui_parity_source_gate.cmake)
endif()

# Pass1098-1122: emergency generator normalization. GENERATE is deterministic,
# only REROLL mutates seed, class size can no longer be achieved through huge
# whole-ship scaling, invalid candidates fail closed, bounds are truly 3D, and
# the old station stacking generator is quarantined as compatibility preview.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_pass1098_1122_tests tests/pass1098_1122_generation_foundation_normalization_tests.cpp)
    target_link_libraries(subspace_pass1098_1122_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass1098To1122GenerationFoundationNormalizationTests COMMAND subspace_pass1098_1122_tests)
    add_test(NAME SubspacePass1098To1122GenerationFoundationSourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass1098_1122_generation_foundation_source_gate.cmake)
endif()

# Pass1123-1162: core authority convergence. Seed-stable generation exposes
# bounded deterministic drafts without reviving scale cheating; Shipyard input
# defaults to view/camera space, authoring mutations share one undo/redo history,
# and Forge/PCC project operations surface through a native project-contract
# thin client rather than embedding ForgePY or guessing command interpreters.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_pass1123_1162_tests tests/pass1123_1162_core_authority_convergence_tests.cpp)
    target_link_libraries(subspace_pass1123_1162_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass1123To1162CoreAuthorityConvergenceTests COMMAND subspace_pass1123_1162_tests)
    add_test(NAME SubspacePass1123To1162CoreAuthorityConvergenceSourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/pass1123_1162_core_authority_convergence_source_gate.cmake)
endif()

# Pass1466-1505: Shipyard Foundry. Viewport navigation continuity, modular DCC
# panels, material-health certification, articulated equipment, interior
# structure authoring, advanced paint finishes and model-authoring primitives.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_pass1466_1505_tests tests/pass1466_1505_shipyard_foundry_tests.cpp)
    target_link_libraries(subspace_pass1466_1505_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass1466To1505ShipyardFoundryTests COMMAND subspace_pass1466_1505_tests)
endif()

# PASS1507: execute the production dock implementation's active-tab regression
# directly, independently of the full game link graph. CTest/Full Gate owns it.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_pass1507_dock_tests
        src/ui/SubspaceUiFramework.cpp
        tests/pass1507_dock_active_tab_tests.cpp)
    target_include_directories(subspace_pass1507_dock_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_compile_features(subspace_pass1507_dock_tests PRIVATE cxx_std_17)
    add_test(NAME SubspacePass1507DockActiveTabTests COMMAND subspace_pass1507_dock_tests)
endif()

# PASS1508: actual native pointer gesture routing exercised on the same dock
# authority the editor uses. Does not depend on the graphics window.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_pass1508_dock_pointer_tests
        src/ui/SubspaceUiFramework.cpp
        tests/pass1508_dock_pointer_tests.cpp)
    target_include_directories(subspace_pass1508_dock_pointer_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_compile_features(subspace_pass1508_dock_pointer_tests PRIVATE cxx_std_17)
    add_test(NAME SubspacePass1508DockPointerTests COMMAND subspace_pass1508_dock_pointer_tests)
endif()

# PASS1508R3: certify the real Shipyard default dock tree and shelf/rail
# independence, not merely a synthetic minimal workspace.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_pass1508r3_dock_region_tests
        tests/pass1508r3_dock_region_tests.cpp)
    target_link_libraries(subspace_pass1508r3_dock_region_tests PRIVATE subspace_engine)
    add_test(NAME SubspacePass1508R3DockRegionTests COMMAND subspace_pass1508r3_dock_region_tests)
endif()

# PASS1508R4: prove the model-less DCC shell cannot regress to painting
# the Asset Browser beneath the rail; runtime dock tests are owned by R3.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_pass1508r4_shell_geometry_tests
        src/editor/EditorDccShellLayoutSystem.cpp
        src/editor/EditorForgeGuiStyleSystem.cpp
        tests/pass1508r4_shell_geometry_tests.cpp)
    target_include_directories(subspace_pass1508r4_shell_geometry_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_compile_features(subspace_pass1508r4_shell_geometry_tests PRIVATE cxx_std_17)
    add_test(NAME SubspacePass1508R4ShellGeometryTests COMMAND subspace_pass1508r4_shell_geometry_tests)
endif()

# PASS1508R5: one compositor authority and continuous editor camera navigation.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_pass1508r5_gui_camera_tests
        src/core/Math.cpp
        src/ui/SubspaceUiFramework.cpp
        src/editor/ConstructionEditorCameraSystem.cpp
        src/rendering/StrategicCamera.cpp
        src/rendering/StrategicViewProjection.cpp
        tests/pass1508r5_gui_camera_normalization_tests.cpp)
    target_include_directories(subspace_pass1508r5_gui_camera_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_compile_features(subspace_pass1508r5_gui_camera_tests PRIVATE cxx_std_17)
    add_test(NAME SubspacePass1508R5GuiCameraNormalizationTests COMMAND subspace_pass1508r5_gui_camera_tests)
endif()

if(SUBSPACE_BUILD_TESTS)
    add_test(NAME SubspacePass1508R5CompositorCameraSourceGate COMMAND ${CMAKE_COMMAND}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/../tools/control/static-gates/pass1508r5_compositor_camera_authority.cmake)
endif()

# SHIPYARD-OVERLAY-SLICE1: active overlay canvas/input/layout persistence contract.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_shipyard_overlay_foundation_tests
        src/ui/SubspaceUiFramework.cpp
        tests/shipyard_overlay_foundation_tests.cpp)
    target_include_directories(subspace_shipyard_overlay_foundation_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_compile_features(subspace_shipyard_overlay_foundation_tests PRIVATE cxx_std_17)
    add_test(NAME SubspaceShipyardOverlayFoundationTests COMMAND subspace_shipyard_overlay_foundation_tests)
endif()

# HOLLOW-SHELL SLICE 1: exercised production carve -> union/portal mesh ->
# shared visual/collision surface authority. Independent of an OpenGL window.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_derived_hollow_shell_tests
        tests/shipyard_derived_hollow_shell_tests.cpp)
    target_link_libraries(subspace_derived_hollow_shell_tests PRIVATE subspace_engine)
    add_test(NAME SubspaceDerivedHollowShellTests COMMAND subspace_derived_hollow_shell_tests)
endif()

# Hollow Shell Slice 2: production collision consumes exact shared render quads.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_hollow_shell_runtime_tests
        src/interior/ShipInteriorDerivedShellSystem.cpp
        src/interior/ShipInteriorShellTraversalSystem.cpp
        tests/shipyard_hollow_shell_runtime_tests.cpp)
    target_include_directories(subspace_hollow_shell_runtime_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_compile_features(subspace_hollow_shell_runtime_tests PRIVATE cxx_std_17)
    add_test(NAME SubspaceHollowShellRuntimeTests COMMAND subspace_hollow_shell_runtime_tests)
endif()

if(SUBSPACE_BUILD_TESTS)
    add_test(NAME SubspaceHollowShellRuntimeSourceGate COMMAND ${CMAKE_COMMAND}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/../tools/control/static-gates/hollow_shell_slice2_runtime_authority.cmake)
endif()

# SHIPYARD GUI RECOVERY: shared Asset Browser geometry and last-card reachability.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_shipyard_gui_recovery_tests tests/shipyard_gui_recovery_tests.cpp)
    target_include_directories(subspace_shipyard_gui_recovery_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_compile_features(subspace_shipyard_gui_recovery_tests PRIVATE cxx_std_17)
    add_test(NAME SubspaceShipyardGuiRecoveryTests COMMAND subspace_shipyard_gui_recovery_tests)
endif()

# Shipyard UI regression: Assets can always be reopened from global chrome,
# including persisted closed/collapsed tabs, floating windows, and maximized view.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_asset_visibility_recovery_tests
        tests/shipyard_asset_visibility_recovery_tests.cpp)
    target_link_libraries(subspace_asset_visibility_recovery_tests PRIVATE subspace_engine)
    add_test(NAME SubspaceAssetVisibilityRecoveryTests COMMAND subspace_asset_visibility_recovery_tests)
endif()

# G1: shared selected-module XYZ manipulation, including projected-pick agreement.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_unified_gizmo_g1_tests tests/shipyard_unified_gizmo_g1_tests.cpp)
    target_link_libraries(subspace_unified_gizmo_g1_tests PRIVATE subspace_engine)
    add_test(NAME SubspaceUnifiedGizmoG1Tests COMMAND subspace_unified_gizmo_g1_tests)
    add_test(NAME SubspaceUnifiedGizmoG1SourceGate COMMAND ${CMAKE_COMMAND}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/../tools/control/static-gates/shipyard_unified_gizmo_g1_authority.cmake)
endif()

# G2: Studio empty-document startup without an OpenGL context.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_shipyard_blank_document_g2_tests tests/shipyard_blank_document_g2_tests.cpp)
    target_include_directories(subspace_shipyard_blank_document_g2_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_compile_features(subspace_shipyard_blank_document_g2_tests PRIVATE cxx_std_17)
    add_test(NAME SubspaceShipyardBlankDocumentG2Tests COMMAND subspace_shipyard_blank_document_g2_tests)
    add_test(NAME SubspaceShipyardBlankDocumentG2SourceGate COMMAND ${CMAKE_COMMAND}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/../tools/control/static-gates/shipyard_blank_document_g2_authority.cmake)
endif()

# G3 editor camera / view modes: standalone source-level test, no OpenGL required.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_shipyard_studio_g3_tests
        tests/shipyard_studio_g3_workflow_tests.cpp
        src/editor/ConstructionEditorCameraSystem.cpp
        src/core/Math.cpp)
    target_include_directories(subspace_shipyard_studio_g3_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_compile_features(subspace_shipyard_studio_g3_tests PRIVATE cxx_std_17)
    add_test(NAME SubspaceShipyardStudioG3Tests COMMAND subspace_shipyard_studio_g3_tests)
    add_test(NAME SubspaceShipyardStudioG3SourceGate COMMAND ${CMAKE_COMMAND}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/../tools/control/static-gates/shipyard_studio_g3_editor_workflow.cmake")
endif()

# STUDIO-S01: This lightweight test exercises the dedicated editor entrypoint
# without requiring a GPU, game session, renderer initialization, or Windows.
if(SUBSPACE_BUILD_TESTS)
    add_test(NAME SubspaceStudioEntrypointHelp COMMAND subspace_studio --help)
    add_executable(subspace_studio_transform_readout_tests "${CMAKE_CURRENT_SOURCE_DIR}/tests/studio_transform_readout_tests.cpp")
    target_include_directories(subspace_studio_transform_readout_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_compile_features(subspace_studio_transform_readout_tests PRIVATE cxx_std_17)
    add_test(NAME SubspaceStudioTransformReadoutTests COMMAND subspace_studio_transform_readout_tests)
    add_executable(subspace_studio_measurement_format_tests "${CMAKE_CURRENT_SOURCE_DIR}/tests/studio_measurement_format_tests.cpp")
    target_include_directories(subspace_studio_measurement_format_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_compile_features(subspace_studio_measurement_format_tests PRIVATE cxx_std_17)
    add_test(NAME SubspaceStudioMeasurementFormatTests COMMAND subspace_studio_measurement_format_tests)
    add_executable(subspace_studio_camera_navigation_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/studio_camera_navigation_tests.cpp")
    target_link_libraries(subspace_studio_camera_navigation_tests PRIVATE subspace_engine)
    add_test(NAME SubspaceStudioCameraNavigationTests COMMAND subspace_studio_camera_navigation_tests)
    add_executable(subspace_studio_gizmo_math_tests "${CMAKE_CURRENT_SOURCE_DIR}/tests/studio_gizmo_math_tests.cpp")
    target_include_directories(subspace_studio_gizmo_math_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_compile_features(subspace_studio_gizmo_math_tests PRIVATE cxx_std_17)
    add_test(NAME SubspaceStudioGizmoMathTests COMMAND subspace_studio_gizmo_math_tests)
    add_executable(subspace_studio_close_policy_tests "${CMAKE_CURRENT_SOURCE_DIR}/tests/studio_close_policy_tests.cpp")
    target_include_directories(subspace_studio_close_policy_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_compile_features(subspace_studio_close_policy_tests PRIVATE cxx_std_17)
    add_test(NAME SubspaceStudioClosePolicyTests COMMAND subspace_studio_close_policy_tests)
    add_executable(subspace_studio_native_close_guard_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/studio_native_close_guard_tests.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/studio/StudioNativeCloseGuard.cpp")
    target_include_directories(subspace_studio_native_close_guard_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_compile_features(subspace_studio_native_close_guard_tests PRIVATE cxx_std_17)
    if(WIN32)
        target_link_libraries(subspace_studio_native_close_guard_tests PRIVATE comctl32 user32)
    endif()
    add_test(NAME SubspaceStudioNativeCloseGuardTests COMMAND subspace_studio_native_close_guard_tests)
    add_executable(subspace_studio_document_recovery_tests "${CMAKE_CURRENT_SOURCE_DIR}/tests/studio_document_recovery_tests.cpp")
    target_include_directories(subspace_studio_document_recovery_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_compile_features(subspace_studio_document_recovery_tests PRIVATE cxx_std_17)
    add_test(NAME SubspaceStudioDocumentRecoveryTests COMMAND subspace_studio_document_recovery_tests)
    add_executable(subspace_studio_document_shortcuts_tests "${CMAKE_CURRENT_SOURCE_DIR}/tests/studio_document_shortcuts_tests.cpp")
    target_include_directories(subspace_studio_document_shortcuts_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_compile_features(subspace_studio_document_shortcuts_tests PRIVATE cxx_std_17)
    add_test(NAME SubspaceStudioDocumentShortcutsTests COMMAND subspace_studio_document_shortcuts_tests)
    add_executable(subspace_studio_session_tests "${CMAKE_CURRENT_SOURCE_DIR}/tests/studio_session_tests.cpp")
    target_link_libraries(subspace_studio_session_tests PRIVATE subspace_engine)
    add_test(NAME SubspaceStudioSessionTests COMMAND subspace_studio_session_tests)
    add_executable(subspace_studio_modeling_geometry_tests "${CMAKE_CURRENT_SOURCE_DIR}/tests/studio_modeling_geometry_tests.cpp")
    target_link_libraries(subspace_studio_modeling_geometry_tests PRIVATE subspace_engine)
    add_test(NAME SubspaceStudioModelingGeometryTests COMMAND subspace_studio_modeling_geometry_tests)
    add_executable(subspace_studio_document_roundtrip_tests "${CMAKE_CURRENT_SOURCE_DIR}/tests/studio_document_roundtrip_tests.cpp")
    target_link_libraries(subspace_studio_document_roundtrip_tests PRIVATE subspace_engine)
    add_test(NAME SubspaceStudioDocumentRoundtripTests COMMAND subspace_studio_document_roundtrip_tests)
endif()
