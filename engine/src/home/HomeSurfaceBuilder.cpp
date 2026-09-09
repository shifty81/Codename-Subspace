#include "home/HomeSurfaceBuilder.h"

#include <algorithm>
#include <sstream>

namespace subspace {
namespace {

std::string ToStructureId(HomeStructureType type, const std::string& zoneId, int x, int y, int count) {
    std::ostringstream out;
    out << HomeStructureTypeName(type) << "-" << zoneId << "-" << x << "-" << y << "-" << count;
    return out.str();
}

bool HasInputs(const HomeFactoryNetworkState& network, const std::vector<HomeInventoryStack>& inputs) {
    for (const auto& input : inputs) {
        if (GetHomeInventoryUnits(network, input.commodity) < input.units) {
            return false;
        }
    }
    return true;
}

void ConsumeInputs(HomeFactoryNetworkState& network,
                   const std::vector<HomeInventoryStack>& inputs,
                   std::vector<HomeInventoryStack>& consumed) {
    for (const auto& input : inputs) {
        AddHomeInventory(network, input.commodity, -input.units);
        consumed.push_back(input);
    }
}

bool IsProtectedStarterStructure(const HomeStructure& structure) {
    return structure.id == "landing-pad-01" || structure.id == "shipyard-bay-01";
}

} // namespace

std::vector<HomeStructureType> CreateHomeBuildPalette(HomeBuildZoneType zoneType) {
    switch (zoneType) {
        case HomeBuildZoneType::PlanetSurface:
        case HomeBuildZoneType::MoonSurface:
        case HomeBuildZoneType::AsteroidSurface:
            return {
                HomeStructureType::Extractor,
                HomeStructureType::ConveyorHub,
                HomeStructureType::StorageDepot,
                HomeStructureType::Refinery,
                HomeStructureType::Assembler,
                HomeStructureType::PowerRelay,
                HomeStructureType::DroneDepot,
                HomeStructureType::ResearchLab
            };
        case HomeBuildZoneType::OrbitalPlatform:
            return {
                HomeStructureType::StorageDepot,
                HomeStructureType::Assembler,
                HomeStructureType::PowerRelay,
                HomeStructureType::SolarCollector,
                HomeStructureType::DroneDepot,
                HomeStructureType::ShipyardBay,
                HomeStructureType::OrbitalRingSegment,
                HomeStructureType::SubspaceAnchor
            };
        case HomeBuildZoneType::SolarOrbit:
            return {
                HomeStructureType::SolarCollector,
                HomeStructureType::PowerRelay,
                HomeStructureType::DysonSwarmNode,
                HomeStructureType::OrbitalRingSegment,
                HomeStructureType::SubspaceAnchor
            };
        case HomeBuildZoneType::GasGiantOrbit:
        case HomeBuildZoneType::DeepSpaceAnchor:
            return {
                HomeStructureType::PowerRelay,
                HomeStructureType::StorageDepot,
                HomeStructureType::DroneDepot,
                HomeStructureType::SubspaceAnchor
            };
    }
    return {HomeStructureType::StorageDepot};
}

HomeStructureCost GetHomeStructureCost(HomeStructureType type, int tier) {
    HomeStructureCost cost;
    cost.type = type;
    cost.tier = std::max(1, tier);
    const int t = cost.tier;
    switch (type) {
        case HomeStructureType::Extractor:
            cost.inputs = {{"hull-plate", 2 * t}, {"recovered-parts", 1 * t}};
            cost.automationDelta = 1;
            break;
        case HomeStructureType::ConveyorHub:
            cost.inputs = {{"hull-plate", 1 * t}, {"ingot", 2 * t}};
            break;
        case HomeStructureType::StorageDepot:
            cost.inputs = {{"hull-plate", 2 * t}, {"ingot", 2 * t}};
            break;
        case HomeStructureType::Refinery:
            cost.inputs = {{"hull-plate", 3 * t}, {"module-component", 1 * t}};
            cost.automationDelta = 1;
            break;
        case HomeStructureType::Assembler:
            cost.inputs = {{"hull-plate", 4 * t}, {"module-component", 2 * t}};
            cost.automationDelta = 1;
            break;
        case HomeStructureType::PowerRelay:
            cost.inputs = {{"hull-plate", 1 * t}, {"module-component", 1 * t}};
            cost.powerDelta = 25 * t;
            break;
        case HomeStructureType::SolarCollector:
            cost.inputs = {{"hull-plate", 3 * t}, {"module-component", 1 * t}};
            cost.powerDelta = 100 * t;
            break;
        case HomeStructureType::DroneDepot:
            cost.inputs = {{"hull-plate", 4 * t}, {"module-component", 3 * t}, {"fuel", 2 * t}};
            cost.automationDelta = 4 * t;
            break;
        case HomeStructureType::ResearchLab:
            cost.inputs = {{"hull-plate", 6 * t}, {"module-component", 4 * t}, {"research-data", 1 * t}};
            cost.automationDelta = 1 * t;
            break;
        case HomeStructureType::ShipyardBay:
            cost.inputs = {{"hull-plate", 10 * t}, {"module-component", 6 * t}};
            break;
        case HomeStructureType::DysonSwarmNode:
            cost.inputs = {{"hull-plate", 15 * t}, {"module-component", 10 * t}, {"exotic-matter", 1 * t}};
            cost.powerDelta = 750 * t;
            cost.automationDelta = 2 * t;
            break;
        case HomeStructureType::OrbitalRingSegment:
            cost.inputs = {{"hull-plate", 8 * t}, {"module-component", 4 * t}};
            cost.powerDelta = 250 * t;
            break;
        case HomeStructureType::SubspaceAnchor:
            cost.inputs = {{"hull-plate", 12 * t}, {"module-component", 8 * t}, {"subspace-crystal", 1 * t}};
            break;
        case HomeStructureType::LandingPad:
            cost.inputs = {{"hull-plate", 4 * t}, {"ingot", 4 * t}};
            break;
        case HomeStructureType::Unknown:
            break;
    }
    return cost;
}

bool IsHomeStructureAllowedInZone(HomeStructureType type, HomeBuildZoneType zoneType) {
    const auto palette = CreateHomeBuildPalette(zoneType);
    return std::find(palette.begin(), palette.end(), type) != palette.end();
}

const HomeBuildZone* FindHomeBuildZone(const HomeSolarSystemState& home, const std::string& zoneId) {
    const auto found = std::find_if(home.buildZones.begin(), home.buildZones.end(), [&](const HomeBuildZone& zone) {
        return zone.id == zoneId;
    });
    return found == home.buildZones.end() ? nullptr : &(*found);
}

const HomeStructure* FindHomeStructureAt(const HomeSolarSystemState& home, const std::string& zoneId, int x, int y) {
    const auto found = std::find_if(home.structures.begin(), home.structures.end(), [&](const HomeStructure& structure) {
        return structure.zoneId == zoneId && structure.x == x && structure.y == y;
    });
    return found == home.structures.end() ? nullptr : &(*found);
}

HomeStructure* FindHomeStructureAt(HomeSolarSystemState& home, const std::string& zoneId, int x, int y) {
    const auto found = std::find_if(home.structures.begin(), home.structures.end(), [&](const HomeStructure& structure) {
        return structure.zoneId == zoneId && structure.x == x && structure.y == y;
    });
    return found == home.structures.end() ? nullptr : &(*found);
}

HomeBuildPlacementResult PlaceHomeStructure(HomeSolarSystemState& home,
                                            HomeFactoryNetworkState& network,
                                            const HomeBuildPlacementRequest& request) {
    HomeBuildPlacementResult result;
    result.status = HomePlacementStatus::InvalidStructure;
    if (request.type == HomeStructureType::Unknown) {
        result.message = "Unknown home structure type.";
        return result;
    }

    const HomeBuildZone* zone = FindHomeBuildZone(home, request.zoneId);
    if (!zone) {
        result.status = HomePlacementStatus::InvalidZone;
        result.message = "Unknown home build zone: " + request.zoneId;
        return result;
    }
    if (request.x < 0 || request.y < 0 || request.x >= zone->gridWidth || request.y >= zone->gridHeight) {
        result.status = HomePlacementStatus::OutOfBounds;
        result.message = "Build cursor is outside the selected home build zone.";
        return result;
    }
    if (FindHomeStructureAt(home, request.zoneId, request.x, request.y)) {
        result.status = HomePlacementStatus::Occupied;
        result.message = "Build tile already contains a structure.";
        return result;
    }
    if (!IsHomeStructureAllowedInZone(request.type, zone->type)) {
        result.status = HomePlacementStatus::InvalidStructure;
        result.message = HomeStructureTypeName(request.type) + " cannot be placed in " + HomeBuildZoneTypeName(zone->type) + ".";
        return result;
    }

    const auto cost = GetHomeStructureCost(request.type, request.tier);
    if (!request.freeBuild && !HasInputs(network, cost.inputs)) {
        result.status = HomePlacementStatus::InsufficientResources;
        result.message = "Insufficient home inventory for " + HomeStructureTypeName(request.type) + ": " + HomeStructureCostSummary(cost);
        return result;
    }

    if (!request.freeBuild) {
        ConsumeInputs(network, cost.inputs, result.consumed);
    }

    HomeStructure structure = CreateHomeStructure(
        ToStructureId(request.type, request.zoneId, request.x, request.y, static_cast<int>(home.structures.size()) + 1),
        request.type,
        request.zoneId,
        request.x,
        request.y,
        request.tier);
    home.structures.push_back(structure);
    RecalculateHomeDerivedState(home, &network);

    result.success = true;
    result.status = HomePlacementStatus::Success;
    result.placedStructure = structure;
    result.message = "Placed " + HomeStructureTypeName(request.type) + " in " + zone->displayName + ".";
    return result;
}

HomeBuildPlacementResult RemoveHomeStructure(HomeSolarSystemState& home,
                                             HomeFactoryNetworkState& network,
                                             const std::string& zoneId,
                                             int x,
                                             int y) {
    HomeBuildPlacementResult result;
    auto found = std::find_if(home.structures.begin(), home.structures.end(), [&](const HomeStructure& structure) {
        return structure.zoneId == zoneId && structure.x == x && structure.y == y;
    });
    if (found == home.structures.end()) {
        result.status = HomePlacementStatus::Occupied;
        result.message = "No structure exists at the selected build tile.";
        return result;
    }
    if (IsProtectedStarterStructure(*found)) {
        result.status = HomePlacementStatus::ProtectedStarterStructure;
        result.message = "Starter structure is protected in this early build mode.";
        return result;
    }

    result.placedStructure = *found;
    home.structures.erase(found);
    RecalculateHomeDerivedState(home, &network);
    result.success = true;
    result.status = HomePlacementStatus::Success;
    result.message = "Removed " + HomeStructureTypeName(result.placedStructure.type) + ".";
    return result;
}

void RecalculateHomeDerivedState(HomeSolarSystemState& home, HomeFactoryNetworkState* network) {
    home.automationBandwidthCap = EstimateHomeAutomationCapacity(home);
    home.automationBandwidthUsed = 0;
    for (const auto& structure : home.structures) {
        if (structure.automated) {
            home.automationBandwidthUsed += std::max(1, structure.tier);
        }
    }
    if (network) {
        network->availablePower = EstimateHomePowerGeneration(home);
    }
}

std::string HomePlacementStatusName(HomePlacementStatus status) {
    switch (status) {
        case HomePlacementStatus::Success: return "Success";
        case HomePlacementStatus::InvalidZone: return "InvalidZone";
        case HomePlacementStatus::OutOfBounds: return "OutOfBounds";
        case HomePlacementStatus::Occupied: return "Occupied";
        case HomePlacementStatus::InsufficientResources: return "InsufficientResources";
        case HomePlacementStatus::InvalidStructure: return "InvalidStructure";
        case HomePlacementStatus::ProtectedStarterStructure: return "ProtectedStarterStructure";
    }
    return "Unknown";
}

std::string HomeStructureCostSummary(const HomeStructureCost& cost) {
    std::ostringstream out;
    out << HomeStructureTypeName(cost.type) << " tier " << cost.tier << " cost";
    if (cost.inputs.empty()) {
        out << " none";
    }
    for (const auto& input : cost.inputs) {
        out << " " << input.commodity << "=" << input.units;
    }
    if (cost.powerDelta != 0) {
        out << " power+" << cost.powerDelta;
    }
    if (cost.automationDelta != 0) {
        out << " automation+" << cost.automationDelta;
    }
    return out.str();
}

} // namespace subspace
