#pragma once

#include "navigation/GalaxyCatalogSystem.h"
#include <string>
#include <vector>

namespace subspace {

enum class GalaxyOverlay { None, Security, Economy, Resources, Population, Shipyards, Exploration, Anomalies };
struct GalaxyMapCamera { float yaw=25,pitch=28,distance=5400; float focusX=0,focusY=0,focusZ=0; };
struct GalaxyMapSelection { bool valid=false; std::uint32_t systemId=0; std::string title; std::vector<std::string> details; };
class GalaxyMapSystem {
public:
    std::vector<const GalaxySystemRecord*> Filter(const std::vector<GalaxySystemRecord>& catalog,GalaxyOverlay overlay,float threshold=0.0f) const;
    GalaxyMapSelection Select(const std::vector<GalaxySystemRecord>& catalog,std::uint32_t id) const;
    void Orbit(GalaxyMapCamera& camera,float dyaw,float dpitch) const;
    void Zoom(GalaxyMapCamera& camera,float delta) const;
};

} // namespace subspace
