#pragma once

#include "core/Math.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

enum class ModuleType { Core, Engine, Weapon, Hull, Cargo, Shield, Utility };

struct Hardpoint {
    Vector3  localPosition;
    Vector3  direction;   // outward-facing normal
    bool     occupied = false;

    Hardpoint() : localPosition(), direction(), occupied(false) {}
    Hardpoint(Vector3 pos, Vector3 dir) : localPosition(pos), direction(dir), occupied(false) {}
};

struct ModuleDef {
    std::string  id;
    ModuleType   type;
    std::string  meshId;
    float        mass = 1.0f;
    float        hp   = 100.0f;
    float        powerDraw   = 0.0f;
    float        powerOutput = 0.0f;
    float        thrustOutput = 0.0f;
    float        cargoCapacity = 0.0f;
    float        shieldStrength = 0.0f;

    std::vector<Hardpoint> hardpoints;

    int HardpointCount() const;
    int FreeHardpointCount() const;
};

struct ModuleInstance {
    const ModuleDef* def = nullptr;
    Vector3          position;   // world-space offset from ship origin
    int              rotation = 0; // 0-3, 90° increments
    int              parent   = -1;
    std::vector<int> children;
    float            currentHP = 0.0f;

    bool IsAlive() const;
};

struct ModularShip {
    std::vector<ModuleInstance> modules;
    std::string name;
    std::string faction;

    // Derived stats (recalculated on change)
    float totalMass    = 0.0f;
    float totalThrust  = 0.0f;
    float totalPowerGen   = 0.0f;
    float totalPowerDraw  = 0.0f;
    float totalHP      = 0.0f;
    float totalCargo   = 0.0f;
    float totalShield  = 0.0f;

    size_t ModuleCount() const;
    bool   IsEmpty() const;
    bool   HasCore() const;
    bool   PowerBalanced() const;
    bool   CanAccelerate() const;

    void RecalculateStats();
    int  AddModule(const ModuleDef* def, Vector3 position, int parentIndex = -1);
    void DestroyModule(int index);
};

class ModuleDatabase {
public:
    // Pre-defined module library
    static const ModuleDef& CoreSmall();
    static const ModuleDef& CoreMedium();
    static const ModuleDef& EngineSmall();
    static const ModuleDef& EngineLarge();
    static const ModuleDef& WeaponTurret();
    static const ModuleDef& WeaponRailgun();
    static const ModuleDef& HullPlate();
    static const ModuleDef& HullBeam();
    static const ModuleDef& CargoSmall();
    static const ModuleDef& CargoLarge();
    static const ModuleDef& ShieldGenerator();
    static const ModuleDef& UtilityScanner();

    static std::vector<const ModuleDef*> GetAll();
    static std::vector<const ModuleDef*> GetByType(ModuleType type);

private:
    static void Initialize();
    static bool s_initialized;
    static std::vector<ModuleDef> s_modules;
};

} // namespace subspace
