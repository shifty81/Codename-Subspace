#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

enum class HazardType { Radiation, Thermal, Cryogenic, IonStorm, Corrosive, Debris, Gravitic, DeepSpace };
struct HazardProtectionModule { std::string id; HazardType hazard=HazardType::Radiation; int rating=0; double coverage=1.0; };
struct HazardRegionRequirement { HazardType hazard=HazardType::Radiation; int minimumRating=0; };
struct HazardCertificationReport { bool suitable=true; std::unordered_map<int,int> ratings; std::vector<std::string> deficiencies; };
class HazardCertificationSystem {
public:
    HazardCertificationReport Certify(const std::vector<HazardProtectionModule>& shipModules,const std::vector<HazardProtectionModule>& fleetSupport,const std::vector<HazardRegionRequirement>& requirements) const;
};

} // namespace subspace
