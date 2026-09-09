#include "station/StationKitbashCatalogSystem.h"

#include <algorithm>
#include <array>

namespace subspace {
namespace {

enum class Family { Structural, Command, Service, Sensor, Hardpoint, Propulsion };

bool Matches(const ShipyardModuleRecord& r, Family f) {
    switch (f) {
        case Family::Structural:
            return r.partRole == ShipyardPartRole::PrimaryHull ||
                   r.partRole == ShipyardPartRole::StructuralFrame ||
                   r.partRole == ShipyardPartRole::StructuralAttachment ||
                   r.partRole == ShipyardPartRole::StructuralBrace ||
                   r.partRole == ShipyardPartRole::StructuralBlock ||
                   r.semantic == ShipyardModuleSemantic::HullMid ||
                   r.semantic == ShipyardModuleSemantic::StructuralFrame ||
                   r.semantic == ShipyardModuleSemantic::Adapter;
        case Family::Command:
            return r.partRole == ShipyardPartRole::Bridge ||
                   r.partRole == ShipyardPartRole::Cockpit ||
                   r.semantic == ShipyardModuleSemantic::CommandBridge ||
                   r.semantic == ShipyardModuleSemantic::CommandCockpit;
        case Family::Service:
            return r.partRole == ShipyardPartRole::Cargo ||
                   r.partRole == ShipyardPartRole::Tank ||
                   r.partRole == ShipyardPartRole::Hangar ||
                   r.partRole == ShipyardPartRole::SurfaceDetail ||
                   r.semantic == ShipyardModuleSemantic::Component;
        case Family::Sensor:
            return r.partRole == ShipyardPartRole::SensorDish ||
                   r.partRole == ShipyardPartRole::SensorMast ||
                   r.partRole == ShipyardPartRole::SensorAntenna ||
                   r.partRole == ShipyardPartRole::Telescope ||
                   r.semantic == ShipyardModuleSemantic::Sensor;
        case Family::Hardpoint:
            return r.partRole == ShipyardPartRole::HardpointBase ||
                   r.partRole == ShipyardPartRole::WeaponTurret ||
                   r.partRole == ShipyardPartRole::MissileMount ||
                   r.semantic == ShipyardModuleSemantic::TurretHardpoint ||
                   r.semantic == ShipyardModuleSemantic::WeaponMount;
        case Family::Propulsion:
            return r.partRole == ShipyardPartRole::EngineHousing ||
                   r.partRole == ShipyardPartRole::EngineMount ||
                   r.partRole == ShipyardPartRole::MainEngine ||
                   r.semantic == ShipyardModuleSemantic::EngineHousing ||
                   r.semantic == ShipyardModuleSemantic::MainEngine;
    }
    return false;
}

const ShipyardModuleRecord* Pick(const std::vector<ShipyardModuleRecord>& catalog,
                                 Family family,
                                 std::size_t salt) {
    std::vector<const ShipyardModuleRecord*> candidates;
    for (const auto& r : catalog) {
        if (r.generatorEligible && Matches(r, family)) candidates.push_back(&r);
    }
    if (candidates.empty() && family != Family::Structural) {
        for (const auto& r : catalog) {
            if (r.generatorEligible && Matches(r, Family::Structural)) candidates.push_back(&r);
        }
    }
    if (candidates.empty()) return nullptr;
    std::sort(candidates.begin(), candidates.end(), [](const auto* a, const auto* b) {
        return a->source.moduleId < b->source.moduleId;
    });
    return candidates[salt % candidates.size()];
}

struct Spec {
    StationKitbashPieceRole role;
    const char* id;
    const char* name;
    StationModuleRole function;
    Family family;
    bool structural;
    bool radial;
};

constexpr std::array<Spec, 28> kSpecs{{
    {StationKitbashPieceRole::CoreHub, "station.core_hub", "Core Hub", StationModuleRole::Core, Family::Structural, true, true},
    {StationKitbashPieceRole::StructuralSpine, "station.structural_spine", "Structural Spine", StationModuleRole::Core, Family::Structural, true, false},
    {StationKitbashPieceRole::CrossJunction, "station.cross_junction", "Cross Junction", StationModuleRole::Logistics, Family::Structural, true, true},
    {StationKitbashPieceRole::TJunction, "station.t_junction", "T Junction", StationModuleRole::Logistics, Family::Structural, true, true},
    {StationKitbashPieceRole::DockNeck, "station.dock_neck", "Dock Neck", StationModuleRole::Dock, Family::Structural, true, false},
    {StationKitbashPieceRole::IndustrialBrace, "station.industrial_brace", "Industrial Brace", StationModuleRole::Manufacturing, Family::Structural, true, false},
    {StationKitbashPieceRole::PowerTruss, "station.power_truss", "Power Truss", StationModuleRole::Power, Family::Structural, true, false},
    {StationKitbashPieceRole::LogisticsNode, "station.logistics_node", "Logistics Node", StationModuleRole::Logistics, Family::Structural, true, true},
    {StationKitbashPieceRole::DockCollar, "station.dock_collar", "Dock Collar", StationModuleRole::Dock, Family::Structural, true, true},
    {StationKitbashPieceRole::HangarArm, "station.hangar_arm", "Hangar Arm", StationModuleRole::Shipyard, Family::Service, false, false},
    {StationKitbashPieceRole::HabitationPod, "station.habitation_pod", "Habitation Pod", StationModuleRole::Habitation, Family::Service, false, true},
    {StationKitbashPieceRole::HabitationCluster, "station.habitation_cluster", "Habitation Cluster", StationModuleRole::Habitation, Family::Service, false, true},
    {StationKitbashPieceRole::StoragePod, "station.storage_pod", "Storage Pod", StationModuleRole::Storage, Family::Service, false, true},
    {StationKitbashPieceRole::CargoHub, "station.cargo_hub", "Cargo Hub", StationModuleRole::Storage, Family::Service, false, true},
    {StationKitbashPieceRole::TankFarm, "station.tank_farm", "Tank Farm", StationModuleRole::Storage, Family::Service, false, true},
    {StationKitbashPieceRole::RefineryPod, "station.refinery_pod", "Refinery Pod", StationModuleRole::Refinery, Family::Service, false, true},
    {StationKitbashPieceRole::ManufacturingPod, "station.manufacturing_pod", "Manufacturing Pod", StationModuleRole::Manufacturing, Family::Service, false, true},
    {StationKitbashPieceRole::ReactorNode, "station.reactor_node", "Reactor Node", StationModuleRole::Power, Family::Propulsion, false, true},
    {StationKitbashPieceRole::ResearchLab, "station.research_lab", "Research Lab", StationModuleRole::Research, Family::Service, false, true},
    {StationKitbashPieceRole::CommandSpire, "station.command_spire", "Command Spire", StationModuleRole::Core, Family::Command, false, false},
    {StationKitbashPieceRole::DefensePylon, "station.defense_pylon", "Defense Pylon", StationModuleRole::Defense, Family::Hardpoint, false, true},
    {StationKitbashPieceRole::DefenseRing, "station.defense_ring", "Defense Ring", StationModuleRole::Defense, Family::Structural, true, true},
    {StationKitbashPieceRole::SensorCrown, "station.sensor_crown", "Sensor Crown", StationModuleRole::Sensor, Family::Sensor, false, true},
    {StationKitbashPieceRole::ShipyardGantry, "station.shipyard_gantry", "Shipyard Gantry", StationModuleRole::Shipyard, Family::Structural, true, false},
    {StationKitbashPieceRole::RepairBay, "station.repair_bay", "Repair Bay", StationModuleRole::Repair, Family::Service, false, false},
    {StationKitbashPieceRole::MarketConcourse, "station.market_concourse", "Market Concourse", StationModuleRole::Market, Family::Service, false, true},
    {StationKitbashPieceRole::DroneBay, "station.drone_bay", "Drone Bay", StationModuleRole::DroneControl, Family::Service, false, true},
    {StationKitbashPieceRole::ElevatorTether, "station.elevator_tether", "Elevator Tether", StationModuleRole::ElevatorInterface, Family::Structural, true, false}
}};

} // namespace

const char* StationKitbashCatalogSystem::RoleName(StationKitbashPieceRole role) {
    for (const auto& s : kSpecs) if (s.role == role) return s.name;
    return "Station Piece";
}

std::vector<StationKitbashPiece> StationKitbashCatalogSystem::Build(const std::vector<ShipyardModuleRecord>& catalog) {
    std::vector<StationKitbashPiece> out;
    out.reserve(kSpecs.size());
    std::size_t salt = 0;
    for (const auto& spec : kSpecs) {
        const auto* source = Pick(catalog, spec.family, salt++ * 7u + static_cast<std::size_t>(spec.role));
        StationKitbashPiece piece;
        piece.id = spec.id;
        piece.displayName = spec.name;
        piece.role = spec.role;
        piece.function = spec.function;
        piece.structural = spec.structural;
        piece.radialFriendly = spec.radial;
        if (source) {
            piece.sourceModuleId = source->source.moduleId;
            piece.generatorEligible = true;
        }
        out.push_back(std::move(piece));
    }
    return out;
}

const StationKitbashPiece* StationKitbashCatalogSystem::Find(const std::vector<StationKitbashPiece>& pieces,
                                                              StationKitbashPieceRole role) {
    const auto it = std::find_if(pieces.begin(), pieces.end(), [role](const auto& p) { return p.role == role; });
    return it == pieces.end() ? nullptr : &*it;
}

} // namespace subspace
