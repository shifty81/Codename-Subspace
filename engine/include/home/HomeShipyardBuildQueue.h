#pragma once

#include "home/HomeShipyardProgression.h"

#include <string>
#include <vector>

namespace subspace {

enum class ShipyardBuildOrderType {
    Module,
    HullBlockBatch,
    Drone,
    ShipRepair,
    ShipFrame,
    DysonComponent
};

struct ShipyardBuildOrder {
    std::string id;
    ShipyardBuildOrderType type = ShipyardBuildOrderType::Module;
    std::string displayName;
    std::vector<HomeInventoryStack> cost;
    float secondsRemaining = 0.0f;
    bool complete = false;
};

struct ShipyardBuildQueueState {
    std::vector<ShipyardBuildOrder> queuedOrders;
    std::vector<ShipyardBuildOrder> completedOrders;
    int parallelCapacity = 1;
};

ShipyardBuildQueueState CreateStarterShipyardBuildQueue(const ShipyardProgressionState& progression);
ShipyardBuildOrder CreateShipyardBuildOrder(ShipyardBuildOrderType type, const std::string& id, const std::string& displayName);
bool QueueShipyardBuildOrder(ShipyardBuildQueueState& queue,
                             HomeFactoryNetworkState& inventory,
                             const ShipyardBuildOrder& order);
void TickShipyardBuildQueue(ShipyardBuildQueueState& queue, float deltaSeconds);
std::string ShipyardBuildOrderTypeName(ShipyardBuildOrderType type);
std::string ShipyardBuildQueueSummary(const ShipyardBuildQueueState& queue);

} // namespace subspace
