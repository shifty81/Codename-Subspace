#pragma once

#include "ships/Block.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

class Ship {
public:
    // Dual structure: list for iteration + map for fast lookup
    std::vector<std::shared_ptr<Block>> blocks;
    std::unordered_map<Vector3Int, std::shared_ptr<Block>> occupiedCells;

    // Ship stats (rebuilt on change)
    float totalMass = 0.0f;
    float thrust = 0.0f;
    float powerGen = 0.0f;
    float totalHP = 0.0f;
    float maxHP = 0.0f;

    std::string name;
    std::string faction;
    int seed = 0;

    size_t BlockCount() const;
    bool IsEmpty() const;
    void Clear();
};

} // namespace subspace
