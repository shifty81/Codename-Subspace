#pragma once

#include "core/Math.h"
#include "core/ecs/Entity.h"
#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/ecs/EntityManager.h"
#include "core/persistence/SaveGameManager.h"

#include <string>
#include <vector>
#include <cstdint>

namespace subspace {

/// Formation pattern types.
enum class FormationType { Line, V, Diamond, Circle, Wedge, Column };

/// A slot in a formation with a relative offset from the leader.
struct FormationSlot {
    int slotIndex = 0;
    Vector3 offset{0.0f, 0.0f, 0.0f};
    EntityId assignedEntity = InvalidEntityId;
};

/// Defines a formation pattern and computes positions.
class FormationPattern {
public:
    FormationType type = FormationType::Line;
    float spacing = 10.0f;  // distance between units

    /// Compute slot offsets for a given number of units.
    std::vector<FormationSlot> ComputeSlots(int unitCount) const;

    /// Get the display name for a formation type.
    static std::string GetFormationName(FormationType type);

    /// Get the max recommended size for a formation type.
    static int GetMaxRecommendedSize(FormationType type);

private:
    std::vector<FormationSlot> ComputeLineSlots(int unitCount) const;
    std::vector<FormationSlot> ComputeVSlots(int unitCount) const;
    std::vector<FormationSlot> ComputeDiamondSlots(int unitCount) const;
    std::vector<FormationSlot> ComputeCircleSlots(int unitCount) const;
    std::vector<FormationSlot> ComputeWedgeSlots(int unitCount) const;
    std::vector<FormationSlot> ComputeColumnSlots(int unitCount) const;
};

/// ECS component that marks an entity as part of a formation.
struct FormationComponent : public IComponent {
    FormationType formationType = FormationType::Line;
    float spacing = 10.0f;
    EntityId leaderId = InvalidEntityId;
    std::vector<EntityId> members;
    int slotIndex = -1;          // this entity's slot in the formation (-1 = leader)
    bool isLeader = false;
    Vector3 targetOffset{0.0f, 0.0f, 0.0f}; // offset from leader to maintain

    /// Add a member to the formation.
    void AddMember(EntityId id);

    /// Remove a member from the formation.
    bool RemoveMember(EntityId id);

    /// Get the number of members (including leader).
    int GetMemberCount() const;

    /// Check if an entity is in the formation.
    bool HasMember(EntityId id) const;

    /// Reassign slots based on current member list.
    void ReassignSlots(const FormationPattern& pattern);

    /// Serialize for save-game persistence.
    ComponentData Serialize() const;

    /// Restore from previously serialized data.
    void Deserialize(const ComponentData& data);
};

/// System that updates formation positions each frame.
class FormationSystem : public SystemBase {
public:
    FormationSystem();
    explicit FormationSystem(EntityManager& entityManager);

    void Update(float deltaTime) override;

private:
    EntityManager* _entityManager = nullptr;
};

} // namespace subspace
