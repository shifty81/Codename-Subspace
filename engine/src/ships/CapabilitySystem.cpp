#include "ships/CapabilitySystem.h"

#include <sstream>

namespace subspace {

// ---------------------------------------------------------------------------
// ShipCapabilities
// ---------------------------------------------------------------------------

float ShipCapabilities::GetHealthFraction() const {
    if (blockCount == 0) return 0.0f;
    return static_cast<float>(aliveCount) / static_cast<float>(blockCount);
}

float ShipCapabilities::GetCapability(const std::string& name) const {
    if (name == "mobility")   return mobility;
    if (name == "firepower")  return firepower;
    if (name == "power")      return power;
    if (name == "command")    return command;
    if (name == "defense")    return defense;
    if (name == "cargo")      return cargo;
    if (name == "totalMass")  return totalMass;
    return 0.0f;
}

std::string ShipCapabilities::GetSummary() const {
    std::ostringstream ss;
    ss << "Caps[mob=" << mobility
       << " fp=" << firepower
       << " pwr=" << power
       << " cmd=" << command
       << " def=" << defense
       << " crg=" << cargo
       << " blk=" << aliveCount << "/" << blockCount
       << "]";
    return ss.str();
}

// ---------------------------------------------------------------------------
// CapabilitySystem
// ---------------------------------------------------------------------------

ShipCapabilities CapabilitySystem::Evaluate(const Ship& ship) {
    return EvaluateBlocks(ship.blocks);
}

ShipCapabilities CapabilitySystem::EvaluateBlocks(
        const std::vector<std::shared_ptr<Block>>& blocks) {
    ShipCapabilities caps;
    caps.blockCount = static_cast<int>(blocks.size());

    for (const auto& block : blocks) {
        if (!block) continue;

        float mass = block->Mass();
        caps.totalMass += mass;

        bool alive = block->currentHP > 0.0f;
        if (alive) {
            ++caps.aliveCount;
        }

        // Capability contribution scales with block volume and alive state.
        float volume = block->Volume();
        float scale = alive ? volume : 0.0f;

        switch (block->type) {
            case BlockType::Engine:
                caps.mobility += 10.0f * scale;
                break;
            case BlockType::WeaponMount:
                caps.firepower += 8.0f * scale;
                break;
            case BlockType::Generator:
                caps.power += 12.0f * scale;
                break;
            case BlockType::Gyro:
                caps.command += 6.0f * scale;
                break;
            case BlockType::Armor:
                caps.defense += 5.0f * scale;
                break;
            case BlockType::Cargo:
                caps.cargo += 15.0f * scale;
                break;
            case BlockType::Hull:
                // Hull blocks don't contribute to a specific capability.
                break;
        }
    }

    return caps;
}

float CapabilitySystem::GetBlockCapabilityWeight(BlockType type,
                                                  const std::string& capability) {
    if (capability == "mobility"  && type == BlockType::Engine)      return 10.0f;
    if (capability == "firepower" && type == BlockType::WeaponMount) return 8.0f;
    if (capability == "power"     && type == BlockType::Generator)   return 12.0f;
    if (capability == "command"   && type == BlockType::Gyro)        return 6.0f;
    if (capability == "defense"   && type == BlockType::Armor)       return 5.0f;
    if (capability == "cargo"     && type == BlockType::Cargo)       return 15.0f;
    return 0.0f;
}

std::string CapabilitySystem::GetBlockTypeName(BlockType type) {
    switch (type) {
        case BlockType::Hull:        return "Hull";
        case BlockType::Armor:       return "Armor";
        case BlockType::Engine:      return "Engine";
        case BlockType::Generator:   return "Generator";
        case BlockType::Gyro:        return "Gyro";
        case BlockType::Cargo:       return "Cargo";
        case BlockType::WeaponMount: return "WeaponMount";
    }
    return "Unknown";
}

} // namespace subspace
