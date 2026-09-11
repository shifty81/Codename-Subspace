# Pass790R2 - Shipyard governed-content/runtime recovery source gate.
if(NOT DEFINED ROOT)
  message(FATAL_ERROR "ROOT is required")
endif()

file(READ "${ROOT}/src/application/NativeBattlefieldRenderer.cpp" RENDERER)
foreach(TOKEN
    "SHIPYARD_CERTIFIED_CONTENT_PRIMARY"
    "SHIPYARD_RUNTIME_READY"
    "SHIPYARD_RUNTIME_NOT_READY"
    "SHIPYARD_STANDALONE_SHIELD_PREVIEW"
    "DrawShipProfileShield(*_assets,previewPhysics")
  string(FIND "${RENDERER}" "${TOKEN}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass790 Shipyard renderer closure missing: ${TOKEN}")
  endif()
endforeach()
string(FIND "${RENDERER}" "Pass336 found project root but no modular OBJ directory; fallback ship geometry remains available." LEGACY_EARLY_RETURN)
if(NOT LEGACY_EARLY_RETURN EQUAL -1)
  message(FATAL_ERROR "Pass790 legacy Pass336 Shipyard early-return path is still present")
endif()

file(READ "${ROOT}/src/ship_editor/ShipyardBuilderSystem.cpp" BUILDER)
foreach(TOKEN
    "SHIPYARD_SIZE_AWARE_GENERATION"
    "FilterForClass(model_.catalog,generationClass,true)"
    "UniversalSizeClass::XL:generationClass=ShipClass::Battleship")
  string(FIND "${BUILDER}" "${TOKEN}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass790 size-aware generation closure missing: ${TOKEN}")
  endif()
endforeach()

file(READ "${ROOT}/../scripts/subspace_ensure_kitbash_sources.ps1" ENSURE)
foreach(TOKEN
    "function Test-ShipyardPayloadReady"
    "ModulesRoot $modules"
    "MetadataRoot $metadata")
  string(FIND "${ENSURE}" "${TOKEN}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass790 governed payload readiness closure missing: ${TOKEN}")
  endif()
endforeach()

message(STATUS "Pass790 Shipyard authority recovery source gate passed")
