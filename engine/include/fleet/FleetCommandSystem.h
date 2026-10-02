#pragma once

#include "core/ecs/Entity.h"
#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/ecs/EntityManager.h"
#include "core/persistence/SaveGameManager.h"

#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace subspace {

/// Type of order that can be issued to a fleet. Historical values are kept in
/// place; strategy-game orders are append-only for save compatibility.
enum class FleetOrderType {
    Idle,
    Patrol,
    Mine,
    Trade,
    Attack,
    Escort,
    Defend,
    Scout,
    Move,
    Approach,
    Orbit,
    Hold,
    Salvage,
    Dock,
    Warp,
    Repair,
    Resupply
};

enum class FleetOrderState {
    Pending,
    Active,
    Paused,
    Completed,
    Failed,
    Blocked
};

enum class FleetRole {
    Flagship,
    Combat,
    Mining,
    Trading,
    Support,
    Scout
};

struct FleetOrderRequest {
    FleetOrderType type = FleetOrderType::Idle;
    std::uint64_t targetEntityId = 0;
    float targetX = 0.0f, targetY = 0.0f, targetZ = 0.0f;
    int priority = 0;
    bool queue = false;
    float acceptanceRadius = 5.0f;
    float orbitRadius = 0.0f;
    std::vector<std::uint64_t> assignedMembers;
};

struct FleetOrder {
    int orderId = 0;
    FleetOrderType type = FleetOrderType::Idle;
    FleetOrderState state = FleetOrderState::Pending;
    std::uint64_t targetEntityId = 0;
    float targetX = 0.0f, targetY = 0.0f, targetZ = 0.0f;
    int priority = 0;
    float progress = 0.0f;

    // R178 strategy authority fields. They are append-only in serialized data.
    std::uint64_t sequence = 0;
    float acceptanceRadius = 5.0f;
    float orbitRadius = 0.0f;
    std::vector<std::uint64_t> assignedMembers;
    std::string status;

    static std::string GetOrderTypeName(FleetOrderType type);
    static std::string GetOrderStateName(FleetOrderState state);
    static std::string GetRoleName(FleetRole role);
};

struct FleetMember {
    std::uint64_t entityId = 0;
    std::string shipName;
    FleetRole role = FleetRole::Combat;
    float morale = 1.0f;
    bool isActive = true;
};

class FleetCommandComponent : public IComponent {
public:
    explicit FleetCommandComponent(const std::string& fleetName = "Fleet");

    const std::string& GetFleetName() const;
    void SetFleetName(const std::string& name);
    int GetMaxMembers() const;
    void SetMaxMembers(int max);
    int GetMaxOrders() const { return _maxOrders; }
    void SetMaxOrders(int max);
    int GetMemberCount() const;
    int GetActiveMemberCount() const;

    bool AddMember(std::uint64_t entityId, const std::string& shipName,
                   FleetRole role = FleetRole::Combat);
    bool RemoveMember(std::uint64_t entityId);
    const FleetMember* GetMember(std::uint64_t entityId) const;
    const std::vector<FleetMember>& GetAllMembers() const;

    /// Historical API preserved as a non-queued request.
    bool IssueOrder(FleetOrderType type, float targetX = 0.0f,
                    float targetY = 0.0f, float targetZ = 0.0f,
                    std::uint64_t targetEntityId = 0, int priority = 0);
    bool IssueOrder(const FleetOrderRequest& request);

    bool CancelOrder(int orderId);
    bool PauseOrder(int orderId);
    bool ResumeOrder(int orderId);
    bool CompleteOrder(int orderId, const std::string& status = "COMPLETED");
    bool FailOrder(int orderId, const std::string& status);
    bool BlockOrder(int orderId, const std::string& status);

    const FleetOrder* GetOrder(int orderId) const;
    FleetOrder* GetMutableOrder(int orderId);
    const std::vector<FleetOrder>& GetAllOrders() const;
    int GetActiveOrderCount() const;
    int GetPendingOrderCount() const;

    float GetAverageMorale() const;
    bool SetMemberMorale(std::uint64_t entityId, float morale);
    bool SetMemberRole(std::uint64_t entityId, FleetRole role);

    ComponentData Serialize() const;
    void Deserialize(const ComponentData& data);

private:
    std::string _fleetName = "Fleet";
    int _maxMembers = 10;
    int _maxOrders = 5;
    std::vector<FleetMember> _members;
    std::vector<FleetOrder> _orders;
    int _nextOrderId = 1;
    std::uint64_t _nextSequence = 1;

    friend class FleetCommandSystem;
};

enum class FleetOrderExecutionDisposition { Running, Completed, Failed, Blocked };
struct FleetOrderExecutionResult {
    FleetOrderExecutionDisposition disposition = FleetOrderExecutionDisposition::Running;
    float progress = -1.0f; // <0 = executor did not update progress
    std::string status;
};
using FleetOrderExecutor = std::function<FleetOrderExecutionResult(FleetCommandComponent&, FleetOrder&, float)>;

/// Fleet order scheduler. It no longer simulates success with an arbitrary
/// timer. Domain systems register executors; orders without an executor remain
/// blocked/pending with an explicit status instead of falsely completing.
class FleetCommandSystem : public SystemBase {
public:
    FleetCommandSystem();
    explicit FleetCommandSystem(EntityManager& entityManager);

    void Update(float deltaTime) override;
    void SetEntityManager(EntityManager* em);

    void RegisterExecutor(FleetOrderType type, FleetOrderExecutor executor);
    void ClearExecutor(FleetOrderType type);
    bool HasExecutor(FleetOrderType type) const;

private:
    static FleetOrder* SelectNextPending(FleetCommandComponent& fleet);
    EntityManager* _entityManager = nullptr;
    std::map<FleetOrderType, FleetOrderExecutor> _executors;
};

} // namespace subspace
