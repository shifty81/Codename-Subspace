#pragma once

#include <cstdint>
#include <string>

namespace subspace {

/// Collision layer categories as bit flags.
/// Each entity belongs to one or more layers and has a mask of layers it collides with.
enum class CollisionCategory : uint32_t {
    None       = 0,
    Player     = 1 << 0,   // Player ship
    Enemy      = 1 << 1,   // Enemy/NPC ships
    Projectile = 1 << 2,   // Bullets, missiles, etc.
    Asteroid   = 1 << 3,   // Asteroids and space rocks
    Station    = 1 << 4,   // Space stations
    Debris     = 1 << 5,   // Wreckage, fragments
    Shield     = 1 << 6,   // Shield volumes
    Sensor     = 1 << 7,   // Sensor/detection volumes (trigger only)
    Pickup     = 1 << 8,   // Collectible items
    Missile    = 1 << 9,   // Guided missiles (separate from projectile)
    All        = 0xFFFFFFFF
};

// Bitwise operators for combining categories
inline constexpr CollisionCategory operator|(CollisionCategory a, CollisionCategory b) {
    return static_cast<CollisionCategory>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline constexpr CollisionCategory operator&(CollisionCategory a, CollisionCategory b) {
    return static_cast<CollisionCategory>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline constexpr CollisionCategory operator~(CollisionCategory a) {
    return static_cast<CollisionCategory>(~static_cast<uint32_t>(a));
}

inline constexpr bool HasCategory(CollisionCategory flags, CollisionCategory test) {
    return (static_cast<uint32_t>(flags) & static_cast<uint32_t>(test)) != 0;
}

/// Predefined collision presets for common entity types.
namespace CollisionPresets {
    /// Default: belongs to All, collides with All
    struct Preset {
        CollisionCategory layer;
        CollisionCategory mask;
    };

    inline constexpr Preset Default()    { return { CollisionCategory::All, CollisionCategory::All }; }
    inline constexpr Preset PlayerShip() {
        return { CollisionCategory::Player,
                 CollisionCategory::Enemy | CollisionCategory::Projectile | CollisionCategory::Asteroid |
                 CollisionCategory::Station | CollisionCategory::Debris | CollisionCategory::Shield |
                 CollisionCategory::Pickup | CollisionCategory::Missile };
    }
    inline constexpr Preset EnemyShip()  {
        return { CollisionCategory::Enemy,
                 CollisionCategory::Player | CollisionCategory::Enemy | CollisionCategory::Projectile |
                 CollisionCategory::Asteroid | CollisionCategory::Station | CollisionCategory::Debris |
                 CollisionCategory::Shield | CollisionCategory::Missile };
    }
    inline constexpr Preset PlayerProjectile() {
        return { CollisionCategory::Projectile,
                 CollisionCategory::Enemy | CollisionCategory::Asteroid | CollisionCategory::Station |
                 CollisionCategory::Shield };
    }
    inline constexpr Preset EnemyProjectile() {
        return { CollisionCategory::Projectile,
                 CollisionCategory::Player | CollisionCategory::Asteroid | CollisionCategory::Station |
                 CollisionCategory::Shield };
    }
    inline constexpr Preset AsteroidPreset() {
        return { CollisionCategory::Asteroid,
                 CollisionCategory::Player | CollisionCategory::Enemy | CollisionCategory::Projectile |
                 CollisionCategory::Missile };
    }
    inline constexpr Preset StationPreset() {
        return { CollisionCategory::Station,
                 CollisionCategory::Player | CollisionCategory::Enemy | CollisionCategory::Projectile |
                 CollisionCategory::Missile };
    }
    inline constexpr Preset DebrisPreset() {
        return { CollisionCategory::Debris,
                 CollisionCategory::Player | CollisionCategory::Enemy };
    }
    inline constexpr Preset SensorPreset() {
        return { CollisionCategory::Sensor,
                 CollisionCategory::Player | CollisionCategory::Enemy | CollisionCategory::Asteroid |
                 CollisionCategory::Station };
    }
    inline constexpr Preset PickupPreset() {
        return { CollisionCategory::Pickup,
                 CollisionCategory::Player };
    }
    inline constexpr Preset MissilePreset() {
        return { CollisionCategory::Missile,
                 CollisionCategory::Player | CollisionCategory::Enemy | CollisionCategory::Asteroid |
                 CollisionCategory::Station | CollisionCategory::Shield };
    }
} // namespace CollisionPresets

/// Check if two objects with given layer/mask should collide.
/// Collision occurs when A's layer overlaps B's mask AND B's layer overlaps A's mask.
inline constexpr bool ShouldCollide(CollisionCategory layerA, CollisionCategory maskA,
                                     CollisionCategory layerB, CollisionCategory maskB) {
    return HasCategory(maskA, layerB) && HasCategory(maskB, layerA);
}

/// Get the name of a single collision category (for debugging).
/// Only returns a meaningful name for single-bit categories; combined flags return "Unknown".
inline std::string GetCategoryName(CollisionCategory cat) {
    switch (cat) {
        case CollisionCategory::None:       return "None";
        case CollisionCategory::Player:     return "Player";
        case CollisionCategory::Enemy:      return "Enemy";
        case CollisionCategory::Projectile: return "Projectile";
        case CollisionCategory::Asteroid:   return "Asteroid";
        case CollisionCategory::Station:    return "Station";
        case CollisionCategory::Debris:     return "Debris";
        case CollisionCategory::Shield:     return "Shield";
        case CollisionCategory::Sensor:     return "Sensor";
        case CollisionCategory::Pickup:     return "Pickup";
        case CollisionCategory::Missile:    return "Missile";
        case CollisionCategory::All:        return "All";
        default:                            return "Unknown";
    }
}

} // namespace subspace
