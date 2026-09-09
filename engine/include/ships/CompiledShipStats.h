#pragma once

#include "core/Math.h"
#include "ships/Block.h"
#include "ships/Ship.h"

#include <memory>
#include <vector>

namespace subspace {

struct CompiledShipStats {
    // Structural
    float mass = 0.0f;
    float totalHitPoints = 0.0f;
    float currentHitPoints = 0.0f;
    Vector3 centerOfMass;
    float momentOfInertia = 0.0f;
    float structuralIntegrity = 0.0f;

    // Power system
    float powerGeneration = 0.0f;
    float powerConsumption = 0.0f;

    // Propulsion
    float thrust = 0.0f;
    float torque = 0.0f;

    // Defense
    float shieldCapacity = 0.0f;
    float armorPoints = 0.0f;

    // Utility
    float cargoCapacity = 0.0f;
    int crewCapacity = 0;
    int weaponMounts = 0;

    // Special systems
    bool hasHyperdrive = false;
    bool hasPodDocking = false;

    // Block counts
    int totalBlocks = 0;
    int aliveBlocks = 0;

    float AvailablePower() const;
    bool HasSufficientPower() const;
    float PowerFactor() const;
    float EffectiveThrust() const;
    float EffectiveTorque() const;
    float Acceleration() const;
    float MaxSpeed() const;
    float MaxRotationSpeed() const;
    bool IsValid() const;
};

class ShipStatsCompiler {
public:
    static CompiledShipStats Compile(const Ship& ship);
    static CompiledShipStats Compile(const std::vector<std::shared_ptr<Block>>& blocks);
};

} // namespace subspace
