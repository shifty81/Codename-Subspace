#include "runtime/AssemblyPersistenceDeltaSystem.h"
#include <utility>
namespace subspace {
bool AssemblyPersistenceDeltaSystem::Append(AssemblyPersistentState&s,AssemblyStateDelta d){
    if(d.targetId.empty())return false;if(!s.deltas.empty()&&d.sequence<=s.deltas.back().sequence)return false;s.deltas.push_back(std::move(d));return true;
}
bool AssemblyPersistenceDeltaSystem::Validate(const AssemblyPersistentState&s){
    if(s.baseline.blueprintId.empty()||s.baseline.generatorVersion.empty()||s.baseline.classId.empty())return false;
    std::uint64_t last=0;for(const auto&d:s.deltas){if(d.targetId.empty()||(last&&d.sequence<=last))return false;last=d.sequence;}return true;
}
} // namespace subspace
