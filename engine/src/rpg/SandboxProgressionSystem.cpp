#include "rpg/SandboxProgressionSystem.h"
namespace subspace {
std::vector<SandboxMilestone> SandboxProgressionSystem::Evaluate(const SandboxProgressionState&s)const{return{{"first_ship","Independent Operator",s.ownedShips>=1},{"first_captain","First Fleet Member",s.hiredCaptains>=1},{"small_fleet","Small Fleet",s.ownedShips>=4},{"first_station","Permanent Footprint",s.stations>=1},{"planetary_industry","Planetary Manufacturing",s.planetaryColonies>=1},{"multi_system","Multi-System Corporation",s.systemsOperating>=3},{"industrial_power","Industrial Power",s.corporationAssets>=10000000.0}};}
} // namespace subspace
