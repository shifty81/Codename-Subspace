#include "ships/ModuleDef.h"

#include <algorithm>

namespace subspace {

// ---------------------------------------------------------------------------
// ModuleDef
// ---------------------------------------------------------------------------
int ModuleDef::HardpointCount() const {
    return static_cast<int>(hardpoints.size());
}

int ModuleDef::FreeHardpointCount() const {
    int count = 0;
    for (const auto& hp : hardpoints) {
        if (!hp.occupied) count++;
    }
    return count;
}

// ---------------------------------------------------------------------------
// ModuleInstance
// ---------------------------------------------------------------------------
bool ModuleInstance::IsAlive() const {
    return def != nullptr && currentHP > 0.0f;
}

// ---------------------------------------------------------------------------
// ModularShip
// ---------------------------------------------------------------------------
size_t ModularShip::ModuleCount() const {
    size_t count = 0;
    for (const auto& m : modules) {
        if (m.IsAlive()) count++;
    }
    return count;
}

bool ModularShip::IsEmpty() const {
    return ModuleCount() == 0;
}

bool ModularShip::HasCore() const {
    for (const auto& m : modules) {
        if (m.IsAlive() && m.def && m.def->type == ModuleType::Core) return true;
    }
    return false;
}

bool ModularShip::PowerBalanced() const {
    return totalPowerGen >= totalPowerDraw;
}

bool ModularShip::CanAccelerate() const {
    return totalThrust > 0.0f && totalMass > 0.0f;
}

void ModularShip::RecalculateStats() {
    totalMass      = 0.0f;
    totalThrust    = 0.0f;
    totalPowerGen  = 0.0f;
    totalPowerDraw = 0.0f;
    totalHP        = 0.0f;
    totalCargo     = 0.0f;
    totalShield    = 0.0f;

    for (const auto& m : modules) {
        if (!m.IsAlive()) continue;
        const ModuleDef* d = m.def;
        totalMass      += d->mass;
        totalThrust    += d->thrustOutput;
        totalPowerGen  += d->powerOutput;
        totalPowerDraw += d->powerDraw;
        totalHP        += m.currentHP;
        totalCargo     += d->cargoCapacity;
        totalShield    += d->shieldStrength;
    }
}

int ModularShip::AddModule(const ModuleDef* def, Vector3 position, int parentIndex) {
    ModuleInstance inst;
    inst.def       = def;
    inst.position  = position;
    inst.parent    = parentIndex;
    inst.currentHP = def->hp;

    int index = static_cast<int>(modules.size());
    modules.push_back(inst);

    if (parentIndex >= 0 && parentIndex < static_cast<int>(modules.size())) {
        modules[parentIndex].children.push_back(index);
    }

    RecalculateStats();
    return index;
}

void ModularShip::DestroyModule(int index) {
    if (index < 0 || index >= static_cast<int>(modules.size())) return;

    // Recursively destroy children
    auto childrenCopy = modules[index].children;
    for (int child : childrenCopy) {
        DestroyModule(child);
    }

    modules[index].def = nullptr;
    modules[index].currentHP = 0.0f;
    modules[index].children.clear();

    RecalculateStats();
}

// ---------------------------------------------------------------------------
// ModuleDatabase
// ---------------------------------------------------------------------------
bool ModuleDatabase::s_initialized = false;
std::vector<ModuleDef> ModuleDatabase::s_modules;

void ModuleDatabase::Initialize() {
    if (s_initialized) return;

    // Core modules
    {
        ModuleDef m;
        m.id = "core_small";
        m.type = ModuleType::Core;
        m.meshId = "mesh_core_small";
        m.mass = 5.0f;
        m.hp = 200.0f;
        m.powerOutput = 10.0f;
        m.hardpoints = {
            {{0, 0,  1}, {0, 0,  1}},  // front
            {{0, 0, -1}, {0, 0, -1}},  // back
            {{1, 0,  0}, {1, 0,  0}},  // right
            {{-1, 0, 0}, {-1, 0, 0}},  // left
        };
        s_modules.push_back(m);
    }
    {
        ModuleDef m;
        m.id = "core_medium";
        m.type = ModuleType::Core;
        m.meshId = "mesh_core_medium";
        m.mass = 12.0f;
        m.hp = 400.0f;
        m.powerOutput = 25.0f;
        m.hardpoints = {
            {{0, 0,  2}, {0, 0,  1}},  // front
            {{0, 0, -2}, {0, 0, -1}},  // back
            {{2, 0,  0}, {1, 0,  0}},  // right
            {{-2, 0, 0}, {-1, 0, 0}},  // left
            {{0, 1,  0}, {0, 1,  0}},  // top
            {{0, -1, 0}, {0, -1, 0}},  // bottom
        };
        s_modules.push_back(m);
    }

    // Engine modules
    {
        ModuleDef m;
        m.id = "engine_small";
        m.type = ModuleType::Engine;
        m.meshId = "mesh_engine_small";
        m.mass = 3.0f;
        m.hp = 80.0f;
        m.powerDraw = 5.0f;
        m.thrustOutput = 50.0f;
        m.hardpoints = {
            {{0, 0, 1}, {0, 0, 1}},  // front (connect toward hull)
        };
        s_modules.push_back(m);
    }
    {
        ModuleDef m;
        m.id = "engine_large";
        m.type = ModuleType::Engine;
        m.meshId = "mesh_engine_large";
        m.mass = 8.0f;
        m.hp = 120.0f;
        m.powerDraw = 12.0f;
        m.thrustOutput = 150.0f;
        m.hardpoints = {
            {{0, 0, 1}, {0, 0, 1}},
            {{1, 0, 0}, {1, 0, 0}},
        };
        s_modules.push_back(m);
    }

    // Weapon modules
    {
        ModuleDef m;
        m.id = "weapon_turret";
        m.type = ModuleType::Weapon;
        m.meshId = "mesh_turret";
        m.mass = 4.0f;
        m.hp = 60.0f;
        m.powerDraw = 8.0f;
        m.hardpoints = {
            {{0, -1, 0}, {0, -1, 0}},  // bottom mount point
        };
        s_modules.push_back(m);
    }
    {
        ModuleDef m;
        m.id = "weapon_railgun";
        m.type = ModuleType::Weapon;
        m.meshId = "mesh_railgun";
        m.mass = 6.0f;
        m.hp = 70.0f;
        m.powerDraw = 15.0f;
        m.hardpoints = {
            {{0, 0, 1}, {0, 0, 1}},
        };
        s_modules.push_back(m);
    }

    // Hull modules
    {
        ModuleDef m;
        m.id = "hull_plate";
        m.type = ModuleType::Hull;
        m.meshId = "mesh_hull_plate";
        m.mass = 2.0f;
        m.hp = 150.0f;
        m.hardpoints = {
            {{0, 0,  1}, {0, 0,  1}},
            {{0, 0, -1}, {0, 0, -1}},
            {{1, 0,  0}, {1, 0,  0}},
            {{-1, 0, 0}, {-1, 0, 0}},
        };
        s_modules.push_back(m);
    }
    {
        ModuleDef m;
        m.id = "hull_beam";
        m.type = ModuleType::Hull;
        m.meshId = "mesh_hull_beam";
        m.mass = 3.0f;
        m.hp = 120.0f;
        m.hardpoints = {
            {{0, 0,  2}, {0, 0,  1}},
            {{0, 0, -2}, {0, 0, -1}},
        };
        s_modules.push_back(m);
    }

    // Cargo modules
    {
        ModuleDef m;
        m.id = "cargo_small";
        m.type = ModuleType::Cargo;
        m.meshId = "mesh_cargo_small";
        m.mass = 3.0f;
        m.hp = 80.0f;
        m.cargoCapacity = 50.0f;
        m.hardpoints = {
            {{0, 0,  1}, {0, 0,  1}},
            {{0, 0, -1}, {0, 0, -1}},
        };
        s_modules.push_back(m);
    }
    {
        ModuleDef m;
        m.id = "cargo_large";
        m.type = ModuleType::Cargo;
        m.meshId = "mesh_cargo_large";
        m.mass = 8.0f;
        m.hp = 100.0f;
        m.cargoCapacity = 200.0f;
        m.hardpoints = {
            {{0, 0,  2}, {0, 0,  1}},
            {{0, 0, -2}, {0, 0, -1}},
            {{2, 0,  0}, {1, 0,  0}},
            {{-2, 0, 0}, {-1, 0, 0}},
        };
        s_modules.push_back(m);
    }

    // Shield module
    {
        ModuleDef m;
        m.id = "shield_gen";
        m.type = ModuleType::Shield;
        m.meshId = "mesh_shield_gen";
        m.mass = 5.0f;
        m.hp = 90.0f;
        m.powerDraw = 10.0f;
        m.shieldStrength = 100.0f;
        m.hardpoints = {
            {{0, -1, 0}, {0, -1, 0}},
        };
        s_modules.push_back(m);
    }

    // Utility module
    {
        ModuleDef m;
        m.id = "utility_scanner";
        m.type = ModuleType::Utility;
        m.meshId = "mesh_scanner";
        m.mass = 1.0f;
        m.hp = 40.0f;
        m.powerDraw = 3.0f;
        m.hardpoints = {
            {{0, 1, 0}, {0, 1, 0}},
        };
        s_modules.push_back(m);
    }

    s_initialized = true;
}

static const ModuleDef& FindModule(const std::string& id) {
    ModuleDatabase::GetAll(); // ensure initialized
    // Linear scan — small database, no performance concern
    auto all = ModuleDatabase::GetAll();
    for (const auto* m : all) {
        if (m->id == id) return *m;
    }
    static ModuleDef fallback;
    return fallback;
}

const ModuleDef& ModuleDatabase::CoreSmall()       { Initialize(); return FindModule("core_small"); }
const ModuleDef& ModuleDatabase::CoreMedium()      { Initialize(); return FindModule("core_medium"); }
const ModuleDef& ModuleDatabase::EngineSmall()     { Initialize(); return FindModule("engine_small"); }
const ModuleDef& ModuleDatabase::EngineLarge()     { Initialize(); return FindModule("engine_large"); }
const ModuleDef& ModuleDatabase::WeaponTurret()    { Initialize(); return FindModule("weapon_turret"); }
const ModuleDef& ModuleDatabase::WeaponRailgun()   { Initialize(); return FindModule("weapon_railgun"); }
const ModuleDef& ModuleDatabase::HullPlate()       { Initialize(); return FindModule("hull_plate"); }
const ModuleDef& ModuleDatabase::HullBeam()        { Initialize(); return FindModule("hull_beam"); }
const ModuleDef& ModuleDatabase::CargoSmall()      { Initialize(); return FindModule("cargo_small"); }
const ModuleDef& ModuleDatabase::CargoLarge()      { Initialize(); return FindModule("cargo_large"); }
const ModuleDef& ModuleDatabase::ShieldGenerator() { Initialize(); return FindModule("shield_gen"); }
const ModuleDef& ModuleDatabase::UtilityScanner()  { Initialize(); return FindModule("utility_scanner"); }

std::vector<const ModuleDef*> ModuleDatabase::GetAll() {
    Initialize();
    std::vector<const ModuleDef*> result;
    result.reserve(s_modules.size());
    for (const auto& m : s_modules) {
        result.push_back(&m);
    }
    return result;
}

std::vector<const ModuleDef*> ModuleDatabase::GetByType(ModuleType type) {
    Initialize();
    std::vector<const ModuleDef*> result;
    for (const auto& m : s_modules) {
        if (m.type == type) result.push_back(&m);
    }
    return result;
}

} // namespace subspace
