#pragma once

#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/ecs/EntityManager.h"
#include "voxel/VoxelSystem.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace subspace {

// ---------------------------------------------------------------------------
// ShipModuleSlot — attachment point on a modular ship
// ---------------------------------------------------------------------------

struct ShipModuleSlot {
    std::string slotId;
    std::string slotType;      ///< "Weapon", "Engine", "Shield", "Utility", …
    float posX = 0, posY = 0, posZ = 0;
    bool  occupied = false;
    std::string occupiedBy;    ///< Module name currently in this slot
};

// ---------------------------------------------------------------------------
// ModularShipBlueprint — recipe for assembling a modular ship
// ---------------------------------------------------------------------------

struct ModularShipBlueprint {
    std::string name;
    std::string shipClass;       ///< "Fighter", "Cruiser", "Freighter", …
    std::string hullMaterial = "Iron";
    float       hullLength   = 20.0f;
    float       hullWidth    = 8.0f;
    float       hullHeight   = 4.0f;
    int         weaponSlots  = 2;
    int         engineSlots  = 1;
    int         utilitySlots = 2;

    /// Preset modules to populate weapon slots.
    std::vector<std::string> defaultWeapons;
    /// Preset modules to populate utility slots.
    std::vector<std::string> defaultUtilities;
};

// ---------------------------------------------------------------------------
// ModularShipComponent — runtime ship built from the blueprint
// ---------------------------------------------------------------------------

class ModularShipComponent : public IComponent {
public:
    std::string shipClass;
    std::string hullMaterial;
    float mass      = 0.0f;
    float thrust    = 0.0f;
    float shieldCap = 0.0f;
    float powerGen  = 0.0f;

    std::vector<ShipModuleSlot> slots;

    /// Dirty flag — set when slots change; cleared after RecalculateStats.
    bool statsDirty = false;

    /// Recompute mass/thrust/shieldCap/powerGen from slot data.
    void RecalculateStats();

    ComponentData Serialize()  const;
    void Deserialize(const ComponentData& data);
};

// ---------------------------------------------------------------------------
// ModularShipFactory
// ---------------------------------------------------------------------------

/// Creates fully populated ModularShipComponent entities from blueprints.
class ModularShipFactory {
public:
    explicit ModularShipFactory(EntityManager& em);

    /// Create a ship entity from a blueprint and return its id.
    uint64_t CreateShip(const ModularShipBlueprint& blueprint,
                        float spawnX = 0, float spawnY = 0, float spawnZ = 0);

    /// Get a pre-defined blueprint by name (Fighter, Cruiser, Freighter, etc.)
    static ModularShipBlueprint GetPreset(const std::string& name);

    /// List available preset names.
    static std::vector<std::string> ListPresets();

private:
    EntityManager& _em;

    void PopulateSlots(ModularShipComponent& comp,
                       const ModularShipBlueprint& bp);
};

// ---------------------------------------------------------------------------
// ModularShipSyncSystem — keeps ModularShipComponent stats up to date
// ---------------------------------------------------------------------------

/// Periodically calls RecalculateStats for dirty ships.
class ModularShipSyncSystem : public SystemBase {
public:
    ModularShipSyncSystem();
    explicit ModularShipSyncSystem(EntityManager& em);

    void Update(float deltaTime) override;
    void SetEntityManager(EntityManager* em);

private:
    EntityManager* _em = nullptr;
};

} // namespace subspace
