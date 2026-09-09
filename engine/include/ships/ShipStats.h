#pragma once

#include "ships/Block.h"
#include "ships/Ship.h"

namespace subspace {

class ShipStats {
public:
    // Recalculate all stats from blocks
    static void Recalculate(Ship& ship);

    // Engine power formula
    static float EnginePower(const Block& block);

    // Generator output formula
    static float GeneratorOutput(const Block& block);
};

} // namespace subspace
