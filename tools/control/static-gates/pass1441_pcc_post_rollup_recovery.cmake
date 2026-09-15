cmake_minimum_required(VERSION 3.20)
# Pass1441: restore PCC safety/intake behavior after PASS1439 cumulative-source overlay.
get_filename_component(P1441_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
set(pcc "${P1441_ROOT}/tools/control/StandaloneProjectControlCenter.ps1")
set(root_tools "${P1441_ROOT}/SubspaceTools.ps1")

foreach(path IN ITEMS "${pcc}" "${root_tools}")
  if(NOT EXISTS "${path}")
    message(FATAL_ERROR "PASS1441 missing required PCC file: ${path}")
  endif()
endforeach()

file(READ "${pcc}" pcc_text)
foreach(token IN ITEMS
  "function Get-HandoffName"
  "function Get-HandoffFullName"
  "function Test-LegacyZipPatchManifest"
  "PATCH_MANIFEST.json"
  "CumulativeSource"
  "SourceSnapshot"
  "function StartupPatchScan"
  "Archive-SupersededLegacyRootDrops")
  string(FIND "${pcc_text}" "${token}" pos)
  if(pos EQUAL -1)
    message(FATAL_ERROR "PASS1441 standalone PCC recovery missing token: ${token}")
  endif()
endforeach()
foreach(unsafe IN ITEMS "$pending.Name" "$legacy.Name")
  string(FIND "${pcc_text}" "${unsafe}" pos)
  if(NOT pos EQUAL -1)
    message(FATAL_ERROR "PASS1441 restored PCC still contains unsafe StrictMode member enumeration: ${unsafe}")
  endif()
endforeach()

file(READ "${root_tools}" tools_text)
foreach(token IN ITEMS
  "function Test-ZipContainsRootPatchManifest"
  "function Test-IsPatchHandoffFile"
  "PATCH_MANIFEST.json"
  "CumulativeSource"
  "SourceSnapshot")
  string(FIND "${tools_text}" "${token}" pos)
  if(pos EQUAL -1)
    message(FATAL_ERROR "PASS1441 root PCC recovery missing token: ${token}")
  endif()
endforeach()

message(STATUS "PASS1441 PCC post-rollup recovery authority PASS")
