#pragma once
#include <string>
#include <vector>
namespace subspace {
struct CorporationProgressionState { std::string corporationId; double assetsValue=0; int ownedShips=0; int ownedStations=0; int developedPlanets=0; int activeFleets=0; double reputation=0; int level=1; std::vector<std::string> unlocks; };
class CorporationProgressionSystem {
public:
 int Recalculate(CorporationProgressionState& state) const;
 double Score(const CorporationProgressionState& state) const;
 std::vector<std::string> UnlocksForLevel(int level) const;
};
}
