#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace subspace {

struct TacticalTargetReference {
    int kind = 0;
    std::size_t index = 0;
    std::string id;
    bool IsValid() const { return kind != 0 && !id.empty(); }
};

struct TacticalTargetSlot {
    TacticalTargetReference contact{};
    float lockProgress = 0.0f;
    bool locked = false;
    bool active = false;
};

struct TacticalTargetingState {
    std::vector<TacticalTargetSlot> targets;
    std::size_t maxTargets = 5;
};

class TacticalTargetingSystem {
public:
    static bool Request(TacticalTargetingState& state,const TacticalTargetReference& selection);
    static void Tick(TacticalTargetingState& state,float deltaSeconds,float acquisitionSeconds=1.25f);
    static bool IsLocked(const TacticalTargetingState& state,const TacticalTargetReference& selection);
    static bool Activate(TacticalTargetingState& state,const TacticalTargetReference& selection);
    static void ClearInvalid(TacticalTargetingState& state,const std::vector<std::string>& validIds);
};

} // namespace subspace
