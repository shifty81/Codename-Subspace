#include "ships/Ship.h"

namespace subspace {

size_t Ship::BlockCount() const {
    return blocks.size();
}

bool Ship::IsEmpty() const {
    return blocks.empty();
}

void Ship::Clear() {
    blocks.clear();
    occupiedCells.clear();
    totalMass = 0.0f;
    thrust = 0.0f;
    powerGen = 0.0f;
    totalHP = 0.0f;
    maxHP = 0.0f;
}

} // namespace subspace
