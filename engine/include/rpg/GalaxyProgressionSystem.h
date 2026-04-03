#pragma once

#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/ecs/EntityManager.h"
#include "core/persistence/SaveGameManager.h"
#include "navigation/NavigationSystem.h"

#include <string>
#include <vector>

namespace subspace {

// ---------------------------------------------------------------------------
// Material tier
// ---------------------------------------------------------------------------

/// Ordered from weakest (rim) to strongest (galactic core).
enum class MaterialTier {
    Iron      = 0,
    Titanium  = 1,
    Naonite   = 2,
    Trinium   = 3,
    Xanion    = 4,
    Ogonite   = 5,
    Avorion   = 6
};

const char* MaterialTierName(MaterialTier tier);

// ---------------------------------------------------------------------------
// PlayerProgressionComponent
// ---------------------------------------------------------------------------

/// Tracks a player's galaxy-wide progression (current zone, tier reached).
class PlayerProgressionComponent : public IComponent {
public:
    SectorCoordinate currentSector;
    int              distanceFromCenter = 0;
    MaterialTier     highestTierReached = MaterialTier::Iron;
    MaterialTier     currentZoneTier    = MaterialTier::Iron;
    float            difficultyMult     = 1.0f;
    float            lootQualityMult    = 1.0f;

    ComponentData Serialize()   const;
    void Deserialize(const ComponentData& data);
};

// ---------------------------------------------------------------------------
// GalaxyProgressionSystem
// ---------------------------------------------------------------------------

/// Manages difficulty scaling, material availability, and zone tracking
/// as players travel from the galactic rim toward the core.
class GalaxyProgressionSystem : public SystemBase {
public:
    GalaxyProgressionSystem();
    explicit GalaxyProgressionSystem(EntityManager& em);

    void Update(float deltaTime) override;
    void SetEntityManager(EntityManager* em);

    /// Calculate Chebyshev distance from the galactic centre (0,0,0).
    static int DistanceFromCenter(const SectorCoordinate& sector);

    /// Highest material tier available at a given rim distance.
    static MaterialTier AvailableTier(int distanceFromCenter);

    /// Enemy difficulty multiplier (1.0 at rim, 10.0 at core).
    static float DifficultyMultiplier(int distanceFromCenter);

    /// Loot quality multiplier (1.0 at rim, 5.0 at core).
    static float LootQualityMultiplier(int distanceFromCenter);

private:
    EntityManager* _em = nullptr;

    void UpdatePlayerProgression(PlayerProgressionComponent& comp) const;
};

} // namespace subspace
