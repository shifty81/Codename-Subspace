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
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ai/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/ecs/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/events/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/logging/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/physics/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/resources/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/factions/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/networking/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/rendering/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ship_editor/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ships/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/weapons/*.cpp"
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

target_link_libraries(subspace_engine PUBLIC
    OpenGL::GL
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
