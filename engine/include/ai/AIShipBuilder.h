#pragma once

#include "factions/FactionProfile.h"
#include "ships/Ship.h"

namespace subspace {

enum class NPCTier { Scout, Frigate, Cruiser, Battleship };

struct TierSettings {
    int minBlocks;
    int maxBlocks;
    int weaponSlots;
    float armorThickness;
};

class AIShipBuilder {
public:
    AIShipBuilder(const FactionProfile& faction, NPCTier tier, int seed);

    Ship Build();

    static TierSettings GetTierSettings(NPCTier tier);

private:
    void BuildSpine(Ship& ship);
    void ExpandHull(Ship& ship);
    void AddArmor(Ship& ship);
    void AddEngines(Ship& ship);
    void AddWeapons(Ship& ship);
    void ApplyMaterials(Ship& ship);

    void PlaceBlock(Ship& ship, const Vector3Int& pos, const Vector3Int& size,
                    BlockType type, BlockShape shape);

    int NextRandom();
    float NextRandomFloat();
    BlockShape PickShape();

    FactionProfile m_faction;
    NPCTier m_tier;
    int m_seed;
    int m_rngState;
};

} // namespace subspace
