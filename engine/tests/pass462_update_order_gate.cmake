if(NOT DEFINED UPDATE_SCRIPT OR NOT EXISTS "${UPDATE_SCRIPT}")
    message(FATAL_ERROR "Pass462: update processor not found: ${UPDATE_SCRIPT}")
endif()
file(READ "${UPDATE_SCRIPT}" UPDATE_SOURCE)
foreach(REQUIRED_TOKEN
    "Get-MaxPassFromName"
    "Get-RepositoryPass"
    "Archive-SupersededPatch"
    "updates\\superseded"
    "date suffixes such as PASS49_20260822"
    "$first -le 9999"
    "$last -le 9999"
    "$patchPass -lt $CurrentPass")
    string(FIND "${UPDATE_SOURCE}" "${REQUIRED_TOKEN}" TOKEN_INDEX)
    if(TOKEN_INDEX EQUAL -1)
        message(FATAL_ERROR "Pass462: update-order guard token missing: ${REQUIRED_TOKEN}")
    endif()
endforeach()
message(STATUS "Pass462 update-order guard source contract present")
