#include "ships/CompiledShipStats.h"
#include "ships/BlockDefinition.h"

#include <algorithm>
#include <cmath>

namespace subspace {
namespace {

float LengthSquared(const Vector3& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

float Length(const Vector3& v) {
    return std::sqrt(LengthSquared(v));
}

Vector3 BlockCenter(const Block& block) {
    return Vector3(
        static_cast<float>(block.gridPos.x) + static_cast<float>(block.size.x) * 0.5f,
        static_cast<float>(block.gridPos.y) + static_cast<float>(block.size.y) * 0.5f,
        static_cast<float>(block.gridPos.z) + static_cast<float>(block.size.z) * 0.5f);
}

float DefinitionMass(const Block& block) {
    const auto& def = BlockDefinitionDatabase::GetDefinition(block.type);
    return block.EffectiveVolume() * def.massPerUnitVolume;
}

float RuntimePowerGeneration(const Block& block, float volume, const BlockDefinition& def) {
    if (block.powerGeneration > 0.0f) return block.powerGeneration;
    const auto& material = MaterialDatabase::Get(block.material);
    return def.powerGenerationPerVolume * volume * material.energyBonus;
}

float RuntimeThrust(const Block& block, float volume, const BlockDefinition& def) {
    if (block.thrustPower > 0.0f) return block.thrustPower;
    const auto& material = MaterialDatabase::Get(block.material);
    return def.thrustPowerPerVolume * volume * material.energyBonus;
}

float RuntimeShield(const Block& block, float volume, const BlockDefinition& def) {
    if (block.shieldCapacity > 0.0f) return block.shieldCapacity;
    const auto& material = MaterialDatabase::Get(block.material);
    return def.shieldCapacityPerVolume * volume * material.shieldMultiplier;
}

} // namespace

float CompiledShipStats::AvailablePower() const {
    return powerGeneration - powerConsumption;
}

bool CompiledShipStats::HasSufficientPower() const {
    return powerGeneration >= powerConsumption;
}

float CompiledShipStats::PowerFactor() const {
    if (powerConsumption <= 0.0f) return 1.0f;
    return std::min(1.0f, powerGeneration / powerConsumption);
}

float CompiledShipStats::EffectiveThrust() const {
    return thrust * PowerFactor();
}

float CompiledShipStats::EffectiveTorque() const {
    return torque * PowerFactor();
}

float CompiledShipStats::Acceleration() const {
    return mass > 0.0f ? EffectiveThrust() / mass : 0.0f;
}

float CompiledShipStats::MaxSpeed() const {
    return Acceleration() * 10.0f;
}

float CompiledShipStats::MaxRotationSpeed() const {
    return momentOfInertia > 0.0f ? EffectiveTorque() / momentOfInertia : 0.0f;
}

bool CompiledShipStats::IsValid() const {
    return totalBlocks > 0;
}

CompiledShipStats ShipStatsCompiler::Compile(const Ship& ship) {
    return Compile(ship.blocks);
}

CompiledShipStats ShipStatsCompiler::Compile(const std::vector<std::shared_ptr<Block>>& blocks) {
    CompiledShipStats stats;
    if (blocks.empty()) return stats;

    // Pass 1: mass, center of mass, hit points.
    Vector3 weightedPos;
    for (const auto& blockPtr : blocks) {
        if (!blockPtr) continue;
        const Block& block = *blockPtr;
        const float blockMass = DefinitionMass(block);
        const Vector3 center = BlockCenter(block);

        stats.mass += blockMass;
        weightedPos = weightedPos + center * blockMass;
        stats.totalHitPoints += block.maxHP;
        stats.currentHitPoints += block.currentHP;
        ++stats.totalBlocks;
        if (block.IsAlive()) ++stats.aliveBlocks;
    }

    if (stats.mass > 0.0f) {
        stats.centerOfMass = weightedPos * (1.0f / stats.mass);
    }

    // Pass 2: functional stats relative to center of mass.
    for (const auto& blockPtr : blocks) {
        if (!blockPtr) continue;
        const Block& block = *blockPtr;
        const auto& def = BlockDefinitionDatabase::GetDefinition(block.type);
        const float volume = block.EffectiveVolume();
        const float blockMass = DefinitionMass(block);
        const Vector3 r = BlockCenter(block) - stats.centerOfMass;

        stats.momentOfInertia += blockMass * LengthSquared(r);
        stats.powerGeneration += RuntimePowerGeneration(block, volume, def);
        stats.powerConsumption += def.powerConsumptionPerVolume * volume;

        if (block.type == BlockType::Engine || block.type == BlockType::Thruster) {
            const float blockThrust = RuntimeThrust(block, volume, def);
            stats.thrust += blockThrust;
            if (block.type == BlockType::Thruster) {
                const float leverage = 1.0f + Length(r) * 0.1f;
                stats.torque += blockThrust * leverage * 0.5f;
            }
        } else if (block.type == BlockType::Gyro) {
            const float gyroPower = RuntimeThrust(block, volume, def);
            const float leverage = 1.0f + Length(r) * 0.05f;
            stats.torque += gyroPower * leverage;
        }

        stats.shieldCapacity += RuntimeShield(block, volume, def);
        if (block.type == BlockType::Armor) {
            stats.armorPoints += block.currentHP;
        }
        if (block.type == BlockType::Cargo) {
            stats.cargoCapacity += def.cargoCapacityPerVolume * volume;
        }
        if (block.type == BlockType::CrewQuarters) {
            stats.crewCapacity += static_cast<int>(def.crewCapacityPerVolume * volume);
        }
        if (block.type == BlockType::WeaponMount) {
            ++stats.weaponMounts;
        }
        if (block.type == BlockType::HyperdriveCore) {
            stats.hasHyperdrive = true;
        }
        if (block.type == BlockType::PodDocking) {
            stats.hasPodDocking = true;
        }
    }

    stats.structuralIntegrity = stats.totalHitPoints > 0.0f
        ? (stats.currentHitPoints / stats.totalHitPoints) * 100.0f
        : 0.0f;

    return stats;
}

} // namespace subspace
