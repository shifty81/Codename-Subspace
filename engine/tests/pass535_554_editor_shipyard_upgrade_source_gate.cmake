if(NOT DEFINED ROOT)
  message(FATAL_ERROR "ROOT not supplied")
endif()
foreach(p
    include/editor/SubspaceEditorCore.h
    include/editor/EditorAssetBrowser.h
    include/editor/EditorGizmoSystem.h
    include/editor/SubspaceEditorAcceptanceSystem.h
    include/ui/SubspaceUiFramework.h
    include/ship_editor/ShipyardWorkspaceSystem.h
    include/ships/ShipyardDesignDnaSystem.h)
  if(NOT EXISTS "${ROOT}/${p}")
    message(FATAL_ERROR "Missing ${p}")
  endif()
endforeach()
file(READ "${ROOT}/src/ship_editor/ShipyardBuilderSystem.cpp" b)
foreach(t WorkspaceBuild WorkspaceAppearance WorkspaceSystems WorkspaceAuthoring CONNECTIONS "TEACH PCG")
  string(FIND "${b}" "${t}" pos)
  if(pos EQUAL -1)
    message(FATAL_ERROR "Shipyard migration missing ${t}")
  endif()
endforeach()
file(READ "${ROOT}/src/application/NativeBattlefieldRenderer.cpp" r)
foreach(t "a neutral DCC canvas replaces the black in-game-space" "STAGED PART - NOT ATTACHED")
  string(FIND "${r}" "${t}" pos)
  if(pos EQUAL -1)
    message(FATAL_ERROR "Renderer integration missing modern DCC authority: ${t}")
  endif()
endforeach()
# Pass1444-1453 successor: orientation stays visible through the compact XYZ
# viewport gizmo while the large center-screen debug arrow is retired.
string(FIND "${r}" "Compact viewport axis gizmo" axis_pos)
string(FIND "${r}" "ShipyardText(\"Y\"" forward_pos)
if(axis_pos EQUAL -1 OR forward_pos EQUAL -1)
  message(FATAL_ERROR "Renderer integration missing compact DCC direction authority")
endif()
message(STATUS "Pass535-554 editor/Shipyard source gate PASS")
