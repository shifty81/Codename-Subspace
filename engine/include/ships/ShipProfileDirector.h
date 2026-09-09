#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

enum class ShipRoleProfile { Combat, Hauler, Mining, Salvage, Industrial, Exploration, Carrier, Patrol, Multipurpose };
enum class CockpitFamily { Wedge, ArmoredBridge, RecessedMilitary, IndustrialTower, InterceptorCanopy, SplitCommand, SensorForward, HeavyCommandDeck };

struct ShipProfileGrammar {
    ShipRoleProfile role=ShipRoleProfile::Multipurpose;
    CockpitFamily cockpit=CockpitFamily::Wedge;
    std::string manufacturer="VANGUARD";
    float noseRatio=.18f;
    float hullRatio=.42f;
    float utilityRatio=.18f;
    float propulsionRatio=.22f;
    float widthToLength=.42f;
    float negativeSpace=.18f;
    float asymmetry=.0f;
    bool articulated=false;
    int profileScore=0;
};

class ShipProfileDirector {
public:
    ShipProfileGrammar Build(ShipRoleProfile role,std::uint32_t seed) const;
    bool Validate(const ShipProfileGrammar& grammar,std::vector<std::string>* errors=nullptr) const;
    static const char* CockpitName(CockpitFamily family);
};

} // namespace subspace
