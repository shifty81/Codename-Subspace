#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace subspace {
struct AssemblyBaselineIdentity {std::string blueprintId,generatorVersion,factionId,classId,roleId,familyId;std::uint32_t seed=0;};
enum class AssemblyDeltaKind { ModuleAdded,ModuleRemoved,TransformChanged,FitChanged,DamageChanged,CargoChanged,InteriorObjectChanged,PortalStateChanged,CrewChanged };
struct AssemblyStateDelta {std::uint64_t sequence=0;AssemblyDeltaKind kind=AssemblyDeltaKind::TransformChanged;std::string targetId,payload;};
struct AssemblyPersistentState {AssemblyBaselineIdentity baseline;std::vector<AssemblyStateDelta> deltas;};
class AssemblyPersistenceDeltaSystem {
public:
    static bool Append(AssemblyPersistentState& state,AssemblyStateDelta delta);
    static bool Validate(const AssemblyPersistentState& state);
};
} // namespace subspace
