#pragma once
#include "fleet/FleetCommandSystem.h"
#include <string>
#include <unordered_map>
#include <vector>
namespace subspace {
enum class FleetFormation { Line, Wedge, Sphere, Screen, Convoy, MiningSpread };
enum class EngagementRule { HoldFire, Defensive, AssignedTargets, Aggressive };
struct FleetDoctrine { std::string id; FleetFormation formation=FleetFormation::Line; EngagementRule engagement=EngagementRule::Defensive; double desiredCombatFraction=0.4; double desiredSupportFraction=0.2; double desiredIndustrialFraction=0.2; bool maintainFormation=true; bool protectHaulers=true; };
struct DoctrineReport { bool valid=true; std::vector<std::string> warnings; double combatFraction=0; double supportFraction=0; double industrialFraction=0; };
class FleetDoctrineSystem {
public:
 bool Register(const FleetDoctrine& doctrine);
 const FleetDoctrine* Get(const std::string& id) const;
 DoctrineReport Evaluate(const FleetDoctrine& doctrine,const std::vector<FleetMember>& members) const;
private:std::unordered_map<std::string,FleetDoctrine> doctrines_;
};
}
