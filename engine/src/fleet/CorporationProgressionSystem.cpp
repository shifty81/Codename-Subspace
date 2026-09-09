#include "fleet/CorporationProgressionSystem.h"
#include <algorithm>
namespace subspace {
double CorporationProgressionSystem::Score(const CorporationProgressionState& s) const {return s.assetsValue/100000.0+s.ownedShips*2+s.ownedStations*12+s.developedPlanets*25+s.activeFleets*8+s.reputation*0.1;}
std::vector<std::string> CorporationProgressionSystem::UnlocksForLevel(int l) const {std::vector<std::string> u={"basic_operations"};if(l>=2)u.push_back("fleet_office");if(l>=3)u.push_back("station_charter");if(l>=4)u.push_back("planetary_industry");if(l>=5)u.push_back("capital_shipyard");if(l>=6)u.push_back("regional_command");return u;}
int CorporationProgressionSystem::Recalculate(CorporationProgressionState& s) const {double score=Score(s);s.level=std::clamp(1+static_cast<int>(score/25.0),1,6);s.unlocks=UnlocksForLevel(s.level);return s.level;}
}
