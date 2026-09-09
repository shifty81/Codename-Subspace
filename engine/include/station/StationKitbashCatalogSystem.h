#pragma once

#include "content/ShipyardModuleSystem.h"
#include "station/StationModuleRole.h"

#include <string>
#include <vector>

namespace subspace {

enum class StationKitbashPieceRole {
    CoreHub,
    StructuralSpine,
    CrossJunction,
    TJunction,
    DockNeck,
    IndustrialBrace,
    PowerTruss,
    LogisticsNode,
    DockCollar,
    HangarArm,
    HabitationPod,
    HabitationCluster,
    StoragePod,
    CargoHub,
    TankFarm,
    RefineryPod,
    ManufacturingPod,
    ReactorNode,
    ResearchLab,
    CommandSpire,
    DefensePylon,
    DefenseRing,
    SensorCrown,
    ShipyardGantry,
    RepairBay,
    MarketConcourse,
    DroneBay,
    ElevatorTether
};

struct StationKitbashPiece {
    std::string id;
    std::string displayName;
    StationKitbashPieceRole role = StationKitbashPieceRole::CoreHub;
    StationModuleRole function = StationModuleRole::Core;
    std::string sourceModuleId;
    bool structural = false;
    bool radialFriendly = false;
    bool generatorEligible = false;
};

class StationKitbashCatalogSystem {
public:
    static std::vector<StationKitbashPiece> Build(const std::vector<ShipyardModuleRecord>& catalog);
    static const StationKitbashPiece* Find(const std::vector<StationKitbashPiece>& pieces,
                                           StationKitbashPieceRole role);
    static const char* RoleName(StationKitbashPieceRole role);
};

} // namespace subspace
