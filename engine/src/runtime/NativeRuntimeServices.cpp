#include "runtime/NativeRuntimeServices.h"

namespace subspace {

NativeRuntimeServices::NativeRuntimeServices() = default;

void NativeRuntimeServices::InitializeDefaults() {
    if (initialized_) return;
    initialized_ = true;
    economy.RegisterCommodity({"ore_iron",12.0,120.0,150.0,1.0,0.45});
    economy.RegisterCommodity({"plate_iron",32.0,60.0,90.0,1.1,0.35});
    economy.RegisterCommodity({"fuel_cell",18.0,80.0,100.0,1.2,0.30});
    economy.RegisterRecipe({"smelt_iron",{{"ore_iron",3}},"plate_iron",1,2.0});
    CombatWeaponSpec starterPdc; starterPdc.id="starter_point_defense"; starterPdc.family=AdvancedWeaponFamily::PointDefense; starterPdc.damage=6.0; starterPdc.heatPerShot=0.8; starterPdc.powerPerShot=1.5; starterPdc.cooldownSeconds=0.12; starterPdc.magazineCapacity=240;
    advancedCombat.Register(starterPdc);
    DynamicFactionState frontier; frontier.id="frontier_independents"; frontier.resources=120.0; frontier.military=45.0; frontier.industry=55.0; frontier.expansion=0.55; frontier.aggression=0.20;
    dynamicFactions.Register(frontier);
    FleetDoctrine convoy; convoy.id="protected_convoy"; convoy.formation=FleetFormation::Convoy; convoy.engagement=EngagementRule::Defensive; convoy.desiredCombatFraction=0.35; convoy.desiredSupportFraction=0.15; convoy.desiredIndustrialFraction=0.25;
    fleetDoctrine.Register(convoy);

    // Pass973-986: seed only identity/frame authority here. Runtime/render
    // entities remain projections from this persistent world state.
    SpatialFrame referenceFrame; referenceFrame.id=1;
    worldSimulation.Frames().Upsert(referenceFrame);
    const auto universeId=worldSimulation.Register(PersistentEntityKind::World,
        "subspace.runtime", "reference-universe", {}, 1, "reference-universe-v1");
    const auto systemId=worldSimulation.Register(PersistentEntityKind::SolarSystem,
        "subspace.runtime", "reference-system", universeId, 1, "reference-system-v1");
    worldSimulation.SetResident(systemId,true);
    worldSimulation.TransitionRepresentation(systemId,SimulationRepresentation::Full);
    developerConsole.Register("runtime_status",[this](const std::vector<std::string>&){
        return ConsoleResult{true,"native-cpp; cargo="+std::to_string(playerCargo.GetStacks().size())+"; commodities="+std::to_string(economy.CommodityCount())+"; "+worldSimulation.Diagnostics()+"; deep_sandbox=50"};
    });
    developerConsole.Register("world_authority",[this](const std::vector<std::string>&){
        return ConsoleResult{true,worldSimulation.Diagnostics()};
    });
}

void NativeRuntimeServices::Update(double deltaSeconds) {
    if(!initialized_)InitializeDefaults();
    if(deltaSeconds<=0.0)return;
    economy.AdvanceMarkets(deltaSeconds/60.0);
    fleetMissions.Update(deltaSeconds);
}

} // namespace subspace
