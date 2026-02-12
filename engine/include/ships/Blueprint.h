#pragma once

#include "ships/Block.h"
#include "ships/Ship.h"

#include <string>
#include <vector>

namespace subspace {

struct BlueprintBlockData {
    int posX, posY, posZ;
    int sizeX, sizeY, sizeZ;
    int rotationIndex;
    int shape;      // Cast from BlockShape
    int type;       // Cast from BlockType
    int material;   // Cast from MaterialType
};

struct Blueprint {
    std::string name;
    std::string author;
    std::string faction;
    int seed = 0;
    std::vector<BlueprintBlockData> blocks;

    // Save blueprint to JSON string
    std::string ToJson() const;

    // Load blueprint from JSON string
    static Blueprint FromJson(const std::string& json);

    // Create blueprint from a ship
    static Blueprint FromShip(const Ship& ship, const std::string& name, const std::string& author);

    // Load a ship from this blueprint
    Ship ToShip() const;

    // Validate blueprint data
    bool Validate() const;
};

} // namespace subspace
