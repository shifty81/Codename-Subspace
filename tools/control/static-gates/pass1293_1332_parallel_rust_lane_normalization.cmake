get_filename_component(PROJECT_ROOT "${ROOT}/.." ABSOLUTE)
set(RUST_ROOT "${PROJECT_ROOT}/rust")

foreach(FILE
    "Cargo.toml"
    "CodenameSubspace.emberproject"
    "project.control.json"
    "crates/subspace_ship/src/lib.rs"
    "crates/subspace_shipyard/src/lib.rs"
    "crates/subspace_ship_pcg/src/lib.rs"
    "crates/subspace_ship_interior/src/lib.rs"
    "crates/subspace_ship_systems/src/lib.rs"
    "crates/subspace_activity/src/lib.rs"
    "editor/subspace_ember_bridge/src/lib.rs")
    if(NOT EXISTS "${RUST_ROOT}/${FILE}")
        message(FATAL_ERROR "PASS1293-1332 missing Rust lane authority: ${FILE}")
    endif()
endforeach()

file(READ "${RUST_ROOT}/crates/subspace_ship/src/lib.rs" SHIP)
file(READ "${RUST_ROOT}/crates/subspace_shipyard/src/lib.rs" SHIPYARD)
file(READ "${RUST_ROOT}/crates/subspace_ship_pcg/src/lib.rs" PCG)
file(READ "${RUST_ROOT}/editor/subspace_ember_bridge/src/lib.rs" EMBER)

foreach(TOKEN "ShipBlueprint" "ModuleDefinition" "SocketDefinition" "MountProfile" "ShipAppearance")
    string(FIND "${SHIP}" "${TOKEN}" POS)
    if(POS EQUAL -1)
        message(FATAL_ERROR "PASS1293-1332 ship contract missing ${TOKEN}")
    endif()
endforeach()

foreach(TOKEN "ShipyardDocument" "ShipyardSession" "CommandRegistry" "History" "AssetBrowserQuery" "PropertyScope")
    string(FIND "${SHIPYARD}" "${TOKEN}" POS)
    if(POS EQUAL -1)
        message(FATAL_ERROR "PASS1293-1332 Shipyard authority missing ${TOKEN}")
    endif()
endforeach()

foreach(TOKEN "new_seed_and_generate" "generator.generate")
    string(FIND "${PCG}${SHIPYARD}" "${TOKEN}" POS)
    if(POS EQUAL -1)
        message(FATAL_ERROR "PASS1293-1332 generator authority missing ${TOKEN}")
    endif()
endforeach()

foreach(TOKEN "asset-browser" "ship-viewport" "outliner" "inspector" "activity" "cortex" "Tool::Attach" "Tool::Measure")
    string(FIND "${EMBER}" "${TOKEN}" POS)
    if(POS EQUAL -1)
        message(FATAL_ERROR "PASS1293-1332 Ember contribution missing ${TOKEN}")
    endif()
endforeach()

message(STATUS "PASS1293-1332 parallel Rust lane normalization certified")
