#include "ai/AIShipBuilder.h"
#include "ships/BlockPlacement.h"
#include "ships/ShipStats.h"

#include <algorithm>
#include <cmath>

namespace subspace {

// ---- Construction ----------------------------------------------------------

AIShipBuilder::AIShipBuilder(const FactionProfile& faction, NPCTier tier, int seed)
    : m_faction(faction), m_tier(tier), m_seed(seed), m_rngState(seed) {}

// ---- Seeded LCG ------------------------------------------------------------

int AIShipBuilder::NextRandom() {
    // Numerical Recipes LCG
    m_rngState = m_rngState * 1664525 + 1013904223;
    return (m_rngState >> 16) & 0x7FFF;
}

float AIShipBuilder::NextRandomFloat() {
    return static_cast<float>(NextRandom()) / 32767.0f;
}

// ---- Tier settings ---------------------------------------------------------

TierSettings AIShipBuilder::GetTierSettings(NPCTier tier) {
    switch (tier) {
        case NPCTier::Scout:      return {50,  80,   1, 0.5f};
        case NPCTier::Frigate:    return {120, 180,  3, 1.0f};
        case NPCTier::Cruiser:    return {300, 500,  6, 1.5f};
        case NPCTier::Battleship: return {900, 1400, 10, 2.0f};
    }
    return {50, 80, 1, 0.5f};
}

// ---- Block placement helper ------------------------------------------------

void AIShipBuilder::PlaceBlock(Ship& ship, const Vector3Int& pos,
                               const Vector3Int& size, BlockType type,
                               BlockShape shape) {
    auto block = std::make_shared<Block>();
    block->gridPos       = pos;
    block->size          = size;
    block->rotationIndex = 0;
    block->shape         = shape;
    block->type          = type;
    block->material      = m_faction.palette.hull;
    block->maxHP         = GetBlockBaseHP(type) * MaterialDatabase::Get(block->material).hpMultiplier;
    block->currentHP     = block->maxHP;

    BlockPlacement::Place(ship, std::move(block));
}

// ---- Spine length from silhouette ------------------------------------------

static int SpineLength(LengthBias bias) {
    switch (bias) {
        case LengthBias::Short:    return 6;
        case LengthBias::Balanced: return 10;
        case LengthBias::Long:     return 16;
    }
    return 10;
}

static int HullRadius(ThicknessBias bias) {
    switch (bias) {
        case ThicknessBias::Thin:   return 1;
        case ThicknessBias::Medium: return 2;
        case ThicknessBias::Chunky: return 3;
    }
    return 2;
}

// ---- Choose a random shape from the faction's allowed list -----------------

BlockShape AIShipBuilder::PickShape() {
    const auto& shapes = m_faction.shapeLanguage.allowedShapes;
    if (shapes.empty()) return BlockShape::Cube;
    return shapes[static_cast<size_t>(NextRandom()) % shapes.size()];
}

// ---- Build pipeline --------------------------------------------------------

Ship AIShipBuilder::Build() {
    Ship ship;
    ship.seed    = m_seed;
    ship.faction = m_faction.id;
    ship.name    = m_faction.displayName + " Ship";
    m_rngState   = m_seed;

    BuildSpine(ship);
    ExpandHull(ship);
    AddArmor(ship);
    AddEngines(ship);
    AddWeapons(ship);
    ApplyMaterials(ship);

    ShipStats::Recalculate(ship);
    return ship;
}

// ---- BuildSpine: line of hull blocks along +Z ------------------------------

void AIShipBuilder::BuildSpine(Ship& ship) {
    int length = SpineLength(m_faction.silhouette.length);
    for (int z = 0; z < length; ++z) {
        PlaceBlock(ship, {0, 0, z}, Vector3Int::One(), BlockType::Hull, BlockShape::Cube);
    }
}

// ---- ExpandHull: grow outward from the spine -------------------------------

void AIShipBuilder::ExpandHull(Ship& ship) {
    int length = SpineLength(m_faction.silhouette.length);
    int radius = HullRadius(m_faction.silhouette.thickness);
    TierSettings ts = GetTierSettings(m_tier);
    int targetBlocks = ts.minBlocks + NextRandom() % (ts.maxBlocks - ts.minBlocks + 1);

    for (int z = 0; z < length && static_cast<int>(ship.BlockCount()) < targetBlocks; ++z) {
        for (int x = -radius; x <= radius; ++x) {
            for (int y = -radius; y <= radius; ++y) {
                if (x == 0 && y == 0) continue; // spine already placed

                if (static_cast<int>(ship.BlockCount()) >= targetBlocks) return;

                // VoidBias: skip certain cells
                if (m_faction.silhouette.voidType == VoidBias::Channelled) {
                    // Skip alternating channels along the length
                    if ((z % 3 == 1) && (x == 0 || y == 0)) continue;
                }
                if (m_faction.silhouette.voidType == VoidBias::Ringed) {
                    // Skip the interior, keep outer ring
                    if (std::abs(x) < radius && std::abs(y) < radius) continue;
                }

                // MassBias shaping
                bool place = true;
                int dist = std::abs(x) + std::abs(y);
                switch (m_faction.silhouette.mass) {
                    case MassBias::Central:
                        // Taper at extremes
                        if (dist > radius && z > length / 2) place = false;
                        break;
                    case MassBias::Spine:
                        // Keep blocks close to the spine
                        if (dist > 1) place = NextRandomFloat() < 0.3f;
                        break;
                    case MassBias::Winged:
                        // Prefer blocks on X axis (wings)
                        if (std::abs(y) > 1 && std::abs(x) < std::abs(y))
                            place = NextRandomFloat() < 0.3f;
                        break;
                    case MassBias::Distributed:
                        // Slight random thinning
                        if (dist > 1) place = NextRandomFloat() < 0.7f;
                        break;
                }

                if (place) {
                    BlockShape shape = PickShape();
                    PlaceBlock(ship, {x, y, z}, Vector3Int::One(),
                               BlockType::Hull, shape);
                }
            }
        }
    }
}

// ---- AddArmor: shell of armor on exposed faces -----------------------------

void AIShipBuilder::AddArmor(Ship& ship) {
    TierSettings ts = GetTierSettings(m_tier);
    float thickness = ts.armorThickness * m_faction.armorBias;
    int layers = std::max(1, static_cast<int>(thickness));

    // Snapshot current block positions to avoid iterating newly added blocks
    std::vector<Vector3Int> positions;
    positions.reserve(ship.blocks.size());
    for (const auto& b : ship.blocks) {
        positions.push_back(b->gridPos);
    }

    for (int layer = 0; layer < layers; ++layer) {
        for (const auto& pos : positions) {
            // Check the 6 face neighbours
            static const Vector3Int dirs[] = {
                {1,0,0}, {-1,0,0}, {0,1,0}, {0,-1,0}, {0,0,1}, {0,0,-1}
            };
            for (const auto& d : dirs) {
                Vector3Int candidate = pos + d;
                if (ship.occupiedCells.find(candidate) == ship.occupiedCells.end()) {
                    PlaceBlock(ship, candidate, Vector3Int::One(),
                               BlockType::Armor, BlockShape::Cube);
                }
            }
        }
    }
}

// ---- AddEngines: place at the rear -Z face ---------------------------------

void AIShipBuilder::AddEngines(Ship& ship) {
    int radius = HullRadius(m_faction.silhouette.thickness);
    int engineCount = std::max(1, static_cast<int>(2.0f * m_faction.engineBias));

    // Place engines along the rear (z = -1)
    int placed = 0;
    for (int x = -radius; x <= radius && placed < engineCount; ++x) {
        Vector3Int pos{x, 0, -1};
        if (ship.occupiedCells.find(pos) == ship.occupiedCells.end()) {
            PlaceBlock(ship, pos, Vector3Int::One(), BlockType::Engine, BlockShape::Cube);
            ++placed;
        }
    }

    // Also add a generator near the center
    Vector3Int genPos{0, 0, 1};
    if (ship.occupiedCells.find(genPos) != ship.occupiedCells.end()) {
        // Already occupied — find an empty neighbour
        for (int z = 2; z < SpineLength(m_faction.silhouette.length); ++z) {
            genPos = {0, -1, z};
            if (ship.occupiedCells.find(genPos) == ship.occupiedCells.end()) break;
        }
    }
    PlaceBlock(ship, genPos, Vector3Int::One(), BlockType::Generator, BlockShape::Cube);
}

// ---- AddWeapons: weapon mounts on top / side surfaces ----------------------

void AIShipBuilder::AddWeapons(Ship& ship) {
    TierSettings ts = GetTierSettings(m_tier);
    int weaponsNeeded = static_cast<int>(ts.weaponSlots * m_faction.weaponBias);
    int length = SpineLength(m_faction.silhouette.length);
    int radius = HullRadius(m_faction.silhouette.thickness);

    int placed = 0;
    // Distribute weapon mounts along the top of the ship
    for (int z = 1; z < length && placed < weaponsNeeded; z += std::max(1, length / weaponsNeeded)) {
        Vector3Int pos{0, radius + 1, z};
        if (ship.occupiedCells.find(pos) == ship.occupiedCells.end()) {
            PlaceBlock(ship, pos, Vector3Int::One(), BlockType::WeaponMount, BlockShape::Cube);
            ++placed;
        }
    }

    // Fill remaining slots on the sides
    for (int z = 1; z < length && placed < weaponsNeeded; z += std::max(1, length / weaponsNeeded)) {
        Vector3Int pos{radius + 1, 0, z};
        if (ship.occupiedCells.find(pos) == ship.occupiedCells.end()) {
            PlaceBlock(ship, pos, Vector3Int::One(), BlockType::WeaponMount, BlockShape::Cube);
            ++placed;
        }
    }
}

// ---- ApplyMaterials: set material types based on palette --------------------

void AIShipBuilder::ApplyMaterials(Ship& ship) {
    for (auto& block : ship.blocks) {
        switch (block->type) {
            case BlockType::Armor:
                block->material = m_faction.palette.armor;
                break;
            case BlockType::Engine:
            case BlockType::Generator:
                block->material = m_faction.palette.accent;
                break;
            case BlockType::WeaponMount:
                block->material = m_faction.palette.accent;
                break;
            default:
                block->material = m_faction.palette.hull;
                break;
        }
        // Recompute HP with final material
        block->maxHP = GetBlockBaseHP(block->type)
                     * MaterialDatabase::Get(block->material).hpMultiplier;
        block->currentHP = block->maxHP;
    }
}

} // namespace subspace
