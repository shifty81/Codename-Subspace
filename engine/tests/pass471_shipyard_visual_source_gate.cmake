file(READ "${RENDERER_SOURCE}" renderer)
file(READ "${MATERIAL_SOURCE}" material)
file(READ "${MODULE_SOURCE}" module)
foreach(token
    "DrawStandaloneShipyardBackdrop"
    "ShipyardGeometryAnalysisSystem::Analyze(mesh)"
    "CacheShipMaterialTextures"
    "SetShipBaseTexture"
    "shipTextures")
  string(FIND "${renderer}" "${token}" found)
  if(found EQUAL -1)
    message(FATAL_ERROR "Pass471 renderer authority token missing: ${token}")
  endif()
endforeach()
foreach(token "uBaseColorTexture" "uUseBaseColorTexture" "vTexCoord")
  string(FIND "${material}" "${token}" found)
  if(found EQUAL -1)
    message(FATAL_ERROR "Pass471 material shader token missing: ${token}")
  endif()
endforeach()
foreach(token "parentPlacement.yawDegrees" "forwardSurface" "engine_port")
  string(FIND "${module}" "${token}" found)
  if(found EQUAL -1)
    message(FATAL_ERROR "Pass471 attachment token missing: ${token}")
  endif()
endforeach()
