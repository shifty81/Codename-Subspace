#include "runtime/WorldSimulationAuthority.h"

#include <algorithm>
#include <sstream>

namespace subspace {
namespace {
bool SameId(PersistentEntityId a, PersistentEntityId b) { return a == b; }
}

std::string WorldSimulationAuthority::Key(PersistentEntityId id) {
    return StableIdentitySystem::ToString(id);
}

PersistentEntityId WorldSimulationAuthority::Register(PersistentEntityKind kind,
                                                       const std::string& authority,
                                                       const std::string& stableKey,
                                                       PersistentEntityId parentId,
                                                       SpatialFrameId frameId,
                                                       const std::string& definitionId,
                                                       std::uint64_t generation) {
    const auto id = StableIdentitySystem::Deterministic(kind, authority, stableKey, generation);
    WorldEntityState state;
    state.header.id = id;
    state.header.kind = kind;
    state.header.schemaVersion = SubspacePersistenceSchemaVersion;
    state.parentId = parentId;
    state.frameId = frameId;
    state.definitionId = definitionId;
    if (!RegisterExplicit(state)) return {};
    return id;
}

bool WorldSimulationAuthority::RegisterExplicit(const WorldEntityState& state) {
    if (!state.header.id.IsValid() || state.header.kind == PersistentEntityKind::Unknown) return false;
    if (Find(state.header.id)) return false;
    if (state.parentId.IsValid() && !Find(state.parentId)) return false;
    if (state.parentId == state.header.id) return false;
    if (state.frameId != InvalidSpatialFrameId && !frames_.Find(state.frameId)) return false;

    WorldEntityState normalized = state;
    normalized.header.schemaVersion = SubspacePersistenceSchemaVersion;
    entities_.push_back(std::move(normalized));
    return true;
}

bool WorldSimulationAuthority::Remove(PersistentEntityId id) {
    if (!id.IsValid()) return false;
    for (const auto& entity : entities_) {
        if (entity.parentId == id) return false;
    }
    const auto oldSize = entities_.size();
    entities_.erase(std::remove_if(entities_.begin(), entities_.end(), [&](const auto& e) {
        return e.header.id == id;
    }), entities_.end());
    prefetch_.erase(std::remove_if(prefetch_.begin(), prefetch_.end(), [&](const auto& p) {
        return p.destinationId == id;
    }), prefetch_.end());
    return entities_.size() != oldSize;
}

WorldEntityState* WorldSimulationAuthority::Find(PersistentEntityId id) {
    for (auto& entity : entities_) if (SameId(entity.header.id, id)) return &entity;
    return nullptr;
}

const WorldEntityState* WorldSimulationAuthority::Find(PersistentEntityId id) const {
    for (const auto& entity : entities_) if (SameId(entity.header.id, id)) return &entity;
    return nullptr;
}

std::size_t WorldSimulationAuthority::EntityCount() const { return entities_.size(); }

bool WorldSimulationAuthority::WouldCreateParentCycle(PersistentEntityId id, PersistentEntityId parentId) const {
    if (!parentId.IsValid()) return false;
    PersistentEntityId cursor = parentId;
    std::size_t guard = 0;
    while (cursor.IsValid() && guard++ <= entities_.size()) {
        if (cursor == id) return true;
        const auto* parent = Find(cursor);
        if (!parent) return false;
        cursor = parent->parentId;
    }
    return guard > entities_.size();
}

bool WorldSimulationAuthority::SetParent(PersistentEntityId id, PersistentEntityId parentId) {
    auto* entity = Find(id);
    if (!entity || id == parentId) return false;
    if (parentId.IsValid() && !Find(parentId)) return false;
    if (WouldCreateParentCycle(id, parentId)) return false;
    if (entity->parentId == parentId) return true;
    entity->parentId = parentId;
    MarkDirty(id);
    return true;
}

bool WorldSimulationAuthority::BindFrame(PersistentEntityId id, SpatialFrameId frameId) {
    auto* entity = Find(id);
    if (!entity) return false;
    if (frameId != InvalidSpatialFrameId && !frames_.Find(frameId)) return false;
    if (entity->frameId == frameId) return true;
    entity->frameId = frameId;
    MarkDirty(id);
    return true;
}

bool WorldSimulationAuthority::SetResident(PersistentEntityId id, bool resident) {
    auto* entity = Find(id);
    if (!entity) return false;
    entity->resident = resident;
    if (!resident && entity->representation == SimulationRepresentation::Full) {
        entity->representation = SimulationRepresentation::Proxy;
    }
    return true;
}

bool WorldSimulationAuthority::TransitionRepresentation(PersistentEntityId id, SimulationRepresentation target) {
    auto* entity = Find(id);
    if (!entity) return false;
    if (target == SimulationRepresentation::Full && !entity->resident) return false;
    entity->representation = target;
    return true;
}

std::uint64_t WorldSimulationAuthority::MarkDirty(PersistentEntityId id) {
    auto* entity = Find(id);
    if (!entity) return 0;
    entity->dirty = true;
    return ++entity->header.revision;
}

bool WorldSimulationAuthority::ClearDirty(PersistentEntityId id) {
    auto* entity = Find(id);
    if (!entity) return false;
    entity->dirty = false;
    return true;
}

SpatialKinematicState WorldSimulationAuthority::ReparentKinematics(const SpatialKinematicState& state,
                                                                    SpatialFrameId newParent) const {
    return frames_.ReparentPreservingWorld(state, newParent);
}

WorldPrefetchRequest WorldSimulationAuthority::RequestPrefetch(PersistentEntityId destinationId,
                                                                SimulationRepresentation minimumRepresentation,
                                                                int priority,
                                                                std::string reason) {
    WorldPrefetchRequest request;
    if (!Find(destinationId)) return request;
    request.ticket = nextPrefetchTicket_++;
    request.destinationId = destinationId;
    request.minimumRepresentation = minimumRepresentation;
    request.priority = priority;
    request.reason = std::move(reason);
    prefetch_.push_back(request);
    return request;
}

std::optional<WorldPrefetchRequest> WorldSimulationAuthority::PopNextPrefetch() {
    if (prefetch_.empty()) return std::nullopt;
    const auto best = std::max_element(prefetch_.begin(), prefetch_.end(), [](const auto& a, const auto& b) {
        if (a.priority != b.priority) return a.priority < b.priority;
        return a.ticket > b.ticket;
    });
    const auto result = *best;
    prefetch_.erase(best);
    return result;
}

void WorldSimulationAuthority::AddUnresolvedReference(PersistentEntityId ownerId,
                                                       PersistentEntityId targetId,
                                                       std::string reason) {
    std::ostringstream out;
    out << Key(ownerId) << " -> " << Key(targetId);
    if (!reason.empty()) out << " : " << reason;
    const auto record = out.str();
    if (std::find(unresolvedReferences_.begin(), unresolvedReferences_.end(), record) == unresolvedReferences_.end()) {
        unresolvedReferences_.push_back(record);
    }
}

PersistenceCheckpoint WorldSimulationAuthority::CreateCheckpoint() const {
    PersistenceCheckpoint checkpoint;
    checkpoint.sequence = ++checkpointSequence_;
    checkpoint.schemaVersion = SubspacePersistenceSchemaVersion;
    std::vector<const WorldEntityState*> ordered;
    ordered.reserve(entities_.size());
    for (const auto& entity : entities_) ordered.push_back(&entity);
    std::sort(ordered.begin(), ordered.end(), [](const auto* a, const auto* b) {
        return Key(a->header.id) < Key(b->header.id);
    });
    for (const auto* entity : ordered) checkpoint.records.push_back(entity->header);
    checkpoint.unresolvedReferences = unresolvedReferences_;
    std::sort(checkpoint.unresolvedReferences.begin(), checkpoint.unresolvedReferences.end());
    return checkpoint;
}

PersistenceMigrationPlan WorldSimulationAuthority::PlanMigration(std::uint32_t fromVersion) const {
    PersistenceMigrationPlan plan;
    plan.fromVersion = fromVersion;
    plan.toVersion = SubspacePersistenceSchemaVersion;
    if (fromVersion == SubspacePersistenceSchemaVersion) {
        plan.supported = true;
        return plan;
    }
    if (fromVersion == 1 && SubspacePersistenceSchemaVersion == 2) {
        plan.supported = true;
        plan.steps = {
            "assign-stable-persistent-identities",
            "bind-records-to-spatial-frames",
            "materialize-cell-and-representation-state",
            "record-generation-recipe-version"
        };
    }
    return plan;
}

std::string WorldSimulationAuthority::Diagnostics() const {
    std::size_t resident = 0, dirty = 0, full = 0;
    for (const auto& entity : entities_) {
        if (entity.resident) ++resident;
        if (entity.dirty) ++dirty;
        if (entity.representation == SimulationRepresentation::Full) ++full;
    }
    std::ostringstream out;
    out << "world-authority-v1; entities=" << entities_.size()
        << "; resident=" << resident
        << "; full=" << full
        << "; dirty=" << dirty
        << "; prefetch=" << prefetch_.size()
        << "; unresolved=" << unresolvedReferences_.size()
        << "; persistence=v" << SubspacePersistenceSchemaVersion;
    return out.str();
}

} // namespace subspace
