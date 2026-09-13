#pragma once
#include <cstdint>
#include <string>

namespace subspace {

enum class PersistentEntityKind : std::uint8_t {
    Unknown,
    World,
    SolarSystem,
    Planet,
    Region,
    Cell,
    Ship,
    Character,
    Settlement,
    Station,
    Vehicle,
    SurveyRecord,
    ModuleInstance
};

struct PersistentEntityId {
    std::uint64_t high = 0;
    std::uint64_t low = 0;

    bool IsValid() const { return high != 0 || low != 0; }
    bool operator==(const PersistentEntityId& other) const { return high == other.high && low == other.low; }
    bool operator!=(const PersistentEntityId& other) const { return !(*this == other); }
};

/// Stable identity is representation-independent. A ship keeps the same ID in
/// surface, atmosphere, orbit, system travel, docking, save/load and future
/// replication layers; render/physics entities are transient projections.
class StableIdentitySystem {
public:
    static PersistentEntityId Deterministic(PersistentEntityKind kind,
                                            const std::string& authority,
                                            const std::string& stableKey,
                                            std::uint64_t generation = 0);
    static std::string ToString(const PersistentEntityId& id);
};

} // namespace subspace
