# Pass932-941 historical intent: GREEN promotion fingerprints normalized working
# tree content independent of Git staging state; Shipyard wheel dolly follows
# the current camera placement ray instead of rebuilding from stale orbit angles.
#
# PASS969-972 migration note:
# The manifest authority is now V3. This gate certifies the original behavioral
# invariant without freezing ProjectOps to the retired V2 schema marker.
set(COMMON "${PROJECT_ROOT}/tools/control/ProjectOpsCommon.psm1")
set(CAMERA_CPP "${PROJECT_ROOT}/engine/src/editor/ConstructionEditorCameraSystem.cpp")
foreach(P IN ITEMS "${COMMON}" "${CAMERA_CPP}")
  if(NOT EXISTS "${P}")
    message(FATAL_ERROR "Pass932-941 required file missing: ${P}")
  endif()
endforeach()
file(READ "${COMMON}" COMMON_TEXT)
file(READ "${CAMERA_CPP}" CAMERA_CPP_TEXT)
foreach(REQUIRED IN ITEMS
  "git HEAD manifest probe failed"
  "'ls-tree','-r','--name-only','HEAD'"
  "'ls-files','--cached','--others','--exclude-standard'"
  "Get-FileHash -Algorithm SHA256"
  "---CERTIFIABLE-WORKTREE-MANIFEST-V3---"
  "fingerprintScope = 'worktree-path-state-bytes-sha256'"
  "System.StringComparer]::Ordinal"
  "FILE`t{0}`t{1}`t{2}"
  "DELETE`t{0}")
  string(FIND "${COMMON_TEXT}" "${REQUIRED}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass932-941 normalized fingerprint authority missing: ${REQUIRED}")
  endif()
endforeach()

# The historical invariant forbids separate staged/unstaged fingerprint buckets.
# V3 legitimately uses `git diff --cached` elsewhere for source-only staging, so
# a whole-module ban on that command would incorrectly reject the modern policy.
foreach(RETIRED IN ITEMS
  "---UNSTAGED---"
  "---STAGED---"
  "---CERTIFIABLE-WORKTREE-MANIFEST-V2---")
  string(FIND "${COMMON_TEXT}" "${RETIRED}" RETIRED_POS)
  if(NOT RETIRED_POS EQUAL -1)
    message(FATAL_ERROR "Pass932-941 retired staging/schema authority still present: ${RETIRED}")
  endif()
endforeach()

foreach(REQUIRED IN ITEMS
  "const Vector3 fromTarget=s.eye-s.assemblyCenter"
  "s.eye=s.assemblyCenter+direction*s.orbitDistance"
  "Synchronize orbit angles to the preserved ray")
  string(FIND "${CAMERA_CPP_TEXT}" "${REQUIRED}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass932-941 camera placement-preserving dolly authority missing: ${REQUIRED}")
  endif()
endforeach()

message(STATUS "Pass932-941 historical promotion/camera intent certified against current V3 manifest authority")
