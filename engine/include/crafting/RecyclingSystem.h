#pragma once

#include "inventory/ItemizationSystem.h"

#include <string>
#include <vector>

namespace subspace {

struct RecoveredMaterial {
    std::string materialId;
    float units = 0.0f;
};

struct RecyclingContext {
    int recyclingSkill = 0;   // 0..100
    int facilityTier = 1;     // 1 field, 2 shipboard, 3+ station/industrial
    bool reverseEngineer = false;
};

struct RecyclingResult {
    std::vector<RecoveredMaterial> materials;
    float blueprintResearch = 0.0f;
    float efficiency = 0.0f;
};

class RecyclingSystem {
public:
    static RecyclingResult Recycle(const GeneratedItem& item,const RecyclingContext& context);
};

} // namespace subspace
