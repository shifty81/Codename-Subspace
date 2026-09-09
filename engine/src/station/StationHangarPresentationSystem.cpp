#include "station/StationHangarPresentationSystem.h"

namespace subspace {

HangarSceneProfile StationHangarPresentationSystem::Build(StationArchetype a,StationBerthSize berth) const {
    HangarSceneProfile p;
    const float scale=berth==StationBerthSize::Small?.72f:berth==StationBerthSize::Heavy?1.45f:berth==StationBerthSize::Capital?2.35f:1.0f;
    p.width*=scale;p.length*=scale;p.height*=scale;p.serviceArms=berth==StationBerthSize::Capital?8:4;p.cargoLifts=berth==StationBerthSize::Capital?4:2;
    switch(a){
        case StationArchetype::TradeHub:p.profileId="COMMERCIAL_HANGAR";p.brightCommercial=true;p.serviceDrones=6;p.ambientActivities={"cargo traffic","passenger shuttles","service drones"};break;
        case StationArchetype::IndustrialRefinery:case StationArchetype::MiningDepot:p.profileId="HEAVY_INDUSTRIAL_BERTH";p.serviceArms=6;p.serviceDrones=8;p.ambientActivities={"ore conveyors","refinery exhaust","cargo tugs"};break;
        case StationArchetype::Military:p.profileId="ARMORED_SERVICE_BAY";p.ambientActivities={"security patrols","munition loaders","repair crews"};break;
        case StationArchetype::Shipyard:p.profileId="CONSTRUCTION_FRAME";p.constructionFrame=true;p.width*=1.35f;p.length*=1.45f;p.ambientActivities={"gantry welders","construction drones","module cranes"};break;
        case StationArchetype::Research:p.profileId="RESEARCH_SPINDLE_BAY";p.brightCommercial=true;p.ambientActivities={"lab shuttles","sensor calibration","clean service drones"};break;
        case StationArchetype::AsteroidStation:p.profileId="EXCAVATED_ROCK_BERTH";p.excavatedRock=true;p.ambientActivities={"rock dust","ore haulers","tunnel lights"};break;
        case StationArchetype::TetherTerminal:p.profileId="ORBITAL_SERVICE_CLAMP";p.externalClamp=true;p.height*=.65f;p.ambientActivities={"tether cargo","surface lift traffic","service craft"};break;
        default:p.profileId="FRONTIER_HANGAR";p.ambientActivities={"utility traffic","repair drones"};break;
    }
    if(berth==StationBerthSize::Capital)p.externalClamp=true;
    return p;
}

} // namespace subspace
