# Pass902-911 deep alignment authority gate.
# This gate prevents retired identity/travel/data-layout assumptions from
# silently becoming active project authority again.
get_filename_component(PROJECT_ROOT "${ROOT}/.." ABSOLUTE)

function(p902_read rel out)
  set(path "${PROJECT_ROOT}/${rel}")
  if(NOT EXISTS "${path}")
    message(FATAL_ERROR "Pass902-911 required authority file missing: ${rel}")
  endif()
  file(READ "${path}" text)
  set(${out} "${text}" PARENT_SCOPE)
endfunction()

function(p902_require text token label)
  string(FIND "${text}" "${token}" pos)
  if(pos EQUAL -1)
    message(FATAL_ERROR "Pass902-911 missing ${label}: ${token}")
  endif()
endfunction()

function(p902_forbid text token label)
  string(FIND "${text}" "${token}" pos)
  if(NOT pos EQUAL -1)
    message(FATAL_ERROR "Pass902-911 stale ${label} returned: ${token}")
  endif()
endfunction()

p902_read("README.md" README_TEXT)
p902_require("${README_TEXT}" "systemic continuity" "Subspace identity")
p902_require("${README_TEXT}" "internal standalone Project Control Center" "standalone PCC authority")
p902_require("${README_TEXT}" "Reference Solar System" "reference-system terminology")
p902_require("${README_TEXT}" "jump gates" "jump-gate authority")
p902_require("${README_TEXT}" "GameData/" "GameData runtime-data authority")
p902_forbid("${README_TEXT}" "seamless or masked-seamless" "ambiguous planetary continuity wording")

p902_read("docs/EMBER_PROJECT_CONTRACT.md" EMBER_TEXT)
p902_require("${EMBER_TEXT}" "Reference Solar System" "Ember reference-system fixture")
p902_require("${EMBER_TEXT}" "internal standalone PCC" "Ember operations boundary")
p902_forbid("${EMBER_TEXT}" "Open Golden Home System" "Golden Home System active workflow")
p902_forbid("${EMBER_TEXT}" "ForgePY remains the universal operational front end" "ForgePY current authority")

p902_read("docs/SUBSPACE_SANDBOX_AUTHORITY_2026-08-31.md" OLD_SANDBOX)
p902_require("${OLD_SANDBOX}" "SUPERSEDED DESIGN AUTHORITY" "old 2D authority supersession banner")

p902_read("docs/design/INTERSTELLAR_RAIL_TRAVEL_SPEC.md" OLD_RAIL)
p902_require("${OLD_RAIL}" "SUPERSEDED TRAVEL MODEL" "rail-travel supersession banner")

p902_read("docs/design/SUBSPACE_ROGUELITE_INCREMENTAL_DIRECTION.md" OLD_ROGUELITE)
p902_require("${OLD_ROGUELITE}" "REFERENCE / OPTIONAL CAMPAIGN MODE" "roguelite direction demotion")

p902_read("engine/src/application/NativeGameApplication.cpp" APP_TEXT)
p902_forbid("${APP_TEXT}" "no landing path exists" "architectural no-landing claim")
p902_require("${APP_TEXT}" "seamless surface landing is current architecture but not yet runtime-wired" "honest surface-landing implementation gap")

p902_read("engine/src/content/ContentManifest.cpp" CONTENT_MANIFEST)
p902_require("${CONTENT_MANIFEST}" "{\"GameData/\", ContentAuthority::ActiveContent" "GameData active content authority")
p902_forbid("${CONTENT_MANIFEST}" "legacy gameplay data awaiting path normalization" "GameData legacy classification")

p902_read("engine/src/editor/ProjectWideEditorNormalizationSystem.cpp" EDITOR_NORM)
p902_require("${EDITOR_NORM}" "GameData + content split authority" "editor content-layout authority")
p902_forbid("${EDITOR_NORM}" "Move GameData/ into content/data" "obsolete GameData physical move")

p902_read("scripts/subspace_normalization_status.ps1" NORM_STATUS)
p902_require("${NORM_STATUS}" "ACTIVE-DATA" "normalization status GameData authority")
p902_forbid("${NORM_STATUS}" "NEEDS-CONTENT-MIGRATION" "obsolete GameData migration state")

p902_read("project.control.json" PROJECT_CONTROL)
p902_require("${PROJECT_CONTROL}" "\"current\": \"internal-pcc\"" "machine-readable operations authority")
p902_require("${PROJECT_CONTROL}" "\"runtimeAuthoredData\": \"GameData\"" "machine-readable GameData authority")

foreach(rel
    "GameData/Tutorials/navigation_basics.json"
    "GameData/Tutorials/ship_building.json"
    "GameData/Quests/advanced_mining.json"
    "GameData/Quests/build_ship.json")
  p902_read("${rel}" PLAYER_DATA)
  foreach(retired "Naonite" "Trinium" "Xanion" "Ogonite" "Avorion" "voxel blocks" "instant travel between sectors")
    p902_forbid("${PLAYER_DATA}" "${retired}" "retired player-facing Avorion vocabulary in ${rel}")
  endforeach()
endforeach()

p902_read("content/architecture/project_completion_truth_v1.json" COMPLETION_TEXT)
p902_require("${COMPLETION_TEXT}" "runtime.composition-root" "runtime composition gap truth")
p902_require("${COMPLETION_TEXT}" "travel.jump-gate" "jump-gate maturity truth")
p902_require("${COMPLETION_TEXT}" "planet.seamless-landing" "seamless landing maturity truth")
p902_require("${COMPLETION_TEXT}" "editor.dock-workspace" "dock maturity truth")

message(STATUS "Pass902-911 deep project alignment authority gate PASS")

foreach(rel
    "docs/audits/DEEP_PROJECT_ALIGNMENT_AUDIT_20260912.md"
    "docs/audits/DEEP_FEATURE_MATURITY_MATRIX_20260912_R1.csv"
    "docs/audits/BUILD_GRAPH_AUTHORITY_MATRIX_20260912_R1.csv"
    "docs/audits/AUTHORITY_NORMALIZATION_MATRIX_20260912_R1.csv")
  if(NOT EXISTS "${PROJECT_ROOT}/${rel}")
    message(FATAL_ERROR "Pass902-911 audit artifact missing: ${rel}")
  endif()
endforeach()
message(STATUS "Pass902-911 audit artifact presence PASS")
