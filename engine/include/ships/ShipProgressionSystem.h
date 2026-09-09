#pragma once

#include "ships/ShipClassSystem.h"

#include <string>
#include <vector>

namespace subspace {

struct ShipLicenseProfile {
    std::string licenseId;
    ShipClass shipClass = ShipClass::Frigate;
    int trainingRank = 1;
    int navigationRank = 1;
    int commandRank = 0;
    int industryRank = 0;
};

struct CharacterShipTraining {
    int trainingRank = 1;
    int navigationRank = 1;
    int commandRank = 0;
    int industryRank = 0;
    std::vector<std::string> licenses;
};

struct ShipProgressionGate {
    bool allowed = false;
    std::string requiredLicense;
    std::vector<std::string> blockers;
};

class ShipProgressionSystem {
public:
    static ShipLicenseProfile LicenseFor(ShipClass shipClass);
    static ShipProgressionGate CanPilot(const CharacterShipTraining& character,ShipClass shipClass);
    static std::vector<ShipClass> CoreSizeProgression();
};

} // namespace subspace
