file(READ "${ROOT}/src/ship_editor/ShipyardProfessionalVisibleCutover.cpp" CUTOVER)

# R3: source-marker checks are literal. CMake MATCHES invokes a regular-expression
# engine, and C++ tokens such as "compatibilityIndex++" are not valid regex patterns.
foreach(TOKEN
    "FriendlyModuleLabel"
    "ModuleSerial"
    "kCompatibilityStride=100000.0f"
    "compatibilityIndex++")
    string(FIND "${CUTOVER}" "${TOKEN}" TOKEN_POS)
    if(TOKEN_POS EQUAL -1)
        message(FATAL_ERROR "PASS1268-1292R2 visible-cutover repair missing literal token: ${TOKEN}")
    endif()
endforeach()

string(FIND "${CUTOVER}" "compatibilityX-=std::max" LEGACY_PROJECTION_POS)
if(NOT LEGACY_PROJECTION_POS EQUAL -1)
    message(FATAL_ERROR "PASS1268-1292R2 reintroduced width-relative compatibility projection overlap risk")
endif()

string(FIND "${CUTOVER}" "FriendlyModuleLabel(rec)" READABLE_LABEL_POS)
if(READABLE_LABEL_POS EQUAL -1)
    message(FATAL_ERROR "PASS1268-1292R2 readable module label is not used by the visible Shipyard")
endif()

message(STATUS "PASS1268-1292R2 legacy control projection / readable label repair certified")
