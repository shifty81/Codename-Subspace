#pragma once

#include "core/ecs/Entity.h"
#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/ecs/EntityManager.h"
#include "core/persistence/SaveGameManager.h"

#include <string>
#include <vector>
#include <unordered_map>

namespace subspace {

/// Standing levels based on reputation value.
enum class Standing { Hostile, Unfriendly, Neutral, Friendly, Allied };

/// A single reputation change event (for history tracking).
struct ReputationEvent {
    std::string factionId;
    int amount = 0;
    std::string reason;
};

/// Manages reputation values for a single faction relationship.
struct FactionReputation {
    std::string factionId;
    int reputation = 0;         // Range: -1000 to +1000
    int minReputation = -1000;
    int maxReputation = 1000;

    /// Get the current standing level.
    Standing GetStanding() const;

    /// Modify reputation by amount (clamped to min/max).
    void ModifyReputation(int amount);

    /// Get reputation as a normalized float (-1.0 to 1.0).
    float GetNormalizedReputation() const;

    /// Get the display name for a standing level.
    static std::string GetStandingName(Standing standing);

    /// Get the reputation threshold for a standing level.
    static int GetStandingThreshold(Standing standing);
};

/// ECS component that tracks an entity's reputation with multiple factions.
struct ReputationComponent : public IComponent {
    std::vector<FactionReputation> factions;
    std::vector<ReputationEvent> recentEvents; // last N events
    int maxEventHistory = 20;
    float decayRate = 0.0f;     // reputation decay per second toward neutral (0 = no decay)

    /// Get reputation with a specific faction. Returns nullptr if not tracked.
    FactionReputation* GetFaction(const std::string& factionId);
    const FactionReputation* GetFaction(const std::string& factionId) const;

    /// Add or initialize a faction reputation entry.
    FactionReputation& AddFaction(const std::string& factionId, int initialRep = 0);

    /// Modify reputation with a faction (creates entry if needed).
    void ModifyReputation(const std::string& factionId, int amount, const std::string& reason = "");

    /// Get standing with a faction. Returns Neutral if faction not tracked.
    Standing GetStanding(const std::string& factionId) const;

    /// Get all factions with a specific standing.
    std::vector<std::string> GetFactionsWithStanding(Standing standing) const;

    /// Get the number of tracked factions.
    size_t GetFactionCount() const;

    /// Serialize for save-game persistence.
    ComponentData Serialize() const;

    /// Restore from previously serialized data.
    void Deserialize(const ComponentData& data);
};

/// System that processes reputation decay over time.
class ReputationSystem : public SystemBase {
public:
    ReputationSystem();
    explicit ReputationSystem(EntityManager& entityManager);

    void Update(float deltaTime) override;

private:
    EntityManager* _entityManager = nullptr;
};

} // namespace subspace
