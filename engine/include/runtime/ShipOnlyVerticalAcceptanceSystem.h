#pragma once

#include <string>
#include <vector>

namespace subspace {

struct ShipOnlyAcceptanceState {
    bool stationDocked=false;
    bool fittedShipVisible=false;
    bool strategicFlight=false;
    bool contextOrders=false;
    bool hotbarReady=false;
    bool galaxyMapReady=false;
    bool orbitalSystemReady=false;
    bool planetaryIndustryReady=false;
    bool fleetWingReady=false;
    bool persistenceReady=false;
    bool onFootPathExposed=false;
};

struct ShipOnlyAcceptanceReport { bool pass=false; int score=0; std::vector<std::string> blockers; };

class ShipOnlyVerticalAcceptanceSystem {
public:
    ShipOnlyAcceptanceReport Evaluate(const ShipOnlyAcceptanceState& state) const;
};

} // namespace subspace
