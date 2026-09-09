#include "station/StationDesignGrammarSystem.h"

#include <algorithm>

namespace subspace {
StationDesignGrammar StationDesignGrammarSystem::ForArchetype(StationArchetype a, bool asteroidEmbedded) {
    StationDesignGrammar g;
    g.archetype = a;
    g.asteroidEmbedded = asteroidEmbedded;
    g.requiredPieces = {StationKitbashPieceRole::CoreHub, StationKitbashPieceRole::StructuralSpine, StationKitbashPieceRole::DockCollar};
    g.optionalPieces = {StationKitbashPieceRole::HabitationPod, StationKitbashPieceRole::StoragePod, StationKitbashPieceRole::DroneBay};
    switch (a) {
        case StationArchetype::TradeHub:
            g.id="trade_hub.radial_market"; g.branchCount=6; g.spineSegments=2; g.branchLengthScale=1.15f;
            g.requiredPieces.insert(g.requiredPieces.end(), {StationKitbashPieceRole::MarketConcourse, StationKitbashPieceRole::CargoHub, StationKitbashPieceRole::HabitationCluster, StationKitbashPieceRole::LogisticsNode});
            break;
        case StationArchetype::IndustrialRefinery:
            g.id="industrial_refinery.spine"; g.branchCount=5; g.spineSegments=3; g.branchLengthScale=1.25f;
            g.requiredPieces.insert(g.requiredPieces.end(), {StationKitbashPieceRole::RefineryPod, StationKitbashPieceRole::TankFarm, StationKitbashPieceRole::PowerTruss, StationKitbashPieceRole::IndustrialBrace});
            break;
        case StationArchetype::MiningDepot:
            g.id="mining_depot.rugged"; g.branchCount=4; g.spineSegments=2; g.branchLengthScale=1.05f;
            g.requiredPieces.insert(g.requiredPieces.end(), {StationKitbashPieceRole::RefineryPod, StationKitbashPieceRole::StoragePod, StationKitbashPieceRole::IndustrialBrace});
            break;
        case StationArchetype::Shipyard:
            g.id="shipyard.gantry_spine"; g.branchCount=4; g.spineSegments=4; g.radial=false; g.branchLengthScale=1.45f;
            g.requiredPieces.insert(g.requiredPieces.end(), {StationKitbashPieceRole::ShipyardGantry, StationKitbashPieceRole::HangarArm, StationKitbashPieceRole::RepairBay, StationKitbashPieceRole::PowerTruss, StationKitbashPieceRole::StoragePod});
            break;
        case StationArchetype::Military:
            g.id="military_citadel.radial_defense"; g.branchCount=6; g.spineSegments=2; g.branchLengthScale=1.15f;
            g.requiredPieces.insert(g.requiredPieces.end(), {StationKitbashPieceRole::DefenseRing, StationKitbashPieceRole::DefensePylon, StationKitbashPieceRole::CommandSpire, StationKitbashPieceRole::SensorCrown});
            break;
        case StationArchetype::Research:
            g.id="research_array.sensor_spokes"; g.branchCount=5; g.spineSegments=2;
            g.requiredPieces.insert(g.requiredPieces.end(), {StationKitbashPieceRole::ResearchLab, StationKitbashPieceRole::SensorCrown, StationKitbashPieceRole::PowerTruss});
            break;
        case StationArchetype::TetherTerminal:
            g.id="tether_terminal.linear"; g.branchCount=3; g.spineSegments=5; g.radial=false; g.branchLengthScale=1.35f;
            g.requiredPieces.insert(g.requiredPieces.end(), {StationKitbashPieceRole::ElevatorTether, StationKitbashPieceRole::LogisticsNode, StationKitbashPieceRole::MarketConcourse});
            break;
        case StationArchetype::FrontierOutpost:
            g.id="frontier_outpost.compact"; g.branchCount=3; g.spineSegments=1; g.branchLengthScale=.85f;
            g.requiredPieces.insert(g.requiredPieces.end(), {StationKitbashPieceRole::DefensePylon, StationKitbashPieceRole::StoragePod, StationKitbashPieceRole::HabitationPod});
            break;
        case StationArchetype::AsteroidStation:
            g.id="asteroid_station.embedded"; g.branchCount=3; g.spineSegments=2; g.branchLengthScale=.75f;
            g.requiredPieces.insert(g.requiredPieces.end(), {StationKitbashPieceRole::DockNeck, StationKitbashPieceRole::IndustrialBrace, StationKitbashPieceRole::StoragePod});
            break;
        case StationArchetype::CorporateHQ:
            g.id="corporate_hq.crown"; g.branchCount=6; g.spineSegments=2; g.branchLengthScale=1.20f;
            g.requiredPieces.insert(g.requiredPieces.end(), {StationKitbashPieceRole::CommandSpire, StationKitbashPieceRole::MarketConcourse, StationKitbashPieceRole::HabitationCluster, StationKitbashPieceRole::SensorCrown});
            break;
    }
    return g;
}

bool StationDesignGrammarSystem::Requires(const StationDesignGrammar& g, StationKitbashPieceRole role) {
    return std::find(g.requiredPieces.begin(), g.requiredPieces.end(), role) != g.requiredPieces.end();
}
} // namespace subspace
