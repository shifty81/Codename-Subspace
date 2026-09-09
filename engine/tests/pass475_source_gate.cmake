file(READ "${ROOT}/src/application/NativeBattlefieldRenderer.cpp" RENDERER)
file(READ "${ROOT}/src/application/NativeGameApplication.cpp" APP)
file(READ "${ROOT}/src/ship_editor/ShipyardBuilderSystem.cpp" BUILDER)
file(READ "${ROOT}/src/ship_editor/ShipyardDesignExchangeSystem.cpp" EXCHANGE)

foreach(REQUIRED
    "GL_LINEAR_MIPMAP_LINEAR"
    "GL_TEXTURE_MAX_ANISOTROPY_EXT"
    "PlanetRenderTier"
    "Pass475 celestial submit"
    "Source cloud art should dominate"
    "importedAlpha=ci==0?1.0f:.82f")
  string(FIND "${RENDERER}" "${REQUIRED}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass475 planet fidelity/performance authority missing: ${REQUIRED}")
  endif()
endforeach()

foreach(REQUIRED
    "NextLiveryPreset"
    "AddSelectedDecal"
    "DecalPresets"
    "LiveryPresets")
  string(FIND "${BUILDER}" "${REQUIRED}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass475 Shipyard livery authority missing: ${REQUIRED}")
  endif()
endforeach()

foreach(REQUIRED
    "shipBuilderAppearance"
    "playerShipAppearance"
    "doc.appearance=_shipBuilder.Appearance()")
  string(FIND "${APP}" "${REQUIRED}" POS)
  if(POS EQUAL -1)
    message(FATAL_ERROR "Pass475 application appearance handoff missing: ${REQUIRED}")
  endif()
endforeach()

string(FIND "${EXCHANGE}" "\\\"decals\\\"" DECAL_POS)
if(DECAL_POS EQUAL -1)
  message(FATAL_ERROR "Pass475 Blender/design exchange does not preserve decal layers")
endif()
message(STATUS "Pass475 source gate passed")
