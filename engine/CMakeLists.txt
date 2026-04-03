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
option(SUBSPACE_BUILD_TESTS "Build the test suite" ON)
option(SUBSPACE_USE_GLFW    "Find and link GLFW3 (install via your package manager or vcpkg)" OFF)

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------
find_package(OpenGL REQUIRED)

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
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ai/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/combat/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/debug_tools/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/config/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/ecs/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/events/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/logging/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/persistence/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/physics/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/resources/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/threading/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/assets/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/world/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/factions/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/formation/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/hangar/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/inventory/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/navigation/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/networking/*.cpp"
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
    "${CMAKE_CURRENT_SOURCE_DIR}/src/input/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/physics/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/widgets/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/voxel/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/building/*.cpp"
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

find_package(Threads REQUIRED)

target_link_libraries(subspace_engine PUBLIC
    OpenGL::GL
    Threads::Threads
)

if(SUBSPACE_USE_GLFW)
    target_link_libraries(subspace_engine PUBLIC glfw)
endif()

# ---------------------------------------------------------------------------
# Game executable
# ---------------------------------------------------------------------------
add_executable(subspace_game
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
)

target_link_libraries(subspace_game PRIVATE
    subspace_engine
)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
if(SUBSPACE_BUILD_TESTS)
    enable_testing()

    file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
    )

    if(TEST_SOURCES)
        add_executable(subspace_tests ${TEST_SOURCES})

        target_link_libraries(subspace_tests PRIVATE
            subspace_engine
        )

        add_test(NAME SubspaceTests COMMAND subspace_tests)
    endif()
endif()
