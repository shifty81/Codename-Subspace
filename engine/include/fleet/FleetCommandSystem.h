#pragma once

#include "core/ecs/Entity.h"
#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/ecs/EntityManager.h"
#include "core/persistence/SaveGameManager.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace subspace {

/// Type of order that can be issued to a fleet.
enum class FleetOrderType {
    Idle,
    Patrol,
    Mine,
    Trade,
    Attack,
    Escort,
    Defend,
    Scout
};

/// State of a fleet order.
enum class FleetOrderState {
    Pending,
    Active,
    Paused,
    Completed,
    Failed
};

/// Role of a fleet member.
enum class FleetRole {
    Flagship,
    Combat,
    Mining,
    Trading,
    Support,
    Scout
};

/// A single order assigned to a fleet.
struct FleetOrder {
    int orderId = 0;
    FleetOrderType type = FleetOrderType::Idle;
    FleetOrderState state = FleetOrderState::Pending;
    uint64_t targetEntityId = 0;
    float targetX = 0.0f, targetY = 0.0f, targetZ = 0.0f;
    int priority = 0;
    float progress = 0.0f;   ///< 0 to 1

    /// Get the display name for an order type.
    static std::string GetOrderTypeName(FleetOrderType type);

    /// Get the display name for an order state.
    static std::string GetOrderStateName(FleetOrderState state);

    /// Get the display name for a fleet role.
    static std::string GetRoleName(FleetRole role);
};

/// A ship that is a member of a fleet.
struct FleetMember {
    uint64_t entityId = 0;
    std::string shipName;
    FleetRole role = FleetRole::Combat;
    float morale = 1.0f;   ///< 0 to 1
    bool isActive = true;
};

/// ECS component that gives an entity fleet command capabilities.
class FleetCommandComponent : public IComponent {
public:
    explicit FleetCommandComponent(const std::string& fleetName = "Fleet");

    const std::string& GetFleetName() const;
    void SetFleetName(const std::string& name);

    int GetMaxMembers() const;
    void SetMaxMembers(int max);

    int GetMemberCount() const;
    int GetActiveMemberCount() const;

    /// Add a ship to the fleet. Returns false if at capacity.
    bool AddMember(uint64_t entityId, const std::string& shipName,
                   FleetRole role = FleetRole::Combat);

    /// Remove a ship from the fleet. Returns false if not found.
    bool RemoveMember(uint64_t entityId);

    /// Get a specific member by entity ID.
    const FleetMember* GetMember(uint64_t entityId) const;

    /// Get all fleet members.
    const std::vector<FleetMember>& GetAllMembers() const;

    /// Issue an order to the fleet. Returns false if order queue is full.
    bool IssueOrder(FleetOrderType type, float targetX = 0.0f,
                    float targetY = 0.0f, float targetZ = 0.0f,
                    uint64_t targetEntityId = 0, int priority = 0);

    /// Cancel an order by ID. Returns false if not found.
    bool CancelOrder(int orderId);

    /// Get a specific order by ID.
    const FleetOrder* GetOrder(int orderId) const;

    /// Get all orders.
    const std::vector<FleetOrder>& GetAllOrders() const;

    /// Get number of active orders.
    int GetActiveOrderCount() const;

    /// Get the fleet's average morale (0 to 1).
    float GetAverageMorale() const;

    /// Set a member's morale. Returns false if member not found.
    bool SetMemberMorale(uint64_t entityId, float morale);

    /// Set a member's role. Returns false if member not found.
    bool SetMemberRole(uint64_t entityId, FleetRole role);

    /// Serialize for save-game persistence.
    ComponentData Serialize() const;

    /// Restore from previously serialized data.
    void Deserialize(const ComponentData& data);

private:
    std::string _fleetName = "Fleet";
    int _maxMembers = 10;
    int _maxOrders = 5;
    std::vector<FleetMember> _members;
    std::vector<FleetOrder> _orders;
    int _nextOrderId = 1;

    friend class FleetCommandSystem;
};

/// System that updates fleet orders and member states each frame.
class FleetCommandSystem : public SystemBase {
public:
    FleetCommandSystem();
    explicit FleetCommandSystem(EntityManager& entityManager);

    void Update(float deltaTime) override;

    void SetEntityManager(EntityManager* em);

private:
    EntityManager* _entityManager = nullptr;
};

} // namespace subspace
