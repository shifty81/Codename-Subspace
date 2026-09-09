get_filename_component(PROJECT_ROOT "${ROOT}/.." ABSOLUTE)
set(PUBLISHER "${PROJECT_ROOT}/tools/control/NormalizeGitHubAuthority.ps1")

if(NOT EXISTS "${PUBLISHER}")
  message(FATAL_ERROR "R6R12 static gate missing publisher: ${PUBLISHER}")
endif()

file(READ "${PUBLISHER}" PUBLISHER_TEXT)

function(r6r12_require TEXT_VALUE NEEDLE)
  string(FIND "${TEXT_VALUE}" "${NEEDLE}" FOUND_AT)
  if(FOUND_AT EQUAL -1)
    message(FATAL_ERROR "R6R12 static gate missing: ${NEEDLE}")
  endif()
endfunction()

r6r12_require("${PUBLISHER_TEXT}" "$leaseArg=[string]::Concat('--force-with-lease=refs/heads/main:',[string]$publishAudit.Sha)")
r6r12_require("${PUBLISHER_TEXT}" "$mainRefspec='refs/heads/main:refs/heads/main'")
r6r12_require("${PUBLISHER_TEXT}" "Run-Git -Working $repo -GitArgs @('push',$leaseArg,'origin',$mainRefspec)")
r6r12_require("${PUBLISHER_TEXT}" "Remote main SHA is not a valid 40-character Git object id")
r6r12_require("${PUBLISHER_TEXT}" "Publishing normalized main with guarded lease")

# The exact inline concatenation that was observed splitting into native argv
# must never return.
string(FIND "${PUBLISHER_TEXT}" "@('push','--force-with-lease=refs/heads/main:'+$publishAudit.Sha,'origin','main:main')" OLD_INLINE)
if(NOT OLD_INLINE EQUAL -1)
  message(FATAL_ERROR "R6R12 rejected inline force-with-lease concatenation")
endif()

# The remote/refspec ordering must remain explicit.
string(FIND "${PUBLISHER_TEXT}" "@('push',$leaseArg,'origin',$mainRefspec)" GOOD_ORDER)
if(GOOD_ORDER EQUAL -1)
  message(FATAL_ERROR "R6R12 rejected repository/refspec ordering drift")
endif()

message(STATUS "Pass746R6R12 repository publish force-with-lease argv repair static gate PASS")
