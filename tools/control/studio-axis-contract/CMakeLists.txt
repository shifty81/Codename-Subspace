cmake_minimum_required(VERSION 3.16)
project(SubspaceR12AxisContract LANGUAGES CXX)
enable_testing()
add_executable(subspace_r12_axis_contract
    "${CMAKE_CURRENT_LIST_DIR}/../../../engine/tests/studio_r12_axis_layout_tests.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../../engine/src/editor/ConstructionSymmetrySystem.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../../engine/src/editor/EditorDccShellLayoutSystem.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../../engine/src/editor/EditorForgeGuiStyleSystem.cpp")
target_compile_features(subspace_r12_axis_contract PRIVATE cxx_std_17)
target_include_directories(subspace_r12_axis_contract PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../../../engine/include")
add_test(NAME SubspaceR12AxisContract COMMAND subspace_r12_axis_contract)
