#include "ships/ShipClassRoleSystem.h"

#include <algorithm>

namespace subspace {

ShipClassEnvelope ShipClassRoleSystem::Envelope(ShipClass c){
    switch(c){
    case ShipClass::Frigate:return {c,UniversalSizeClass::XS,40.0f,90.0f,65.0f};
    case ShipClass::Destroyer:return {c,UniversalSizeClass::S,90.0f,160.0f,125.0f};
    case ShipClass::Cruiser:return {c,UniversalSizeClass::M,160.0f,280.0f,220.0f};
    case ShipClass::Battlecruiser:return {c,UniversalSizeClass::L,280.0f,450.0f,365.0f};
    case ShipClass::Battleship:return {c,UniversalSizeClass::XL,450.0f,750.0f,600.0f};
    case ShipClass::Capital:return {c,UniversalSizeClass::XL,750.0f,1800.0f,1050.0f};
    }
    return {};
}
const char* ShipClassRoleSystem::ClassName(ShipClass c){switch(c){case ShipClass::Frigate:return "FRIGATE";case ShipClass::Destroyer:return "DESTROYER";case ShipClass::Cruiser:return "CRUISER";case ShipClass::Battlecruiser:return "BATTLECRUISER";case ShipClass::Battleship:return "BATTLESHIP";case ShipClass::Capital:return "CAPITAL";}return "FRIGATE";}
const char* ShipClassRoleSystem::RoleName(ShipRole r){switch(r){case ShipRole::GeneralCombat:return "COMBAT";case ShipRole::Escort:return "ESCORT";case ShipRole::Scout:return "SCOUT";case ShipRole::ElectronicWarfare:return "ELECTRONIC WARFARE";case ShipRole::Exploration:return "EXPLORATION";case ShipRole::Logistics:return "LOGISTICS";case ShipRole::Mining:return "MINING";case ShipRole::Salvage:return "SALVAGE";case ShipRole::Carrier:return "CARRIER";case ShipRole::Command:return "COMMAND";case ShipRole::Siege:return "SIEGE";case ShipRole::Interdiction:return "INTERDICTION";case ShipRole::Hauling:return "HAULER";case ShipRole::Boarding:return "BOARDING";}return "COMBAT";}
ShipRoleBudget ShipClassRoleSystem::RoleBudget(ShipRole r){
    ShipRoleBudget b;b.role=r;
    switch(r){
    case ShipRole::GeneralCombat:b.weapons=1.35f;b.armor=1.25f;b.sensors=1.05f;break;
    case ShipRole::Escort:b.weapons=1.15f;b.armor=1.05f;b.propulsion=1.30f;b.sensors=1.15f;break;
    case ShipRole::Scout:b.weapons=.65f;b.armor=.65f;b.sensors=1.65f;b.propulsion=1.55f;b.utility=1.25f;break;
    case ShipRole::ElectronicWarfare:b.weapons=.75f;b.armor=.80f;b.sensors=1.85f;b.utility=1.60f;b.propulsion=1.15f;break;
    case ShipRole::Exploration:b.weapons=.70f;b.armor=.75f;b.sensors=1.70f;b.utility=1.55f;b.cargo=1.30f;b.propulsion=1.25f;break;
    case ShipRole::Logistics:b.weapons=.55f;b.armor=.90f;b.utility=1.55f;b.cargo=1.45f;b.logistics=1.85f;break;
    case ShipRole::Mining:b.weapons=.55f;b.armor=.90f;b.utility=1.35f;b.cargo=1.75f;b.industry=1.90f;break;
    case ShipRole::Salvage:b.weapons=.55f;b.armor=.85f;b.utility=1.55f;b.cargo=1.55f;b.industry=1.55f;break;
    case ShipRole::Carrier:b.weapons=.85f;b.armor=1.10f;b.hangar=2.0f;b.utility=1.25f;b.cargo=1.25f;break;
    case ShipRole::Command:b.weapons=1.0f;b.armor=1.20f;b.sensors=1.45f;b.utility=1.50f;break;
    case ShipRole::Siege:b.weapons=1.85f;b.armor=1.40f;b.propulsion=.75f;b.sensors=1.10f;break;
    case ShipRole::Interdiction:b.weapons=1.05f;b.armor=.90f;b.sensors=1.40f;b.propulsion=1.45f;b.utility=1.30f;break;
    case ShipRole::Hauling:b.weapons=.40f;b.armor=.80f;b.cargo=2.10f;b.logistics=1.25f;break;
    case ShipRole::Boarding:b.weapons=.90f;b.armor=1.20f;b.utility=1.50f;b.cargo=1.15f;b.propulsion=1.15f;break;
    }
    return b;
}
std::vector<FactionHullFamilyDefinition> ShipClassRoleSystem::BuildDefaultHullFamilies(const std::string& faction,ShipClass c){
    const std::string prefix=faction+"_"+ClassName(c)+"_HULL_";
    std::vector<FactionHullFamilyDefinition> o;
    o.push_back({faction,c,0,prefix+"A","FAST_NARROW",{ShipRole::GeneralCombat,ShipRole::Escort,ShipRole::Scout,ShipRole::Exploration,ShipRole::Interdiction},{ShipRole::Scout,ShipRole::Escort},1.30f,.78f,1.0f,.82f});
    o.push_back({faction,c,1,prefix+"B","BALANCED_GENERAL",{ShipRole::GeneralCombat,ShipRole::Escort,ShipRole::ElectronicWarfare,ShipRole::Exploration,ShipRole::Logistics,ShipRole::Command,ShipRole::Boarding},{ShipRole::GeneralCombat,ShipRole::Command},1.0f,1.0f,1.15f,1.05f});
    o.push_back({faction,c,2,prefix+"C","HEAVY_ARMORED",{ShipRole::GeneralCombat,ShipRole::Escort,ShipRole::Command,ShipRole::Siege,ShipRole::Boarding},{ShipRole::GeneralCombat,ShipRole::Siege},.82f,1.40f,.85f,1.10f});
    o.push_back({faction,c,3,prefix+"D","MODULAR_UTILITY",{ShipRole::Exploration,ShipRole::Logistics,ShipRole::Mining,ShipRole::Salvage,ShipRole::Carrier,ShipRole::Hauling,ShipRole::Boarding},{ShipRole::Logistics,ShipRole::Carrier},.90f,.90f,1.45f,1.45f});
    // Class-specific role pruning keeps doctrine readable while preserving four
    // physical chassis. Small hulls do not become siege/carrier absurdities;
    // large hulls do not pretend to be tiny scouts.
    for(auto& f:o){
        auto remove=[&](ShipRole role){f.allowedRoles.erase(std::remove(f.allowedRoles.begin(),f.allowedRoles.end(),role),f.allowedRoles.end());f.preferredRoles.erase(std::remove(f.preferredRoles.begin(),f.preferredRoles.end(),role),f.preferredRoles.end());};
        if(c==ShipClass::Frigate){remove(ShipRole::Siege);remove(ShipRole::Carrier);}
        if(c==ShipClass::Battleship||c==ShipClass::Capital){remove(ShipRole::Scout);}
    }
    return o;
}
bool ShipClassRoleSystem::SupportsRole(const FactionHullFamilyDefinition& f,ShipRole r){return std::find(f.allowedRoles.begin(),f.allowedRoles.end(),r)!=f.allowedRoles.end();}

} // namespace subspace
