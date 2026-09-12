#include "ships/ShipClassRoleSystem.h"

#include <algorithm>

namespace subspace {
namespace {

ShipClassComponentProfile Profile(ShipClass c,
                                  UniversalSizeClass preferred,
                                  UniversalSizeClass structuralMin,
                                  UniversalSizeClass structuralMax,
                                  UniversalSizeClass auxiliaryMin,
                                  UniversalSizeClass auxiliaryMax,
                                  std::array<float,5> structural,
                                  std::array<float,5> auxiliary) {
    ShipClassComponentProfile p;
    p.shipClass=c;
    p.preferredStructuralSize=preferred;
    p.minimumStructuralSize=structuralMin;
    p.maximumStructuralSize=structuralMax;
    p.minimumAuxiliarySize=auxiliaryMin;
    p.maximumAuxiliarySize=auxiliaryMax;
    p.structuralWeights=structural;
    p.auxiliaryWeights=auxiliary;
    return p;
}

} // namespace

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

ShipClassComponentProfile ShipClassRoleSystem::ComponentProfile(ShipClass c){
    using U=UniversalSizeClass;
    switch(c){
    case ShipClass::Fighter:
        return Profile(c,U::XS,U::XS,U::S,U::XS,U::S,
                       {{.82f,.18f,0,0,0}},{{.90f,.10f,0,0,0}});
    case ShipClass::Shuttle:
        return Profile(c,U::XS,U::XS,U::S,U::XS,U::M,
                       {{.75f,.25f,0,0,0}},{{.72f,.23f,.05f,0,0}});
    case ShipClass::Corvette:
        return Profile(c,U::XS,U::XS,U::M,U::XS,U::M,
                       {{.55f,.38f,.07f,0,0}},{{.55f,.35f,.10f,0,0}});
    case ShipClass::Frigate:
        return Profile(c,U::S,U::XS,U::M,U::XS,U::L,
                       {{.25f,.55f,.20f,0,0}},{{.42f,.38f,.17f,.03f,0}});
    case ShipClass::Destroyer:
        return Profile(c,U::S,U::XS,U::L,U::XS,U::L,
                       {{.10f,.45f,.38f,.07f,0}},{{.25f,.40f,.28f,.07f,0}});
    case ShipClass::Cruiser:
        return Profile(c,U::M,U::S,U::L,U::XS,U::XL,
                       {{0,.20f,.56f,.22f,.02f}},{{.12f,.27f,.42f,.16f,.03f}});
    case ShipClass::Battlecruiser:
        return Profile(c,U::L,U::S,U::XL,U::XS,U::XL,
                       {{0,.08f,.32f,.48f,.12f}},{{.06f,.18f,.34f,.34f,.08f}});
    case ShipClass::Battleship:
        return Profile(c,U::L,U::M,U::XL,U::S,U::XL,
                       {{0,0,.18f,.52f,.30f}},{{0,.08f,.25f,.43f,.24f}});
    case ShipClass::Carrier:
        return Profile(c,U::L,U::M,U::XL,U::XS,U::XL,
                       {{0,0,.14f,.51f,.35f}},{{.03f,.08f,.23f,.42f,.24f}});
    case ShipClass::Freighter:
    case ShipClass::Miner:
        return Profile(c,U::M,U::S,U::XL,U::XS,U::XL,
                       {{0,.12f,.48f,.31f,.09f}},{{.08f,.22f,.38f,.24f,.08f}});
    case ShipClass::Explorer:
        return Profile(c,U::S,U::XS,U::L,U::XS,U::L,
                       {{.12f,.46f,.35f,.07f,0}},{{.28f,.40f,.25f,.07f,0}});
    case ShipClass::Dreadnought:
    case ShipClass::IndustrialCapital:
    case ShipClass::Capital:
        return Profile(c,U::XL,U::M,U::XL,U::S,U::XL,
                       {{0,0,.08f,.34f,.58f}},{{0,.04f,.18f,.35f,.43f}});
    }
    return Profile(c,U::S,U::XS,U::M,U::XS,U::M,
                   {{.25f,.55f,.20f,0,0}},{{.42f,.38f,.20f,0,0}});
}

const char* ShipClassRoleSystem::ClassName(ShipClass c){
    switch(c){
    case ShipClass::Fighter:return "FIGHTER";
    case ShipClass::Corvette:return "CORVETTE";
    case ShipClass::Frigate:return "FRIGATE";
    case ShipClass::Destroyer:return "DESTROYER";
    case ShipClass::Cruiser:return "CRUISER";
    case ShipClass::Battleship:return "BATTLESHIP";
    case ShipClass::Carrier:return "CARRIER";
    case ShipClass::Freighter:return "FREIGHTER";
    case ShipClass::Miner:return "MINER";
    case ShipClass::Explorer:return "EXPLORER";
    case ShipClass::Shuttle:return "SHUTTLE";
    case ShipClass::Battlecruiser:return "BATTLECRUISER";
    case ShipClass::Dreadnought:return "DREADNOUGHT";
    case ShipClass::IndustrialCapital:return "INDUSTRIAL CAPITAL";
    case ShipClass::Capital:return "CAPITAL";
    }
    return "FRIGATE";
}

const char* ShipClassRoleSystem::RoleName(ShipRole r){
    switch(r){
    case ShipRole::Combat:case ShipRole::GeneralCombat:return "COMBAT";
    case ShipRole::Trade:return "TRADE";
    case ShipRole::Mining:return "MINING";
    case ShipRole::Exploration:return "EXPLORATION";
    case ShipRole::Support:return "SUPPORT";
    case ShipRole::MultiRole:return "MULTI ROLE";
    case ShipRole::Hauling:return "HAULER";
    case ShipRole::Salvage:return "SALVAGE";
    case ShipRole::Science:return "SCIENCE";
    case ShipRole::Logistics:return "LOGISTICS";
    case ShipRole::ElectronicWarfare:return "ELECTRONIC WARFARE";
    case ShipRole::FleetCommand:case ShipRole::Command:return "COMMAND";
    case ShipRole::Construction:return "CONSTRUCTION";
    case ShipRole::Refining:return "REFINING";
    case ShipRole::Escort:return "ESCORT";
    case ShipRole::Scout:return "SCOUT";
    case ShipRole::Carrier:return "CARRIER";
    case ShipRole::Siege:return "SIEGE";
    case ShipRole::Interdiction:return "INTERDICTION";
    case ShipRole::Boarding:return "BOARDING";
    }
    return "COMBAT";
}

ShipRoleBudget ShipClassRoleSystem::RoleBudget(ShipRole r){
    ShipRoleBudget b;b.role=r;
    switch(r){
    case ShipRole::Combat:case ShipRole::GeneralCombat:b.weapons=1.35f;b.armor=1.25f;b.sensors=1.05f;break;
    case ShipRole::Escort:b.weapons=1.15f;b.armor=1.05f;b.propulsion=1.30f;b.sensors=1.15f;break;
    case ShipRole::Scout:b.weapons=.65f;b.armor=.65f;b.sensors=1.65f;b.propulsion=1.55f;b.utility=1.25f;break;
    case ShipRole::ElectronicWarfare:b.weapons=.75f;b.armor=.80f;b.sensors=1.85f;b.utility=1.60f;b.propulsion=1.15f;break;
    case ShipRole::Exploration:b.weapons=.70f;b.armor=.75f;b.sensors=1.70f;b.utility=1.55f;b.cargo=1.30f;b.propulsion=1.25f;break;
    case ShipRole::Logistics:case ShipRole::Support:b.weapons=.55f;b.armor=.90f;b.utility=1.55f;b.cargo=1.45f;b.logistics=1.85f;break;
    case ShipRole::Mining:b.weapons=.55f;b.armor=.90f;b.utility=1.35f;b.cargo=1.75f;b.industry=1.90f;break;
    case ShipRole::Salvage:b.weapons=.55f;b.armor=.85f;b.utility=1.55f;b.cargo=1.55f;b.industry=1.55f;break;
    case ShipRole::Carrier:b.weapons=.85f;b.armor=1.10f;b.hangar=2.0f;b.utility=1.25f;b.cargo=1.25f;break;
    case ShipRole::FleetCommand:case ShipRole::Command:b.weapons=1.0f;b.armor=1.20f;b.sensors=1.45f;b.utility=1.50f;break;
    case ShipRole::Siege:b.weapons=1.85f;b.armor=1.40f;b.propulsion=.75f;b.sensors=1.10f;break;
    case ShipRole::Interdiction:b.weapons=1.05f;b.armor=.90f;b.sensors=1.40f;b.propulsion=1.45f;b.utility=1.30f;break;
    case ShipRole::Trade:case ShipRole::Hauling:b.weapons=.40f;b.armor=.80f;b.cargo=2.10f;b.logistics=1.25f;break;
    case ShipRole::Boarding:b.weapons=.90f;b.armor=1.20f;b.utility=1.50f;b.cargo=1.15f;b.propulsion=1.15f;break;
    case ShipRole::Science:b.weapons=.45f;b.armor=.70f;b.sensors=1.75f;b.utility=1.70f;b.cargo=1.10f;break;
    case ShipRole::Construction:b.weapons=.35f;b.armor=1.0f;b.utility=1.80f;b.cargo=1.70f;b.industry=1.65f;break;
    case ShipRole::Refining:b.weapons=.35f;b.armor=1.0f;b.utility=1.45f;b.cargo=1.65f;b.industry=2.0f;break;
    case ShipRole::MultiRole:break;
    }
    return b;
}

ShipRoleSpatialProfile ShipClassRoleSystem::RoleSpatialProfile(ShipRole r){
    ShipRoleSpatialProfile p;p.role=r;
    switch(r){
    case ShipRole::Mining:
        p.cargoVolumeBias=1.75f;p.machineryVolumeBias=1.85f;p.maintenanceAccessBias=1.5f;p.exteriorAccessBias=1.35f;
        p.requiredInteriorFunctions={"COMMAND","ENGINEERING","ORE_STORAGE","PROCESSING","INDUSTRIAL_CONTROL","AIRLOCK"};
        p.preferredExteriorRoles={"MINING","CARGO","UTILITY","RCS"};break;
    case ShipRole::Salvage:
        p.cargoVolumeBias=1.55f;p.machineryVolumeBias=1.35f;p.maintenanceAccessBias=1.65f;p.exteriorAccessBias=1.45f;
        p.requiredInteriorFunctions={"COMMAND","ENGINEERING","SALVAGE_CONTROL","CARGO","AIRLOCK"};
        p.preferredExteriorRoles={"SALVAGE","TRACTOR","CARGO","UTILITY"};break;
    case ShipRole::Carrier:
        p.hangarVolumeBias=2.0f;p.commandVolumeBias=1.25f;p.maintenanceAccessBias=1.45f;p.exteriorAccessBias=1.8f;
        p.requiredInteriorFunctions={"BRIDGE","FLIGHT_CONTROL","HANGAR","MAINTENANCE","FUEL_SERVICE","AIRLOCK"};
        p.preferredExteriorRoles={"HANGAR","DEFENSE","SENSOR","PROPULSION"};break;
    case ShipRole::Scout:
        p.cargoVolumeBias=.65f;p.habitationVolumeBias=.75f;p.sensorExposureBias=1.8f;p.armorShellBias=.72f;p.propulsionReserveBias=1.55f;
        p.requiredInteriorFunctions={"COMMAND","NAVIGATION","SENSORS","ENGINEERING","AIRLOCK"};
        p.preferredExteriorRoles={"SENSOR","PROPULSION","COMMUNICATIONS","RCS"};break;
    case ShipRole::Exploration:
    case ShipRole::Science:
        p.cargoVolumeBias=1.2f;p.habitationVolumeBias=1.2f;p.sensorExposureBias=1.65f;p.propulsionReserveBias=1.25f;
        p.requiredInteriorFunctions={"COMMAND","NAVIGATION","SCIENCE","SENSORS","HABITATION","ENGINEERING","AIRLOCK"};
        p.preferredExteriorRoles={"SENSOR","PROBE","UTILITY","COMMUNICATIONS"};break;
    case ShipRole::Command:
    case ShipRole::FleetCommand:
        p.commandVolumeBias=1.75f;p.sensorExposureBias=1.4f;p.armorShellBias=1.2f;
        p.requiredInteriorFunctions={"BRIDGE","CIC","COMMUNICATIONS","TACTICAL","ENGINEERING","AIRLOCK"};
        p.preferredExteriorRoles={"COMMUNICATIONS","SENSOR","DEFENSE","COMMAND"};break;
    case ShipRole::Hauling:
    case ShipRole::Trade:
        p.cargoVolumeBias=2.15f;p.exteriorAccessBias=1.75f;p.maintenanceAccessBias=1.15f;
        p.requiredInteriorFunctions={"COMMAND","ENGINEERING","CARGO","LOGISTICS_CONTROL","AIRLOCK"};
        p.preferredExteriorRoles={"CARGO","DOCKING","PROPULSION","UTILITY"};break;
    case ShipRole::Boarding:
        p.habitationVolumeBias=1.35f;p.exteriorAccessBias=1.8f;p.armorShellBias=1.25f;
        p.requiredInteriorFunctions={"COMMAND","TACTICAL","ARMORY","TROOP_STAGING","BOARDING_AIRLOCK","ENGINEERING"};
        p.preferredExteriorRoles={"BOARDING","BREACH","ARMOR","DEFENSE"};break;
    case ShipRole::Siege:
        p.machineryVolumeBias=1.35f;p.commandVolumeBias=1.2f;p.armorShellBias=1.55f;p.propulsionReserveBias=.75f;
        p.requiredInteriorFunctions={"COMMAND","TACTICAL","MAGAZINE","POWER","ENGINEERING","DAMAGE_CONTROL"};
        p.preferredExteriorRoles={"HEAVY_WEAPON","ARMOR","THERMAL","POWER"};break;
    default:
        p.requiredInteriorFunctions={"COMMAND","ENGINEERING","LIFE_SUPPORT","AIRLOCK"};
        p.preferredExteriorRoles={"WEAPON","ARMOR","SENSOR","PROPULSION"};break;
    }
    return p;
}

float ShipClassRoleSystem::SizeWeight(const ShipClassComponentProfile& p,UniversalSizeClass s,bool auxiliary){
    const int i=static_cast<int>(s);
    if(i<0||i>=5)return 0.0f;
    return auxiliary?p.auxiliaryWeights[static_cast<std::size_t>(i)]:p.structuralWeights[static_cast<std::size_t>(i)];
}

bool ShipClassRoleSystem::SupportsModuleSize(const ShipClassComponentProfile& p,UniversalSizeClass s,bool auxiliary){
    const auto minSize=auxiliary?p.minimumAuxiliarySize:p.minimumStructuralSize;
    const auto maxSize=auxiliary?p.maximumAuxiliarySize:p.maximumStructuralSize;
    return UniversalKitbashAuthority::SizeWithin(s,minSize,maxSize)&&SizeWeight(p,s,auxiliary)>0.0f;
}

std::vector<FactionHullFamilyDefinition> ShipClassRoleSystem::BuildDefaultHullFamilies(const std::string& faction,ShipClass c){
    const std::string prefix=faction+"_"+ClassName(c)+"_HULL_";
    std::vector<FactionHullFamilyDefinition> o;
    o.push_back({faction,c,0,prefix+"A","FAST_NARROW",{ShipRole::GeneralCombat,ShipRole::Escort,ShipRole::Scout,ShipRole::Exploration,ShipRole::Interdiction},{ShipRole::Scout,ShipRole::Escort},1.30f,.78f,1.0f,.82f});
    o.push_back({faction,c,1,prefix+"B","BALANCED_GENERAL",{ShipRole::GeneralCombat,ShipRole::Escort,ShipRole::ElectronicWarfare,ShipRole::Exploration,ShipRole::Logistics,ShipRole::Command,ShipRole::Boarding},{ShipRole::GeneralCombat,ShipRole::Command},1.0f,1.0f,1.15f,1.05f});
    o.push_back({faction,c,2,prefix+"C","HEAVY_ARMORED",{ShipRole::GeneralCombat,ShipRole::Escort,ShipRole::Command,ShipRole::Siege,ShipRole::Boarding},{ShipRole::GeneralCombat,ShipRole::Siege},.82f,1.40f,.85f,1.10f});
    o.push_back({faction,c,3,prefix+"D","MODULAR_UTILITY",{ShipRole::Exploration,ShipRole::Logistics,ShipRole::Mining,ShipRole::Salvage,ShipRole::Carrier,ShipRole::Hauling,ShipRole::Boarding},{ShipRole::Logistics,ShipRole::Carrier},.90f,.90f,1.45f,1.45f});
    for(auto& f:o){
        auto remove=[&](ShipRole role){f.allowedRoles.erase(std::remove(f.allowedRoles.begin(),f.allowedRoles.end(),role),f.allowedRoles.end());f.preferredRoles.erase(std::remove(f.preferredRoles.begin(),f.preferredRoles.end(),role),f.preferredRoles.end());};
        if(c==ShipClass::Fighter||c==ShipClass::Shuttle||c==ShipClass::Corvette||c==ShipClass::Frigate){remove(ShipRole::Siege);remove(ShipRole::Carrier);}
        if(c==ShipClass::Battleship||c==ShipClass::Dreadnought||c==ShipClass::Capital||c==ShipClass::IndustrialCapital){remove(ShipRole::Scout);}
    }
    return o;
}

bool ShipClassRoleSystem::SupportsRole(const FactionHullFamilyDefinition& f,ShipRole r){
    return std::find(f.allowedRoles.begin(),f.allowedRoles.end(),r)!=f.allowedRoles.end();
}

} // namespace subspace
