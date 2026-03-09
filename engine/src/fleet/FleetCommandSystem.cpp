#include "fleet/FleetCommandSystem.h"

#include <algorithm>
#include <numeric>

namespace subspace {

// ---------------------------------------------------------------------------
// FleetOrder helpers
// ---------------------------------------------------------------------------

std::string FleetOrder::GetOrderTypeName(FleetOrderType type) {
    switch (type) {
        case FleetOrderType::Idle:    return "Idle";
        case FleetOrderType::Patrol:  return "Patrol";
        case FleetOrderType::Mine:    return "Mine";
        case FleetOrderType::Trade:   return "Trade";
        case FleetOrderType::Attack:  return "Attack";
        case FleetOrderType::Escort:  return "Escort";
        case FleetOrderType::Defend:  return "Defend";
        case FleetOrderType::Scout:   return "Scout";
    }
    return "Unknown";
}

std::string FleetOrder::GetOrderStateName(FleetOrderState state) {
    switch (state) {
        case FleetOrderState::Pending:   return "Pending";
        case FleetOrderState::Active:    return "Active";
        case FleetOrderState::Paused:    return "Paused";
        case FleetOrderState::Completed: return "Completed";
        case FleetOrderState::Failed:    return "Failed";
    }
    return "Unknown";
}

std::string FleetOrder::GetRoleName(FleetRole role) {
    switch (role) {
        case FleetRole::Flagship: return "Flagship";
        case FleetRole::Combat:   return "Combat";
        case FleetRole::Mining:   return "Mining";
        case FleetRole::Trading:  return "Trading";
        case FleetRole::Support:  return "Support";
        case FleetRole::Scout:    return "Scout";
    }
    return "Unknown";
}

// ---------------------------------------------------------------------------
// FleetCommandComponent
// ---------------------------------------------------------------------------

FleetCommandComponent::FleetCommandComponent(const std::string& fleetName)
    : _fleetName(fleetName)
{
}

const std::string& FleetCommandComponent::GetFleetName() const {
    return _fleetName;
}

void FleetCommandComponent::SetFleetName(const std::string& name) {
    _fleetName = name;
}

int FleetCommandComponent::GetMaxMembers() const {
    return _maxMembers;
}

void FleetCommandComponent::SetMaxMembers(int max) {
    _maxMembers = max;
}

int FleetCommandComponent::GetMemberCount() const {
    return static_cast<int>(_members.size());
}

int FleetCommandComponent::GetActiveMemberCount() const {
    int count = 0;
    for (const auto& m : _members) {
        if (m.isActive) ++count;
    }
    return count;
}

bool FleetCommandComponent::AddMember(uint64_t entityId, const std::string& shipName,
                                       FleetRole role) {
    if (static_cast<int>(_members.size()) >= _maxMembers) {
        return false;
    }

    // Check for duplicate
    for (const auto& m : _members) {
        if (m.entityId == entityId) return false;
    }

    FleetMember member;
    member.entityId = entityId;
    member.shipName = shipName;
    member.role = role;
    member.morale = 1.0f;
    member.isActive = true;

    _members.push_back(member);
    return true;
}

bool FleetCommandComponent::RemoveMember(uint64_t entityId) {
    auto it = std::find_if(_members.begin(), _members.end(),
        [entityId](const FleetMember& m) { return m.entityId == entityId; });

    if (it == _members.end()) return false;

    _members.erase(it);
    return true;
}

const FleetMember* FleetCommandComponent::GetMember(uint64_t entityId) const {
    for (const auto& m : _members) {
        if (m.entityId == entityId) return &m;
    }
    return nullptr;
}

const std::vector<FleetMember>& FleetCommandComponent::GetAllMembers() const {
    return _members;
}

bool FleetCommandComponent::IssueOrder(FleetOrderType type, float targetX,
                                        float targetY, float targetZ,
                                        uint64_t targetEntityId, int priority) {
    // Count active/pending orders against limit
    int activeCount = 0;
    for (const auto& o : _orders) {
        if (o.state == FleetOrderState::Active || o.state == FleetOrderState::Pending) {
            ++activeCount;
        }
    }
    if (activeCount >= _maxOrders) return false;

    FleetOrder order;
    order.orderId = _nextOrderId++;
    order.type = type;
    order.state = FleetOrderState::Pending;
    order.targetEntityId = targetEntityId;
    order.targetX = targetX;
    order.targetY = targetY;
    order.targetZ = targetZ;
    order.priority = priority;
    order.progress = 0.0f;

    _orders.push_back(order);
    return true;
}

bool FleetCommandComponent::CancelOrder(int orderId) {
    auto it = std::find_if(_orders.begin(), _orders.end(),
        [orderId](const FleetOrder& o) { return o.orderId == orderId; });

    if (it == _orders.end()) return false;

    _orders.erase(it);
    return true;
}

const FleetOrder* FleetCommandComponent::GetOrder(int orderId) const {
    for (const auto& o : _orders) {
        if (o.orderId == orderId) return &o;
    }
    return nullptr;
}

const std::vector<FleetOrder>& FleetCommandComponent::GetAllOrders() const {
    return _orders;
}

int FleetCommandComponent::GetActiveOrderCount() const {
    int count = 0;
    for (const auto& o : _orders) {
        if (o.state == FleetOrderState::Active) ++count;
    }
    return count;
}

float FleetCommandComponent::GetAverageMorale() const {
    if (_members.empty()) return 0.0f;

    float total = 0.0f;
    for (const auto& m : _members) {
        total += m.morale;
    }
    return total / static_cast<float>(_members.size());
}

bool FleetCommandComponent::SetMemberMorale(uint64_t entityId, float morale) {
    for (auto& m : _members) {
        if (m.entityId == entityId) {
            m.morale = morale;
            if (m.morale < 0.0f) m.morale = 0.0f;
            if (m.morale > 1.0f) m.morale = 1.0f;
            return true;
        }
    }
    return false;
}

bool FleetCommandComponent::SetMemberRole(uint64_t entityId, FleetRole role) {
    for (auto& m : _members) {
        if (m.entityId == entityId) {
            m.role = role;
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Serialization
// ---------------------------------------------------------------------------

ComponentData FleetCommandComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "FleetCommandComponent";
    cd.data["fleetName"]   = _fleetName;
    cd.data["maxMembers"]  = std::to_string(_maxMembers);
    cd.data["maxOrders"]   = std::to_string(_maxOrders);
    cd.data["nextOrderId"] = std::to_string(_nextOrderId);

    // Members
    cd.data["memberCount"] = std::to_string(_members.size());
    for (size_t i = 0; i < _members.size(); ++i) {
        std::string prefix = "member_" + std::to_string(i) + "_";
        const auto& m = _members[i];
        cd.data[prefix + "entityId"] = std::to_string(m.entityId);
        cd.data[prefix + "shipName"] = m.shipName;
        cd.data[prefix + "role"]     = std::to_string(static_cast<int>(m.role));
        cd.data[prefix + "morale"]   = std::to_string(m.morale);
        cd.data[prefix + "isActive"] = m.isActive ? "1" : "0";
    }

    // Orders
    cd.data["orderCount"] = std::to_string(_orders.size());
    for (size_t i = 0; i < _orders.size(); ++i) {
        std::string prefix = "order_" + std::to_string(i) + "_";
        const auto& o = _orders[i];
        cd.data[prefix + "orderId"]        = std::to_string(o.orderId);
        cd.data[prefix + "type"]           = std::to_string(static_cast<int>(o.type));
        cd.data[prefix + "state"]          = std::to_string(static_cast<int>(o.state));
        cd.data[prefix + "targetEntityId"] = std::to_string(o.targetEntityId);
        cd.data[prefix + "targetX"]        = std::to_string(o.targetX);
        cd.data[prefix + "targetY"]        = std::to_string(o.targetY);
        cd.data[prefix + "targetZ"]        = std::to_string(o.targetZ);
        cd.data[prefix + "priority"]       = std::to_string(o.priority);
        cd.data[prefix + "progress"]       = std::to_string(o.progress);
    }

    return cd;
}

void FleetCommandComponent::Deserialize(const ComponentData& data) {
    auto getStr = [&](const std::string& key) -> std::string {
        auto it = data.data.find(key);
        return it != data.data.end() ? it->second : "";
    };
    auto getInt = [&](const std::string& key, int def = 0) -> int {
        auto it = data.data.find(key);
        if (it == data.data.end()) return def;
        try { return std::stoi(it->second); } catch (...) { return def; }
    };
    auto getFloat = [&](const std::string& key, float def = 0.0f) -> float {
        auto it = data.data.find(key);
        if (it == data.data.end()) return def;
        try { return std::stof(it->second); } catch (...) { return def; }
    };
    auto getUint64 = [&](const std::string& key, uint64_t def = 0) -> uint64_t {
        auto it = data.data.find(key);
        if (it == data.data.end()) return def;
        try { return std::stoull(it->second); } catch (...) { return def; }
    };

    _fleetName   = getStr("fleetName");
    if (_fleetName.empty()) _fleetName = "Fleet";
    _maxMembers  = getInt("maxMembers", 10);
    _maxOrders   = getInt("maxOrders", 5);
    _nextOrderId = getInt("nextOrderId", 1);

    // Members
    int memberCount = getInt("memberCount", 0);
    _members.clear();
    _members.reserve(static_cast<size_t>(memberCount));
    for (int i = 0; i < memberCount; ++i) {
        std::string prefix = "member_" + std::to_string(i) + "_";
        FleetMember m;
        m.entityId = getUint64(prefix + "entityId", 0);
        m.shipName = getStr(prefix + "shipName");
        int roleVal = getInt(prefix + "role", 1);
        constexpr int kMaxRole = static_cast<int>(FleetRole::Scout);
        if (roleVal >= 0 && roleVal <= kMaxRole) {
            m.role = static_cast<FleetRole>(roleVal);
        } else {
            m.role = FleetRole::Combat;
        }
        m.morale  = getFloat(prefix + "morale", 1.0f);
        m.isActive = getInt(prefix + "isActive", 1) != 0;
        _members.push_back(m);
    }

    // Orders
    int orderCount = getInt("orderCount", 0);
    _orders.clear();
    _orders.reserve(static_cast<size_t>(orderCount));
    for (int i = 0; i < orderCount; ++i) {
        std::string prefix = "order_" + std::to_string(i) + "_";
        FleetOrder o;
        o.orderId = getInt(prefix + "orderId", 0);
        int typeVal = getInt(prefix + "type", 0);
        constexpr int kMaxType = static_cast<int>(FleetOrderType::Scout);
        if (typeVal >= 0 && typeVal <= kMaxType) {
            o.type = static_cast<FleetOrderType>(typeVal);
        } else {
            o.type = FleetOrderType::Idle;
        }
        int stateVal = getInt(prefix + "state", 0);
        constexpr int kMaxState = static_cast<int>(FleetOrderState::Failed);
        if (stateVal >= 0 && stateVal <= kMaxState) {
            o.state = static_cast<FleetOrderState>(stateVal);
        } else {
            o.state = FleetOrderState::Pending;
        }
        o.targetEntityId = getUint64(prefix + "targetEntityId", 0);
        o.targetX  = getFloat(prefix + "targetX", 0.0f);
        o.targetY  = getFloat(prefix + "targetY", 0.0f);
        o.targetZ  = getFloat(prefix + "targetZ", 0.0f);
        o.priority = getInt(prefix + "priority", 0);
        o.progress = getFloat(prefix + "progress", 0.0f);
        _orders.push_back(o);
    }
}

// ---------------------------------------------------------------------------
// FleetCommandSystem
// ---------------------------------------------------------------------------

FleetCommandSystem::FleetCommandSystem() : SystemBase("FleetCommandSystem") {}

FleetCommandSystem::FleetCommandSystem(EntityManager& entityManager)
    : SystemBase("FleetCommandSystem")
    , _entityManager(&entityManager)
{
}

void FleetCommandSystem::SetEntityManager(EntityManager* em) {
    _entityManager = em;
}

void FleetCommandSystem::Update(float deltaTime) {
    if (!_entityManager) return;

    auto fleets = _entityManager->GetAllComponents<FleetCommandComponent>();
    for (auto* fleet : fleets) {
        for (auto& order : fleet->_orders) {
            switch (order.state) {
                case FleetOrderState::Pending: {
                    order.state = FleetOrderState::Active;
                    break;
                }

                case FleetOrderState::Active: {
                    // Base order time: 15 seconds to complete
                    float baseOrderTime = 15.0f;
                    order.progress += deltaTime / baseOrderTime;

                    if (order.progress >= 1.0f) {
                        order.progress = 1.0f;
                        order.state = FleetOrderState::Completed;
                    }
                    break;
                }

                default:
                    break;
            }
        }
    }
}

} // namespace subspace
