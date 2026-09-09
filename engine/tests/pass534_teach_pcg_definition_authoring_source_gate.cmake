set(required_files
  "${ROOT}/include/ship_editor/ShipyardDefinitionOverrideSystem.h"
  "${ROOT}/src/ship_editor/ShipyardDefinitionOverrideSystem.cpp"
  "${ROOT}/include/assets/CanonicalAsset.h"
  "${ROOT}/src/content/ShipyardCanonicalAssetBridge.cpp"
  "${ROOT}/include/ship_editor/ShipyardBuilderSystem.h"
  "${ROOT}/src/ship_editor/ShipyardBuilderSystem.cpp")
foreach(f IN LISTS required_files)
  if(NOT EXISTS "${f}")
    message(FATAL_ERROR "Pass534 missing ${f}")
  endif()
endforeach()
file(READ "${ROOT}/src/ship_editor/ShipyardBuilderSystem.cpp" builder)
foreach(token "Teach PCG definition authoring" "SaveDefinitionOverrides" "CycleSelectedSemantic" "ToggleSelectedGeneratorEligibility")
  string(FIND "${builder}" "${token}" hit)
  if(hit EQUAL -1)
    message(FATAL_ERROR "Pass534 missing builder token ${token}")
  endif()
endforeach()
file(READ "${ROOT}/src/content/ShipyardCanonicalAssetBridge.cpp" bridge)
string(FIND "${bridge}" "SocketAuthority::ManualOverride" manual_hit)
if(manual_hit EQUAL -1)
  message(FATAL_ERROR "Pass534 canonical bridge does not preserve manual socket authority")
endif()
message(STATUS "Pass534 Teach PCG definition authoring source gate passed")
