#pragma once

#include "mining/MiningSalvageModel.h"

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

struct RuntimeSaveGameSnapshot {
    std::string saveId = "dev-save";
    std::string sectorId;
    std::uint32_t sectorSeed = 0;
    float playerX = 0.0f;
    float playerY = 0.0f;
    float playerAngle = 0.0f;
    float hull = 100.0f;
    int credits = 0;
    std::vector<CargoYieldItem> cargo;
};

std::string SerializeRuntimeSaveGameSnapshot(const RuntimeSaveGameSnapshot& snapshot);
RuntimeSaveGameSnapshot DeserializeRuntimeSaveGameSnapshot(const std::string& text);
std::string RuntimeSaveGameSummary(const RuntimeSaveGameSnapshot& snapshot);

} // namespace subspace
