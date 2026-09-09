set(PROJECT_ROOT "${ROOT}/..")
function(require_token path token label)
  if(NOT EXISTS "${path}")
    message(FATAL_ERROR "Pass735-744 missing ${label}: ${path}")
  endif()
  file(READ "${path}" CONTENT)
  string(FIND "${CONTENT}" "${token}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass735-744 ${label} missing token: ${token}")
  endif()
endfunction()

set(BUILDER "${ROOT}/src/ship_editor/ShipyardBuilderSystem.cpp")
set(WORKSPACE "${ROOT}/include/ship_editor/ShipyardWorkspaceSystem.h")
set(CAPS "${ROOT}/include/ship_editor/ShipyardCapabilitySystem.h")
set(DEVWORLD "${ROOT}/include/developer/ShipyardDevWorldSystem.h")
set(INTERIOR "${ROOT}/include/interior/ModularInteriorKitSystem.h")
foreach(token "WorkspaceInterior" "WorkspaceCharacter" "WorkspaceDevWorld" "ScalePlayerUp" "DevWorldNextBackdrop")
  require_token("${BUILDER}" "${token}" "Shipyard cumulative Dev workspace")
endforeach()
foreach(token "Interior" "Character" "DevWorld")
  require_token("${WORKSPACE}" "${token}" "Shipyard workspace authority")
endforeach()
foreach(token "interior" "character" "devWorld")
  require_token("${CAPS}" "${token}" "Shipyard capability profile")
endforeach()
foreach(token "Checkerboard" "AnimationViewer" "KitbashCatalog" "CertifiedShips" "InteriorKit" "PcgProvingGround" "Terraforming")
  require_token("${DEVWORLD}" "${token}" "Shipyard Dev World")
endforeach()
require_token("${INTERIOR}" "std::vector<InteriorSnapSocket> sockets;" "historical interior aggregate compatibility")
require_token("${INTERIOR}" "sourcePackId" "append-only interior provenance")

set(TOOLS "${PROJECT_ROOT}/SubspaceTools.ps1")
set(UPDATE "${PROJECT_ROOT}/scripts/subspace_apply_update_inbox.ps1")
set(ROOTAUDIT "${PROJECT_ROOT}/scripts/subspace_root_cleanliness_audit.ps1")
set(INSTALLER "${PROJECT_ROOT}/scripts/install_subspace_root_utility.ps1")
foreach(path
    "${PROJECT_ROOT}/tools/control/WriteQualityGateRecord.ps1"
    "${PROJECT_ROOT}/tools/control/BuildArtifactIndex.ps1"
    "${PROJECT_ROOT}/tools/control/CompareQualityGates.ps1"
    "${PROJECT_ROOT}/tools/control/GetDependencyStatus.ps1"
    "${PROJECT_ROOT}/tools/control/InvokePatchUndo.ps1"
    "${PROJECT_ROOT}/tools/control/InvokeRootPatchIntake.ps1"
    "${PROJECT_ROOT}/tools/control/AuditRoot.ps1"
    "${PROJECT_ROOT}/content/architecture/project_control_center_v2.json"
    "${PROJECT_ROOT}/content/architecture/root_patch_intake_contract_v1.json"
    "${PROJECT_ROOT}/content/schemas/root_patch_manifest.schema.v1.json")
  if(NOT EXISTS "${path}")
    message(FATAL_ERROR "Pass735-744 PCC v2 artifact missing: ${path}")
  endif()
endforeach()
foreach(token "Fast development gate" "last-green-quality-gate.json" "Invoke-GateRecord" "Invoke-ArtifactIndex" "Compare latest Full/Fast gate records" "Undo latest committed patch")
  require_token("${TOOLS}" "${token}" "Project Control Center v2")
endforeach()
# Pass746R6R3: source-rollup safety is now delegated to the shared ProjectOps
# source authority. Do not require the retired in-host `.subspace` exclusion
# literal; verify the delegated authority and its generated-state policy instead.
set(PROJECTOPS_COMMON "${PROJECT_ROOT}/tools/control/ProjectOpsCommon.psm1")
set(PROJECT_CONTROL "${PROJECT_ROOT}/project.control.json")
require_token("${TOOLS}" "Invoke-ProjectOpsGovernedTreeStage" "delegated governed source rollup")
require_token("${PROJECTOPS_COMMON}" "function Invoke-ProjectOpsGovernedTreeStage" "ProjectOps governed source staging")
require_token("${PROJECTOPS_COMMON}" "function Test-ProjectOpsGeneratedRelativePath" "ProjectOps generated-state policy")
foreach(token ".subspace\\" "updates\\" "artifacts\\" "dist\\" "logs\\")
  require_token("${PROJECTOPS_COMMON}" "${token}" "ProjectOps generated-state exclusion")
endforeach()
require_token("${PROJECT_CONTROL}" "\"sourceAuthority\"" "ProjectOps source authority contract")
require_token("${PROJECT_CONTROL}" "\"independentOfGitIgnore\": true" "filesystem-owned source authority")

foreach(token "PATCH_MANIFEST.json" "Get-PatchManifestInfo" "operation='overlay'" "operation='remove'" "Post-overlay hash mismatch" "Post-remove verification failed")
  require_token("${UPDATE}" "${token}" "transactional patch intake")
endforeach()
# Pass746R6R4: the compatibility wrapper no longer owns root-policy literals.
# Root authority lives in ProjectOpsRootAudit.ps1 + project.control.json.
set(ROOTAUDIT_AUTHORITY "${PROJECT_ROOT}/tools/control/ProjectOpsRootAudit.ps1")
require_token("${ROOTAUDIT}" "ProjectOpsRootAudit.ps1" "delegated root audit wrapper")
require_token("${ROOTAUDIT_AUTHORITY}" "projectOps.rootPolicy" "ProjectOps root policy authority")
require_token("${ROOTAUDIT_AUTHORITY}" "forbiddenGeneratedRoots" "ProjectOps forbidden generated roots")
require_token("${PROJECT_CONTROL}" "\"rootPolicy\"" "ProjectOps root-policy contract")
foreach(token "tools\\control\\InvokePatchUndo.ps1" "root_patch_intake_contract_v1.json" "root_patch_manifest.schema.v1.json" "updates\\transactions" "updates\\undo")
  require_token("${INSTALLER}" "${token}" "root utility installer")
endforeach()
require_token("${PROJECT_ROOT}/tools/control/InvokePatchUndo.ps1" "removed by the patch has since been recreated" "drift-guarded patch undo")

message(STATUS "Pass735-744 Shipyard Dev / Project Control Center v2 source gate PASS")
