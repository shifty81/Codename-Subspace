#include "home/HomeShipyardBuildQueue.h"

#include <algorithm>
#include <sstream>

namespace subspace {
namespace {
bool CanPay(const HomeFactoryNetworkState& inventory, const std::vector<HomeInventoryStack>& cost) {
    for (const auto& item : cost) {
        if (GetHomeInventoryUnits(inventory, item.commodity) < item.units) return false;
    }
    return true;
}

void Pay(HomeFactoryNetworkState& inventory, const std::vector<HomeInventoryStack>& cost) {
    for (const auto& item : cost) {
        AddHomeInventory(inventory, item.commodity, -item.units);
    }
}
}

ShipyardBuildQueueState CreateStarterShipyardBuildQueue(const ShipyardProgressionState& progression) {
    ShipyardBuildQueueState queue;
    queue.parallelCapacity = std::max(1, EstimateShipyardBuildCapacity(progression));
    return queue;
}

ShipyardBuildOrder CreateShipyardBuildOrder(ShipyardBuildOrderType type, const std::string& id, const std::string& displayName) {
    ShipyardBuildOrder order;
    order.id = id;
    order.type = type;
    order.displayName = displayName;
    switch (type) {
        case ShipyardBuildOrderType::Module:
            order.cost = {{"refined-metal", 12}, {"module-component", 4}};
            order.secondsRemaining = 20.0f;
            break;
        case ShipyardBuildOrderType::HullBlockBatch:
            order.cost = {{"refined-metal", 8}};
            order.secondsRemaining = 12.0f;
            break;
        case ShipyardBuildOrderType::Drone:
            order.cost = {{"module-component", 8}, {"electronics", 4}};
            order.secondsRemaining = 30.0f;
            break;
        case ShipyardBuildOrderType::ShipRepair:
            order.cost = {{"scrap", 10}, {"refined-metal", 4}};
            order.secondsRemaining = 15.0f;
            break;
        case ShipyardBuildOrderType::ShipFrame:
            order.cost = {{"refined-metal", 40}, {"module-component", 12}};
            order.secondsRemaining = 60.0f;
            break;
        case ShipyardBuildOrderType::DysonComponent:
            order.cost = {{"refined-metal", 80}, {"electronics", 18}, {"research-data", 3}};
            order.secondsRemaining = 120.0f;
            break;
    }
    return order;
}

bool QueueShipyardBuildOrder(ShipyardBuildQueueState& queue,
                             HomeFactoryNetworkState& inventory,
                             const ShipyardBuildOrder& order) {
    if (!CanPay(inventory, order.cost)) {
        return false;
    }
    Pay(inventory, order.cost);
    queue.queuedOrders.push_back(order);
    return true;
}

void TickShipyardBuildQueue(ShipyardBuildQueueState& queue, float deltaSeconds) {
    int active = 0;
    for (auto& order : queue.queuedOrders) {
        if (order.complete) continue;
        if (active >= queue.parallelCapacity) break;
        ++active;
        order.secondsRemaining -= deltaSeconds;
        if (order.secondsRemaining <= 0.0f) {
            order.complete = true;
            order.secondsRemaining = 0.0f;
            queue.completedOrders.push_back(order);
        }
    }
    queue.queuedOrders.erase(std::remove_if(queue.queuedOrders.begin(), queue.queuedOrders.end(), [](const ShipyardBuildOrder& order) {
        return order.complete;
    }), queue.queuedOrders.end());
}

std::string ShipyardBuildOrderTypeName(ShipyardBuildOrderType type) {
    switch (type) {
        case ShipyardBuildOrderType::Module: return "Module";
        case ShipyardBuildOrderType::HullBlockBatch: return "HullBlockBatch";
        case ShipyardBuildOrderType::Drone: return "Drone";
        case ShipyardBuildOrderType::ShipRepair: return "ShipRepair";
        case ShipyardBuildOrderType::ShipFrame: return "ShipFrame";
        case ShipyardBuildOrderType::DysonComponent: return "DysonComponent";
        default: return "Unknown";
    }
}

std::string ShipyardBuildQueueSummary(const ShipyardBuildQueueState& queue) {
    std::ostringstream stream;
    stream << "BuildQueue active=" << queue.queuedOrders.size() << " completed=" << queue.completedOrders.size()
           << " capacity=" << queue.parallelCapacity;
    return stream.str();
}

} // namespace subspace
