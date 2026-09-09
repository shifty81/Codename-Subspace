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
            case BlockType::Thruster:
                caps.mobility += 10.0f * scale;
                break;
            case BlockType::WeaponMount:
                caps.firepower += 8.0f * scale;
                break;
            case BlockType::Generator:
            case BlockType::Battery:
                caps.power += 12.0f * scale;
                break;
            case BlockType::Gyro:
            case BlockType::Computer:
            case BlockType::HyperdriveCore:
                caps.command += 6.0f * scale;
                break;
            case BlockType::Armor:
            case BlockType::ShieldGenerator:
            case BlockType::IntegrityField:
                caps.defense += 5.0f * scale;
                break;
            case BlockType::Cargo:
            case BlockType::CrewQuarters:
            case BlockType::PodDocking:
                caps.cargo += 15.0f * scale;
                break;
            case BlockType::Hull:
            case BlockType::Framework:
                // Hull/framework blocks do not contribute to a specific capability.
                break;
        }
    }

    return caps;
}

float CapabilitySystem::GetBlockCapabilityWeight(BlockType type,
                                                  const std::string& capability) {
    if (capability == "mobility"  && (type == BlockType::Engine || type == BlockType::Thruster)) return 10.0f;
    if (capability == "firepower" && type == BlockType::WeaponMount) return 8.0f;
    if (capability == "power"     && (type == BlockType::Generator || type == BlockType::Battery)) return 12.0f;
    if (capability == "command"   && (type == BlockType::Gyro || type == BlockType::Computer || type == BlockType::HyperdriveCore)) return 6.0f;
    if (capability == "defense"   && (type == BlockType::Armor || type == BlockType::ShieldGenerator || type == BlockType::IntegrityField)) return 5.0f;
    if (capability == "cargo"     && (type == BlockType::Cargo || type == BlockType::CrewQuarters || type == BlockType::PodDocking)) return 15.0f;
    return 0.0f;
}

std::string CapabilitySystem::GetBlockTypeName(BlockType type) {
    return BlockTypeName(type);
}

} // namespace subspace
