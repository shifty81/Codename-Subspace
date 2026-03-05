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
#include <utility>

namespace subspace {

/// Classification of a wormhole connection.
enum class WormholeType { Natural, Artificial, Unstable, Persistent };

/// Current lifecycle state of a wormhole link.
enum class WormholeState { Dormant, Activating, Active, Destabilizing, Collapsed };

/// One end of a wormhole, anchored in a specific sector and position.
struct WormholeEndpoint {
    int sectorX = 0;
    int sectorY = 0;
    float posX = 0.0f;
    float posY = 0.0f;
    float posZ = 0.0f;
    std::string name;
};

/// A connection between two wormhole endpoints.
struct WormholeLink {
    int linkId = 0;
    WormholeEndpoint endpointA;
    WormholeEndpoint endpointB;
    WormholeType type = WormholeType::Natural;
    WormholeState state = WormholeState::Dormant;
    float stability = 1.0f;       // 0 – 1
    float maxMass = 10000.0f;
    float currentMass = 0.0f;
    float traversalTime = 5.0f;   // seconds
    bool bidirectional = true;

    /// Get the display name for a wormhole type.
    static std::string GetTypeName(WormholeType type);

    /// Get the display name for a wormhole state.
    static std::string GetStateName(WormholeState state);
};

/// ECS component that gives an entity wormhole-network capabilities.
class WormholeComponent : public IComponent {
public:
    explicit WormholeComponent(int maxLinks = 8);

    /// Add a link (up to maxLinks).
    void AddLink(const WormholeLink& link);

    const WormholeLink* GetLink(int linkId) const;
    WormholeLink* GetLink(int linkId);

    /// Get all links whose state is Active.
    std::vector<const WormholeLink*> GetActiveLinks() const;

    /// Get all links that have an endpoint in the given sector.
    std::vector<const WormholeLink*> GetLinksToSector(int sectorX, int sectorY) const;

    /// Find a link connecting two specific sectors (either direction).
    const WormholeLink* FindLink(int fromSectorX, int fromSectorY,
                                 int toSectorX, int toSectorY) const;

    /// Request traversal through a wormhole link.
    /// Succeeds only when the link is Active, stability > 0.1, and mass fits.
    bool RequestTraversal(int linkId, EntityId shipId, float shipMass);

    /// Complete a traversal, reducing the link's current mass usage.
    bool CompleteTraversal(int linkId, EntityId shipId, float shipMass);

    float GetStability(int linkId) const;
    int GetLinkCount() const;
    int GetMaxLinks() const;
    const std::vector<WormholeLink>& GetAllLinks() const;

    /// Serialize for save-game persistence.
    ComponentData Serialize() const;

    /// Restore from previously serialized data.
    void Deserialize(const ComponentData& data);

private:
    std::vector<WormholeLink> _links;
    int _maxLinks = 8;
    float _stabilityDecayRate = 0.01f;
    float _stabilityRegenRate = 0.005f;
    std::vector<std::pair<int, EntityId>> _activeTraversals;

    friend class WormholeSystem;
};

/// System that advances wormhole state machines and stability each frame.
class WormholeSystem : public SystemBase {
public:
    WormholeSystem();
    explicit WormholeSystem(EntityManager& entityManager);

    void Update(float deltaTime) override;

    void SetEntityManager(EntityManager* em);

private:
    EntityManager* _entityManager = nullptr;
};

} // namespace subspace
