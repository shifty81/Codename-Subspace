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

/// State of a ship's docking sequence.
enum class DockingState {
    Undocked,
    RequestingDock,
    Approaching,
    Docking,
    Docked,
    Undocking,
    Launching
};

/// Size classification for docking bays and ships.
enum class BaySize { Small, Medium, Large, Capital };

/// A single docking bay within a hangar.
struct DockingBay {
    int bayId = 0;
    std::string bayName;
    BaySize size = BaySize::Medium;
    bool isOccupied = false;
    EntityId dockedShipId = 0;
    float repairRate = 5.0f;   // hull points per second for docked ships
    float refuelRate = 10.0f;  // fuel units per second
    bool isEnabled = true;

    /// Get the display name for a bay size.
    static std::string GetSizeName(BaySize size);

    /// Get the display name for a docking state.
    static std::string GetStateName(DockingState state);
};

/// A pending or active docking request.
struct DockingRequest {
    EntityId shipId = 0;
    int assignedBayId = -1;
    float requestTime = 0.0f;
    DockingState state = DockingState::RequestingDock;
    float approachProgress = 0.0f;  // 0 to 1 progress of approach
    float dockingProgress = 0.0f;   // 0 to 1 progress of docking sequence
};

/// ECS component that gives an entity (station/carrier) hangar capabilities.
class HangarComponent : public IComponent {
public:
    explicit HangarComponent(int maxBays = 4);

    int GetMaxBays() const;
    int GetOccupiedBayCount() const;
    int GetFreeBayCount() const;

    /// Add a bay (up to maxBays).
    void AddBay(const DockingBay& bay);

    const DockingBay* GetBay(int bayId) const;
    DockingBay* GetBay(int bayId);

    /// Get all unoccupied, enabled bays.
    std::vector<const DockingBay*> GetFreeBays() const;

    /// Get free bays that are at least the given minimum size.
    std::vector<const DockingBay*> GetFreeBaysBySize(BaySize minSize) const;

    /// Create a docking request and assign to the smallest available bay
    /// that fits. Returns false if no bay is available.
    bool RequestDocking(EntityId shipId, BaySize requiredSize = BaySize::Small);

    /// Cancel a docking request or undock a ship.
    bool CancelDocking(EntityId shipId);

    bool IsShipDocked(EntityId shipId) const;

    /// Returns Undocked if the ship is not found.
    DockingState GetShipDockingState(EntityId shipId) const;

    const DockingRequest* GetDockingRequest(EntityId shipId) const;

    /// Start the undocking/launch sequence for a docked ship.
    bool RequestLaunch(EntityId shipId);

    /// Put a ship into long-term storage.
    void StoreShip(EntityId shipId);

    /// Retrieve a ship from storage. Returns false if not found.
    bool RetrieveShip(EntityId shipId);

    const std::vector<EntityId>& GetStoredShips() const;
    int GetStoredShipCount() const;

    const std::vector<DockingBay>& GetAllBays() const;
    const std::vector<DockingRequest>& GetActiveRequests() const;

    /// Serialize for save-game persistence.
    ComponentData Serialize() const;

    /// Restore from previously serialized data.
    void Deserialize(const ComponentData& data);

private:
    std::vector<DockingBay> _bays;
    std::vector<DockingRequest> _activeRequests;
    int _maxBays = 4;
    float _approachSpeed = 2.0f;   // seconds for approach sequence
    float _dockingSpeed = 3.0f;    // seconds for docking sequence
    float _launchSpeed = 2.0f;     // seconds for launch sequence
    std::vector<EntityId> _storedShips;  // ships in long-term storage

    friend class HangarSystem;
};

/// System that processes docking sequences each frame.
class HangarSystem : public SystemBase {
public:
    HangarSystem();
    explicit HangarSystem(EntityManager& entityManager);

    void Update(float deltaTime) override;

    void SetEntityManager(EntityManager* em);

private:
    EntityManager* _entityManager = nullptr;
};

} // namespace subspace
