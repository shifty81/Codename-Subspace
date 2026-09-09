#include "station/StationServiceEnvelopeSystem.h"

namespace subspace {

StationServiceEnvelopeProfile StationServiceEnvelopeSystem::Build(StationArchetype a,bool authorized){
    StationServiceEnvelopeProfile p;if(!authorized)return p;
    p.radius=58.0f;p.repairPerSecond=.35f;p.shieldPerSecond=.55f;p.refuelPerSecond=.40f;p.defenseCoverage=true;
    switch(a){
        case StationArchetype::TradeHub:p.radius=72.0f;p.refuelPerSecond=.65f;p.tractorAssistance=true;break;
        case StationArchetype::IndustrialRefinery:case StationArchetype::MiningDepot:p.radius=66.0f;p.repairPerSecond=.46f;p.refuelPerSecond=.55f;p.tractorAssistance=true;break;
        case StationArchetype::Shipyard:p.radius=82.0f;p.repairPerSecond=.85f;p.shieldPerSecond=.72f;p.refuelPerSecond=.78f;p.tractorAssistance=true;break;
        case StationArchetype::Military:p.radius=78.0f;p.repairPerSecond=.72f;p.shieldPerSecond=.82f;break;
        case StationArchetype::Research:p.radius=58.0f;p.shieldPerSecond=.48f;break;
        case StationArchetype::TetherTerminal:p.radius=86.0f;p.repairPerSecond=.62f;p.shieldPerSecond=.72f;p.refuelPerSecond=.72f;break;
        case StationArchetype::FrontierOutpost:p.radius=48.0f;p.repairPerSecond=.24f;p.shieldPerSecond=.28f;p.refuelPerSecond=.30f;break;
        case StationArchetype::AsteroidStation:p.radius=54.0f;p.repairPerSecond=.42f;p.refuelPerSecond=.44f;break;
        case StationArchetype::CorporateHQ:p.radius=82.0f;p.repairPerSecond=.68f;p.shieldPerSecond=.72f;p.refuelPerSecond=.62f;p.tractorAssistance=true;break;
    }
    return p;
}

} // namespace subspace
