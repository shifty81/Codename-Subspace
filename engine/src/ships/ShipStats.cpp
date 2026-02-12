#include "ships/ShipStats.h"

namespace subspace {

void ShipStats::Recalculate(Ship& ship) {
    ship.totalMass = 0.0f;
    ship.thrust = 0.0f;
    ship.powerGen = 0.0f;
    ship.totalHP = 0.0f;
    ship.maxHP = 0.0f;

    for (const auto& block : ship.blocks) {
        ship.totalMass += block->Mass();
        ship.totalHP += block->currentHP;
        ship.maxHP += block->maxHP;

        if (block->type == BlockType::Engine) {
            ship.thrust += EnginePower(*block);
        }
        if (block->type == BlockType::Generator) {
            ship.powerGen += GeneratorOutput(*block);
        }
    }
}

float ShipStats::EnginePower(const Block& block) {
    return static_cast<float>(block.size.z) * 100.0f *
           MaterialDatabase::Get(block.material).density;
}

float ShipStats::GeneratorOutput(const Block& block) {
    return block.Volume() * 50.0f *
           MaterialDatabase::Get(block.material).energyBonus;
}

} // namespace subspace
