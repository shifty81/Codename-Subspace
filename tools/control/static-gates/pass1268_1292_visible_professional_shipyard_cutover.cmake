cmake_policy(SET CMP0054 NEW)
cmake_policy(SET CMP0064 NEW)
file(READ "${ROOT}/include/ship_editor/ShipyardBuilderSystem.h" BUILDER_H)
file(READ "${ROOT}/include/ship_editor/ShipyardDefinitionOverrideSystem.h" SHIM_H)
file(READ "${ROOT}/src/ship_editor/ShipyardProfessionalVisibleCutover.cpp" CUTOVER)
file(READ "${ROOT}/src/ship_editor/ShipyardWorkspaceSystem.cpp" WORKSPACE)
file(READ "${ROOT}/src/application/NativeBattlefieldRenderer.cpp" RENDERER)

foreach(TOKEN
    "LegacyActivate"
    "LegacyLayout"
    "LegacyBuildControls"
    "LegacyHitTest"
    "developerWorkspacesVisible"
    "testWorkspaceActive")
    string(FIND "${BUILDER_H}" "${TOKEN}" POS)
    if(POS EQUAL -1)
        message(FATAL_ERROR "PASS1268-1292 builder bridge missing ${TOKEN}")
    endif()
endforeach()

foreach(TOKEN
    "#define Activate LegacyActivate"
    "#define Layout LegacyLayout"
    "#define BuildControls LegacyBuildControls"
    "#define HitTest LegacyHitTest")
    string(FIND "${SHIM_H}" "${TOKEN}" POS)
    if(POS EQUAL -1)
        message(FATAL_ERROR "PASS1268-1292 confined legacy shim missing ${TOKEN}")
    endif()
endforeach()

# Historical visible-cutover intent remains certified, but later passes changed
# the presentation contract. Pass1335 promoted right-panel chrome ownership to
# the renderer; Pass1340-1508 converged the Blender/DCC shell; R32/Pass1509 then
# removed the duplicate developer second row and the peer top-level MODEL tab.
# Certify the live single-strip shell plus the successor workspace naming
# authority instead of requiring retired literal labels to survive forever.
# R32's visible surface is CONSTRUCT/GEOMETRY with contextual workspaces;
# historical PAINT is now the Appearance domain, not a visible peer-tab token.
# Validate domain existence in the canonical workspace authority below instead
# of reintroducing retired navigation text into the new one-strip projection.
foreach(TOKEN
    "one authoritative workspace strip"
    "ConstructionUiLayoutPolicy::TabWidth"
    "ShipyardDccUiSystem::AssetPresetName"
    "DccToggleFavoriteSelected"
    "ToolSelect"
    "ToolMove"
    "ToolRotate"
    "ToolScale"
    "DccToggleMaximizeViewport"
    "DccToggleCommandPalette")
    string(FIND "${CUTOVER}" "${TOKEN}" POS)
    if(POS EQUAL -1)
        message(FATAL_ERROR "PASS1268-1292 visible shell missing current authority token ${TOKEN}")
    endif()
endforeach()

foreach(TOKEN
    "ShipyardWorkspaceMode::Interior"
    "ShipyardWorkspaceMode::Systems"
    "ShipyardWorkspaceMode::Appearance"
    "ShipyardWorkspaceMode::Test"
    "ShipyardWorkspaceMode::DevWorld")
    string(FIND "${WORKSPACE}" "${TOKEN}" DOMAIN_POS)
    if(DOMAIN_POS EQUAL -1)
        message(FATAL_ERROR "PASS1268-1292 historical workspace domain missing from current authority: ${TOKEN}")
    endif()
endforeach()

foreach(TOKEN
    "case ShipyardWorkspaceMode::Build:return\"CONSTRUCT\";"
    "case ShipyardWorkspaceMode::Model:return\"GEOMETRY\";")
    string(FIND "${WORKSPACE}" "${TOKEN}" POS)
    if(POS EQUAL -1)
        message(FATAL_ERROR "PASS1268-1292 successor workspace naming authority missing ${TOKEN}")
    endif()
endforeach()

# Pass1338 replaced the composite pre-Blender right-panel title with distinct
# Blender-derived OUTLINER and PROPERTIES editor regions. Preserve the
# historical single-owner intent while certifying the newer visible authority.
foreach(TOKEN
    "\"OUTLINER\""
    "\"PROPERTIES\""
    "build_identity::kShipyardUiProfile")
    string(FIND "${RENDERER}" "${TOKEN}" POS)
    if(POS EQUAL -1)
        message(FATAL_ERROR "PASS1268-1292/1335/1338 renderer-owned right panel missing ${TOKEN}")
    endif()
endforeach()

# Pass1335 intentionally removed these decorative controls from BuildControls.
# Reappearance would restore the exact overlap the deconflict pass eliminated.
foreach(FORBIDDEN
    "OUTLINER  /  SHIP HIERARCHY"
    "PROPERTIES  /  INSTANCE + DEFINITION")
    string(FIND "${CUTOVER}" "${FORBIDDEN}" POS)
    if(NOT POS EQUAL -1)
        message(FATAL_ERROR "PASS1268-1292/1335 duplicate right-panel projection returned: ${FORBIDDEN}")
    endif()
endforeach()

string(FIND "${CUTOVER}" "ActivateInternal(ShipyardBuilderCommand::PcgReroll,0)" REROLL_POS)
string(FIND "${CUTOVER}" "ActivateInternal(ShipyardBuilderCommand::GenerateVariant,0)" GENERATE_POS)
if(REROLL_POS EQUAL -1 OR GENERATE_POS EQUAL -1)
    message(FATAL_ERROR "PASS1268-1292 New Seed + Generate must compose reroll + canonical generator")
endif()

string(FIND "${CUTOVER}" "WorkspaceTest" NEW_ENUM_TEST)
string(FIND "${CUTOVER}" "NewSeedAndGenerate" NEW_ENUM_GEN)
if(NOT NEW_ENUM_TEST EQUAL -1 OR NOT NEW_ENUM_GEN EQUAL -1)
    message(FATAL_ERROR "PASS1268-1292 must not append compatibility enum ordinals for visible-only shell state")
endif()

message(STATUS "PASS1268-1292 visible professional Shipyard cutover certified with Pass1335 single-chrome ownership")
