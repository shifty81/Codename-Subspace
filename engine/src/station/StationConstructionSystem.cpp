#include "station/StationConstructionSystem.h"

#include <algorithm>

namespace subspace {
StationPlacementResult StationConstructionSystem::ValidatePlacement(const StationPlacementRequest&r)const{if(r.restrictedTerritory&&!r.hasPermission)return{false,"territory permission required"};if(r.nearestBodyRadiusMeters>0&&r.altitudeMeters<r.nearestBodyRadiusMeters*.02)return{false,"inside unsafe gravity/structure envelope"};return{true,{}};}
bool StationConstructionSystem::HasOperationalCore(const PlayerStationDesign&s)const{return std::any_of(s.modules.begin(),s.modules.end(),[](const auto&m){return m.type==StationModuleType::Core;})&&std::any_of(s.modules.begin(),s.modules.end(),[](const auto&m){return m.type==StationModuleType::Power&&m.powerNet>0;});}
std::vector<std::string> StationConstructionSystem::EmergentRoles(const PlayerStationDesign&s)const{std::vector<std::string>r;auto has=[&](StationModuleType t){return std::any_of(s.modules.begin(),s.modules.end(),[&](const auto&m){return m.type==t;});};if(has(StationModuleType::Refinery))r.push_back("Refinery");if(has(StationModuleType::Manufacturing))r.push_back("Factory");if(has(StationModuleType::Shipyard))r.push_back("Shipyard");if(has(StationModuleType::Market))r.push_back("Trade Hub");if(has(StationModuleType::Defense))r.push_back("Fleet Base");if(has(StationModuleType::ElevatorInterface))r.push_back("Planetary Orbital Terminal");if(r.empty()&&has(StationModuleType::Dock))r.push_back("Outpost");return r;}
double StationConstructionSystem::StructuralMaterialRequired(const PlayerStationDesign&s)const{double total=0;for(const auto&m:s.modules)total+=m.structuralMaterial;return total;}
} // namespace subspace
