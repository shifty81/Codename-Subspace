#pragma once
#include "procedural/GalaxyGenerator.h"
#include <string>
#include <vector>
namespace subspace {
struct SolarSystemEcologyReport {
    bool certified=false;
    int unsafeContacts=0;
    int localTraffic=0;
    int salvageContacts=0;
    int resourceRegions=0;
    int occupiedBands=0;
    float distributionScore=0.0f;
    std::vector<std::string> issues;
};
class SolarSystemEcologySystem {
public:
    SolarSystemEcologyReport Audit(const GalaxySector& sector) const;
};
}
