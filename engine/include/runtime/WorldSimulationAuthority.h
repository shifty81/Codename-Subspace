#pragma once

#include "runtime/StableIdentitySystem.h"
#include "runtime/WorldPersistenceContract.h"
#include "world/SpatialFrameSystem.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace subspace {

enum class SimulationRepresentation : std::uint8_t {
    Unloaded = 0,
    Strategic = 1,
    Proxy = 2,
    Full = 3
};

struct WorldEntityState {
    PersistentRecordHeader header{};
    PersistentEntityId parentId{};
    SpatialFrameId frameId = InvalidSpatialFrameId;
    SimulationRepresentation representation = SimulationRepresentation::Unloaded;
    bool resident = false;
    bool dirty = false;
    std::string definitionId;
};

struct WorldPrefetchRequest {
    std::uint64_t ticket = 0;
    PersistentEntityId destinationId{};
    SimulationRepresentation minimumRepresentation = SimulationRepresentation::Proxy;
    int priority = 0;
    std::string reason;
};

struct PersistenceCheckpoint {
    std::uint64_t sequence = 0;
    std::uint32_t schemaVersion = SubspacePersistenceSchemaVersion;
    std::vector<PersistentRecordHeader> records;
    std::vector<std::string> unresolvedReferences;
};

struct PersistenceMigrationPlan {
    std::uint32_t fromVersion = 0;
    std::uint32_t toVersion = SubspacePersistenceSchemaVersion;
    bool supported = false;
    std::vector<std::string> steps;
};

/// Canonical runtime/world composition authority introduced in Pass973-986.
///
/// This class deliberately owns representation-independent world identity,
/// spatial-frame bindings, residency/LOD state, destination prefetch requests,
/// dirty revisions and persistence checkpoints in one place. Renderer, physics,
/// editor and networking layers project from this state instead of inventing
/// parallel entity ownership.
class WorldSimulationAuthority {
public:
    PersistentEntityId Register(PersistentEntityKind kind,
                                const std::string& authority,
                                const std::string& stableKey,
                                PersistentEntityId parentId = {},
                                SpatialFrameId frameId = InvalidSpatialFrameId,
                                const std::string& definitionId = {},
                                std::uint64_t generation = 0);

    bool RegisterExplicit(const WorldEntityState& state);
    bool Remove(PersistentEntityId id);

    WorldEntityState* Find(PersistentEntityId id);
    const WorldEntityState* Find(PersistentEntityId id) const;
    std::size_t EntityCount() const;

    bool SetParent(PersistentEntityId id, PersistentEntityId parentId);
    bool BindFrame(PersistentEntityId id, SpatialFrameId frameId);
    bool SetResident(PersistentEntityId id, bool resident);
    bool TransitionRepresentation(PersistentEntityId id, SimulationRepresentation target);
    std::uint64_t MarkDirty(PersistentEntityId id);
    bool ClearDirty(PersistentEntityId id);

    SpatialFrameSystem& Frames() { return frames_; }
    const SpatialFrameSystem& Frames() const { return frames_; }
    SpatialKinematicState ReparentKinematics(const SpatialKinematicState& state,
                                             SpatialFrameId newParent) const;

    WorldPrefetchRequest RequestPrefetch(PersistentEntityId destinationId,
                                         SimulationRepresentation minimumRepresentation,
                                         int priority,
                                         std::string reason);
    std::optional<WorldPrefetchRequest> PopNextPrefetch();
    std::size_t PendingPrefetchCount() const { return prefetch_.size(); }

    void AddUnresolvedReference(PersistentEntityId ownerId,
                                PersistentEntityId targetId,
                                std::string reason);
    bool HasUnresolvedReferences() const { return !unresolvedReferences_.empty(); }

    PersistenceCheckpoint CreateCheckpoint() const;
    PersistenceMigrationPlan PlanMigration(std::uint32_t fromVersion) const;
    std::string Diagnostics() const;

private:
    static std::string Key(PersistentEntityId id);
    bool WouldCreateParentCycle(PersistentEntityId id, PersistentEntityId parentId) const;

    std::vector<WorldEntityState> entities_;
    SpatialFrameSystem frames_;
    std::vector<WorldPrefetchRequest> prefetch_;
    std::vector<std::string> unresolvedReferences_;
    std::uint64_t nextPrefetchTicket_ = 1;
    mutable std::uint64_t checkpointSequence_ = 0;
};

} // namespace subspace
