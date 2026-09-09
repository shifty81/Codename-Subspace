#pragma once

#include "ships/Block.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

struct BlockDefinition {
    std::string id;
    std::string displayName;
    BlockType blockType = BlockType::Hull;
    std::string description;
    std::unordered_map<std::string, int> resourceCosts;

    float hitPointsPerVolume = 100.0f;
    float massPerUnitVolume = 1.0f;
    bool scalable = true;
    std::string function;

    float powerGenerationPerVolume = 0.0f;
    float powerConsumptionPerVolume = 0.0f;
    float thrustPowerPerVolume = 0.0f;
    float shieldCapacityPerVolume = 0.0f;
    float cargoCapacityPerVolume = 0.0f;
    float crewCapacityPerVolume = 0.0f;
    float batteryCapacityPerVolume = 0.0f;
    float computerSlotsPerVolume = 0.0f;

    int aiPlacementPriority = 0;
    bool requiresInternalPlacement = false;
    bool suitableForExterior = true;
    int minTechLevel = 1;
    std::uint32_t defaultColorRGB = 0x808080;
};

class BlockDefinitionDatabase {
public:
    static const std::unordered_map<BlockType, BlockDefinition>& GetDefinitions();
    static const BlockDefinition& GetDefinition(BlockType blockType);
    static std::vector<BlockType> GetBuildableTypes();

private:
    static void Initialize();
    static bool s_initialized;
    static std::unordered_map<BlockType, BlockDefinition> s_definitions;
};

BlockDefinition MakeBlockDefinition(BlockType type,
                                    std::string id,
                                    std::string displayName,
                                    std::string description,
                                    std::string function,
                                    float hpPerVolume,
                                    float massPerVolume,
                                    std::uint32_t colorRgb);

} // namespace subspace
