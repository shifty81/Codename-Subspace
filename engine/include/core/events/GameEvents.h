#pragma once

#include <cstdint>
#include <string>

namespace subspace {

/// Base event data class.
struct GameEvent {
    std::string eventType;
    virtual ~GameEvent() = default;
};

/// Entity-related event data.
struct EntityEvent : GameEvent {
    uint64_t entityId = 0;
    std::string entityName;
};

/// Resource event data.
struct ResourceEvent : GameEvent {
    uint64_t entityId = 0;
    std::string resourceType;
    int amount = 0;
};

/// Collision event data.
struct CollisionEvent : GameEvent {
    uint64_t entity1Id = 0;
    uint64_t entity2Id = 0;
    float impactForce = 0.0f;
};

/// Progression event data.
struct ProgressionEvent : GameEvent {
    uint64_t entityId = 0;
    int level = 0;
    int experience = 0;
    int skillPoints = 0;
};

/// Common game event type constants (mirrors C# GameEvents).
namespace GameEvents {
    // Entity events
    constexpr const char* EntityCreated   = "entity.created";
    constexpr const char* EntityDestroyed = "entity.destroyed";
    constexpr const char* EntityDamaged   = "entity.damaged";

    // Component events
    constexpr const char* ComponentAdded   = "component.added";
    constexpr const char* ComponentRemoved = "component.removed";

    // Resource events
    constexpr const char* ResourceCollected = "resource.collected";
    constexpr const char* ResourceSpent     = "resource.spent";
    constexpr const char* InventoryFull     = "inventory.full";

    // Progression events
    constexpr const char* PlayerLevelUp     = "player.levelup";
    constexpr const char* ExperienceGained  = "player.experience";
    constexpr const char* SkillPointsEarned = "player.skillpoints";

    // Ship events
    constexpr const char* ShipDamaged      = "ship.damaged";
    constexpr const char* ShipDestroyed    = "ship.destroyed";
    constexpr const char* ShipRepaired     = "ship.repaired";
    constexpr const char* VoxelBlockAdded  = "ship.block.added";
    constexpr const char* VoxelBlockRemoved= "ship.block.removed";

    // Physics events
    constexpr const char* CollisionDetected = "physics.collision";
    constexpr const char* EntityCollision   = "physics.entity.collision";
    constexpr const char* VelocityChanged   = "physics.velocity";

    // Combat events
    constexpr const char* WeaponFired   = "combat.weapon.fired";
    constexpr const char* ProjectileHit = "combat.projectile.hit";
    constexpr const char* ShieldHit     = "combat.shield.hit";

    // Trading events
    constexpr const char* TradeCompleted = "trade.completed";
    constexpr const char* ItemPurchased  = "trade.purchased";
    constexpr const char* ItemSold       = "trade.sold";

    // Faction events
    constexpr const char* ReputationChanged   = "faction.reputation";
    constexpr const char* FactionStatusChanged = "faction.status";

    // Network events
    constexpr const char* ClientConnected    = "network.client.connected";
    constexpr const char* ClientDisconnected = "network.client.disconnected";
    constexpr const char* ServerStarted      = "network.server.started";
    constexpr const char* ServerStopped      = "network.server.stopped";

    // System events
    constexpr const char* GameStarted = "game.started";
    constexpr const char* GamePaused  = "game.paused";
    constexpr const char* GameResumed = "game.resumed";
    constexpr const char* GameSaved   = "game.saved";
    constexpr const char* GameLoaded  = "game.loaded";

    // Sector events
    constexpr const char* SectorEntered   = "sector.entered";
    constexpr const char* SectorExited    = "sector.exited";
    constexpr const char* SectorGenerated = "sector.generated";

    // Audio events
    constexpr const char* SoundPlayed     = "audio.sound.played";
    constexpr const char* SoundStopped    = "audio.sound.stopped";
    constexpr const char* MusicStarted    = "audio.music.started";
    constexpr const char* MusicStopped    = "audio.music.stopped";
    constexpr const char* MusicTrackChanged = "audio.music.track_changed";
} // namespace GameEvents

} // namespace subspace
