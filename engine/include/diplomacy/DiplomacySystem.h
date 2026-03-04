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

/// The type of diplomatic relationship between two factions.
enum class DiplomaticStatus { War, Hostile, Neutral, NonAggression, Trade, Alliance };

/// Types of treaties that can be proposed or active.
enum class TreatyType { NonAggression, TradeAgreement, DefensivePact, Alliance, Ceasefire };

/// A treaty between two factions.
struct Treaty {
    std::string treatyId;
    TreatyType type = TreatyType::NonAggression;
    std::string factionA;
    std::string factionB;
    float duration = -1.0f;     // remaining seconds, -1 = indefinite
    float totalDuration = -1.0f;
    bool isActive = true;

    /// Get the display name for a treaty type.
    static std::string GetTreatyName(TreatyType type);

    /// Get progress (0-100) for timed treaties. Returns 100 if indefinite.
    float GetProgress() const;
};

/// A diplomatic relationship between two factions.
struct DiplomaticRelation {
    std::string factionA;
    std::string factionB;
    DiplomaticStatus status = DiplomaticStatus::Neutral;
    int trust = 0;              // -100 to +100
    std::vector<std::string> activeTreatyIds;

    /// Get the display name for a diplomatic status.
    static std::string GetStatusName(DiplomaticStatus status);

    /// Get trust clamped to valid range.
    void ModifyTrust(int amount);
};

/// Manages all treaties in the game world.
class DiplomacyDatabase {
public:
    /// Add a treaty. Returns the assigned treatyId.
    std::string AddTreaty(Treaty treaty);

    /// Find a treaty by ID. Returns nullptr if not found.
    const Treaty* FindTreaty(const std::string& treatyId) const;
    Treaty* FindTreaty(const std::string& treatyId);

    /// Get all treaties involving a faction.
    std::vector<const Treaty*> GetTreatiesForFaction(const std::string& factionId) const;

    /// Get all active treaties.
    std::vector<const Treaty*> GetActiveTreaties() const;

    /// Remove a treaty by ID. Returns true if removed.
    bool RemoveTreaty(const std::string& treatyId);

    /// Get the number of treaties.
    size_t GetTreatyCount() const;

    /// Create a default set of treaties for testing.
    static DiplomacyDatabase CreateDefaultDatabase();

private:
    std::vector<Treaty> _treaties;
    std::unordered_map<std::string, size_t> _treatyIndex;
    int _nextId = 1;
};

/// ECS component that gives an entity (faction) diplomatic capabilities.
struct DiplomacyComponent : public IComponent {
    std::string factionId;
    std::vector<DiplomaticRelation> relations;
    float warWeariness = 0.0f;      // 0-100, increases while at war
    float warWearinessRate = 1.0f;  // per second
    float trustGainRate = 0.1f;     // per second for allied/trade factions

    /// Get relation with a specific faction. Returns nullptr if not tracked.
    DiplomaticRelation* GetRelation(const std::string& otherFaction);
    const DiplomaticRelation* GetRelation(const std::string& otherFaction) const;

    /// Add or initialize a diplomatic relation.
    DiplomaticRelation& AddRelation(const std::string& otherFaction,
                                    DiplomaticStatus status = DiplomaticStatus::Neutral);

    /// Declare war on another faction.
    void DeclareWar(const std::string& otherFaction);

    /// Propose peace (sets status to Neutral).
    void ProposePeace(const std::string& otherFaction);

    /// Set diplomatic status with another faction.
    void SetStatus(const std::string& otherFaction, DiplomaticStatus status);

    /// Get the diplomatic status with a faction. Returns Neutral if not tracked.
    DiplomaticStatus GetStatus(const std::string& otherFaction) const;

    /// Get all factions with a specific status.
    std::vector<std::string> GetFactionsWithStatus(DiplomaticStatus status) const;

    /// Get the number of tracked relations.
    size_t GetRelationCount() const;

    /// Check if at war with any faction.
    bool IsAtWar() const;

    /// Get the number of active wars.
    int GetWarCount() const;

    /// Serialize for save-game persistence.
    ComponentData Serialize() const;

    /// Restore from previously serialized data.
    void Deserialize(const ComponentData& data);
};

/// System that updates diplomatic state each frame.
class DiplomacySystem : public SystemBase {
public:
    DiplomacySystem();
    explicit DiplomacySystem(EntityManager& entityManager);

    void Update(float deltaTime) override;

private:
    EntityManager* _entityManager = nullptr;
};

} // namespace subspace
