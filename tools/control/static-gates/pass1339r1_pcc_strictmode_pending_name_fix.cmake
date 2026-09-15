# Pass1339R1: PowerShell 5.1 StrictMode-safe pending handoff naming.
get_filename_component(P1339R1_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
set(pcc "${P1339R1_ROOT}/tools/control/StandaloneProjectControlCenter.ps1")
set(status "${P1339R1_ROOT}/docs/CURRENT_STATUS.md")
if(NOT EXISTS "${pcc}")
  message(FATAL_ERROR "Pass1339R1 missing standalone PCC")
endif()
file(READ "${pcc}" pcc_text)
foreach(token
  "function Get-HandoffName"
  "function Get-HandoffFullName"
  "foreach($item in $pending)"
  "foreach($item in $legacy)"
  "<unresolved update handoff>"
  "Archive-SupersededLegacyRootDrops")
  string(FIND "${pcc_text}" "${token}" pos)
  if(pos EQUAL -1)
    message(FATAL_ERROR "Pass1339R1 missing token '${token}'")
  endif()
endforeach()
string(FIND "${pcc_text}" "$pending.Name" unsafe_pending)
if(NOT unsafe_pending EQUAL -1)
  message(FATAL_ERROR "Pass1339R1 unsafe $pending.Name member enumeration remains")
endif()
string(FIND "${pcc_text}" "$legacy.Name" unsafe_legacy)
if(NOT unsafe_legacy EQUAL -1)
  message(FATAL_ERROR "Pass1339R1 unsafe $legacy.Name member enumeration remains")
endif()
file(READ "${status}" status_text)
string(FIND "${status_text}" "Pass1339R1 / PENDING-NAME-STRICTMODE-FIX" status_pos)
if(status_pos EQUAL -1)
  message(FATAL_ERROR "Pass1339R1 status authority missing")
endif()
message(STATUS "Pass1339R1 PCC StrictMode pending-name fix PASS")
