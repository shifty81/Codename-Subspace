#include "ships/ShipArchetype.h"

#include <algorithm>
#include <queue>

namespace subspace {

// ---------------------------------------------------------------------------
// Pre-defined archetypes
// ---------------------------------------------------------------------------
ShipArchetype ShipArchetypes::Interceptor() {
    return {"interceptor", ArchetypeClass::Interceptor, 4, 8, 1, 2, 2, 20.0f, 0.6f};
}

ShipArchetype ShipArchetypes::FrigateArchetype() {
    return {"frigate", ArchetypeClass::Frigate, 8, 14, 2, 4, 2, 50.0f, 0.7f};
}

ShipArchetype ShipArchetypes::Freighter() {
    return {"freighter", ArchetypeClass::Freighter, 6, 12, 0, 1, 1, 80.0f, 0.1f};
}

ShipArchetype ShipArchetypes::CruiserArchetype() {
    return {"cruiser", ArchetypeClass::Cruiser, 12, 20, 3, 6, 3, 100.0f, 0.8f};
}

ShipArchetype ShipArchetypes::BattleshipArchetype() {
    return {"battleship", ArchetypeClass::Battleship, 18, 30, 5, 10, 4, 200.0f, 0.9f};
}

std::vector<ShipArchetype> ShipArchetypes::GetAll() {
    return {
        Interceptor(),
        FrigateArchetype(),
        Freighter(),
        CruiserArchetype(),
        BattleshipArchetype()
    };
}

// ---------------------------------------------------------------------------
// ModularShipGenerator
// ---------------------------------------------------------------------------
ModularShipGenerator::ModularShipGenerator(const ShipArchetype& archetype,
                                           const FactionProfile& faction,
                                           int seed)
    : m_archetype(archetype)
    , m_faction(faction)
    , m_seed(seed)
    , m_rngState(seed)
{
}

int ModularShipGenerator::NextRandom() {
    // Simple LCG matching AIShipBuilder pattern
    m_rngState = m_rngState * 1103515245 + 12345;
    return (m_rngState >> 16) & 0x7FFF;
}

float ModularShipGenerator::NextRandomFloat() {
    return static_cast<float>(NextRandom()) / 32767.0f;
}

const ModuleDef* ModularShipGenerator::PickModuleForSlot(ModuleType preferred) const {
    auto candidates = ModuleDatabase::GetByType(preferred);
    if (candidates.empty()) return nullptr;
    // Use const_cast-free approach: copy state for pick
    int idx = ((m_rngState >> 16) & 0x7FFF) % static_cast<int>(candidates.size());
    if (idx < 0) idx = 0;
    return candidates[idx];
}

Vector3 ModularShipGenerator::NextHardpointPosition(const ModularShip& ship, int moduleIndex) const {
    if (moduleIndex < 0 || moduleIndex >= static_cast<int>(ship.modules.size())) {
        return Vector3(0, 0, 0);
    }

    const auto& mod = ship.modules[moduleIndex];
    if (!mod.def) return mod.position;

    // Find first free hardpoint
    for (const auto& hp : mod.def->hardpoints) {
        if (!hp.occupied) {
            return Vector3(
                mod.position.x + hp.localPosition.x + hp.direction.x * 2.0f,
                mod.position.y + hp.localPosition.y + hp.direction.y * 2.0f,
                mod.position.z + hp.localPosition.z + hp.direction.z * 2.0f
            );
        }
    }

    // Fallback: offset from parent position
    return Vector3(
        mod.position.x + static_cast<float>(((m_rngState >> 8) & 0xF) - 8),
        mod.position.y,
        mod.position.z + static_cast<float>(((m_rngState >> 4) & 0xF) - 8)
    );
}

ModularShip ModularShipGenerator::Generate() {
    ModularShip ship;
    ship.name = m_faction.displayName + " " + m_archetype.id;
    ship.faction = m_faction.id;

    PlaceCore(ship);
    GrowStructure(ship);
    AddEngines(ship);
    AddWeapons(ship);
    FillRemaining(ship);

    // Validate and retry with simplified ship if needed
    if (!ValidateShip(ship) && !ship.IsEmpty()) {
        // Add minimum engines if missing
        while (!ship.CanAccelerate() && static_cast<int>(ship.ModuleCount()) < m_archetype.maxModules) {
            const ModuleDef* eng = PickModuleForSlot(ModuleType::Engine);
            if (!eng) break;
            int parent = 0;
            Vector3 pos = NextHardpointPosition(ship, parent);
            ship.AddModule(eng, pos, parent);
            NextRandom(); // advance state
        }
    }

    ship.RecalculateStats();
    return ship;
}

void ModularShipGenerator::PlaceCore(ModularShip& ship) {
    // Larger ships get larger cores
    const ModuleDef* core;
    if (m_archetype.archClass == ArchetypeClass::Cruiser ||
        m_archetype.archClass == ArchetypeClass::Battleship) {
        core = &ModuleDatabase::CoreMedium();
    } else {
        core = &ModuleDatabase::CoreSmall();
    }
    ship.AddModule(core, Vector3(0, 0, 0));
}

void ModularShipGenerator::GrowStructure(ModularShip& ship) {
    // BFS growth from core — attach hull modules to available hardpoints
    std::queue<int> openModules;
    openModules.push(0); // core index

    int targetHull = (m_archetype.maxModules - m_archetype.minWeapons - m_archetype.minEngines) / 2;
    if (targetHull < 1) targetHull = 1;
    int hullPlaced = 0;

    while (!openModules.empty() && hullPlaced < targetHull) {
        int current = openModules.front();
        openModules.pop();

        if (current >= static_cast<int>(ship.modules.size())) continue;

        const ModuleDef* hull = PickModuleForSlot(ModuleType::Hull);
        if (!hull) continue;

        Vector3 pos = NextHardpointPosition(ship, current);
        int child = ship.AddModule(hull, pos, current);
        openModules.push(child);
        hullPlaced++;
        NextRandom();
    }
}

void ModularShipGenerator::AddEngines(ModularShip& ship) {
    int engineCount = m_archetype.minEngines;
    // Interceptors get more engines relative to size
    if (m_archetype.archClass == ArchetypeClass::Interceptor) {
        engineCount = std::max(engineCount, 2);
    }

    for (int i = 0; i < engineCount && static_cast<int>(ship.ModuleCount()) < m_archetype.maxModules; i++) {
        const ModuleDef* eng;
        if (m_archetype.archClass == ArchetypeClass::Battleship ||
            m_archetype.archClass == ArchetypeClass::Cruiser) {
            eng = &ModuleDatabase::EngineLarge();
        } else {
            eng = &ModuleDatabase::EngineSmall();
        }

        // Engines go at the back (negative z)
        int parentIdx = 0;
        Vector3 pos(
            static_cast<float>(i - engineCount / 2) * 3.0f,
            0.0f,
            -static_cast<float>(i + 1) * 3.0f
        );
        ship.AddModule(eng, pos, parentIdx);
        NextRandom();
    }
}

void ModularShipGenerator::AddWeapons(ModularShip& ship) {
    int weaponCount = m_archetype.minWeapons +
        static_cast<int>(NextRandomFloat() * static_cast<float>(m_archetype.maxWeapons - m_archetype.minWeapons));

    for (int i = 0; i < weaponCount && static_cast<int>(ship.ModuleCount()) < m_archetype.maxModules; i++) {
        const ModuleDef* wpn;
        // Faction aggressiveness influences weapon choice
        if (m_faction.weaponBias > 1.0f || m_archetype.aggressiveness > 0.7f) {
            wpn = &ModuleDatabase::WeaponRailgun();
        } else {
            wpn = &ModuleDatabase::WeaponTurret();
        }

        // Weapons go on top
        int parentIdx = 0;
        Vector3 pos(
            static_cast<float>(i - weaponCount / 2) * 2.5f,
            2.0f,
            static_cast<float>(i) * 2.0f
        );
        ship.AddModule(wpn, pos, parentIdx);
        NextRandom();
    }
}

void ModularShipGenerator::FillRemaining(ModularShip& ship) {
    // Freighters always get at least one cargo module
    if (m_archetype.archClass == ArchetypeClass::Freighter && ship.totalCargo == 0.0f) {
        const ModuleDef* cargo = &ModuleDatabase::CargoLarge();
        int parentIdx = 0;
        Vector3 pos = NextHardpointPosition(ship, parentIdx);
        ship.AddModule(cargo, pos, parentIdx);
        NextRandom();
    }

    int currentCount = static_cast<int>(ship.ModuleCount());
    int target = m_archetype.minModules;

    while (currentCount < target) {
        ModuleType fillType;
        float roll = NextRandomFloat();

        if (m_archetype.archClass == ArchetypeClass::Freighter) {
            // Freighters fill with cargo
            fillType = (roll < 0.6f) ? ModuleType::Cargo : ModuleType::Hull;
        } else if (m_archetype.aggressiveness > 0.5f) {
            // Combat ships fill with shields/hull
            fillType = (roll < 0.4f) ? ModuleType::Shield : ModuleType::Hull;
        } else {
            fillType = ModuleType::Utility;
        }

        const ModuleDef* mod = PickModuleForSlot(fillType);
        if (!mod) {
            mod = PickModuleForSlot(ModuleType::Hull); // fallback
            if (!mod) break;
        }

        int parentIdx = 0;
        if (!ship.modules.empty()) {
            parentIdx = NextRandom() % static_cast<int>(ship.modules.size());
        }
        Vector3 pos = NextHardpointPosition(ship, parentIdx);
        ship.AddModule(mod, pos, parentIdx);
        currentCount = static_cast<int>(ship.ModuleCount());
    }
}

bool ModularShipGenerator::ValidateShip(const ModularShip& ship) const {
    if (!ship.HasCore()) return false;
    if (!ship.CanAccelerate()) return false;
    if (static_cast<int>(ship.ModuleCount()) < m_archetype.minModules) return false;
    return true;
}

} // namespace subspace
