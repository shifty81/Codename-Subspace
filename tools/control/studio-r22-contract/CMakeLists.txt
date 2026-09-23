cmake_minimum_required(VERSION 3.20)
project(SubspaceStudioR22Contract LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
get_filename_component(ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
add_executable(subspace_studio_r22_contract
    "${CMAKE_CURRENT_LIST_DIR}/studio_r22_contract_tests.cpp"
    "${ROOT}/engine/src/editor/ConstructionSymmetrySystem.cpp")
target_include_directories(subspace_studio_r22_contract PRIVATE "${ROOT}/engine/include")
if(MSVC)
    target_compile_options(subspace_studio_r22_contract PRIVATE /W4 /permissive-)
else()
    target_compile_options(subspace_studio_r22_contract PRIVATE -Wall -Wextra -Wpedantic)
endif()
