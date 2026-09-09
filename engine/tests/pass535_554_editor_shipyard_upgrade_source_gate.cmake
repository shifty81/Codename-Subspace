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
foreach(t "SubspaceUiTheme::Dark" "DRAG INTO VIEWPORT TO PREVIEW")
  string(FIND "${r}" "${t}" pos)
  if(pos EQUAL -1)
    message(FATAL_ERROR "Renderer integration missing ${t}")
  endif()
endforeach()
# Pass655 semantic successor: the previous planar gizmo helper could only
# represent part of the camera orbit. The permanent +Y indicator now projects
# through the actual camera basis, retaining the Pass535 editor-gizmo contract
# while fixing full-360-degree readability.
string(FIND "${r}" "StrategicViewProjection::Build" basis_pos)
string(FIND "${r}" "SHIP FORWARD +Y" forward_pos)
if(basis_pos EQUAL -1 OR forward_pos EQUAL -1)
  message(FATAL_ERROR "Renderer integration missing 360-degree editor direction projection authority")
endif()
message(STATUS "Pass535-554 editor/Shipyard source gate PASS")
