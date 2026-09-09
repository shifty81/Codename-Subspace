file(READ "${ROOT}/src/content/ShipyardModuleSystem.cpp" MODULE_SOURCE)
file(READ "${ROOT}/src/content/ShipyardPartTaxonomySystem.cpp" TAXONOMY_SOURCE)
file(READ "${ROOT}/src/ship_editor/ShipyardBuilderSystem.cpp" BUILDER_SOURCE)
file(READ "${ROOT}/src/application/NativeGameApplication.cpp" APP_SOURCE)
file(READ "${ROOT}/src/application/NativeBattlefieldRenderer.cpp" RENDERER_SOURCE)

foreach(token
    "miscfinhanger"
    "USER_ROOT_REVIEW"
    "Wing mounted vertically / wrong root face"
    "if (record.partRole == ShipyardPartRole::Fin) finFallback.push_back")
    string(FIND "${MODULE_SOURCE}" "${token}" POS)
    if(POS EQUAL -1)
        message(FATAL_ERROR "Pass497 module authority token missing: ${token}")
    endif()
endforeach()

string(FIND "${TAXONOMY_SOURCE}" "if(v==\"miscfinhanger\")return ShipyardPartRole::Wing" TAX_POS)
if(TAX_POS EQUAL -1)
    message(FATAL_ERROR "Pass497 miscFinHanger user-certified Wing override missing")
endif()

foreach(token
    "SyncCatalogSelectionToPlaced"
    "Selected assembled")
    string(FIND "${BUILDER_SOURCE}" "${token}" POS)
    if(POS EQUAL -1)
        message(FATAL_ERROR "Pass497 selection sync token missing: ${token}")
    endif()
endforeach()

foreach(token
    "_shipyardCatalogPressPending"
    "_shipyardCatalogPressDistance>=8.0f"
    "CancelCatalogDrag")
    string(FIND "${APP_SOURCE}" "${token}" POS)
    if(POS EQUAL -1)
        message(FATAL_ERROR "Pass497 click-vs-drag token missing: ${token}")
    endif()
endforeach()

foreach(token
    "Semantic Shipyard paint zones"
    "Mat_Main"
    "Mat_Seco"
    "Mat_MetalL"
    "ZONES: PRIMARY / SECONDARY / STRUCTURAL / METAL / EMISSIVE / GLASS")
    string(FIND "${RENDERER_SOURCE}" "${token}" POS)
    if(POS EQUAL -1)
        message(FATAL_ERROR "Pass497 paint-zone token missing: ${token}")
    endif()
endforeach()

message(STATUS "Pass497 source gate PASS")
