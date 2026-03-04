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

    // Voxel damage events
    constexpr const char* BlockDamaged             = "ship.block.damaged";
    constexpr const char* BlockDestroyed           = "ship.block.destroyed";
    constexpr const char* BlockRepaired            = "ship.block.repaired";
    constexpr const char* SplashDamageApplied      = "ship.splash.damage";
    constexpr const char* PenetratingDamageApplied = "ship.penetrating.damage";

    // Structural integrity events
    constexpr const char* StructuralCheck   = "ship.structural.check";
    constexpr const char* ShipFragmented    = "ship.fragmented";
    constexpr const char* IntegrityRestored = "ship.integrity.restored";

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

    // Particle events
    constexpr const char* ParticleEmitted  = "particle.emitted";
    constexpr const char* ParticleBurst    = "particle.burst";
    constexpr const char* EmitterStarted   = "particle.emitter.started";
    constexpr const char* EmitterStopped   = "particle.emitter.stopped";

    // Achievement events
    constexpr const char* AchievementUnlocked  = "achievement.unlocked";
    constexpr const char* AchievementProgress  = "achievement.progress";

    // Collision layer events
    constexpr const char* CollisionLayerChanged = "physics.collision.layer_changed";
    constexpr const char* TriggerEntered        = "physics.trigger.entered";
    constexpr const char* TriggerExited         = "physics.trigger.exited";

    // Spatial partitioning events
    constexpr const char* OctreeRebuilt     = "spatial.octree.rebuilt";
    constexpr const char* SpatialQueryPerformed = "spatial.query.performed";

    // Pathfinding events
    constexpr const char* PathFound        = "navigation.path.found";
    constexpr const char* PathNotFound     = "navigation.path.not_found";
    constexpr const char* WaypointReached  = "navigation.waypoint.reached";
    constexpr const char* PathCompleted    = "navigation.path.completed";
    constexpr const char* NavGridBuilt     = "navigation.grid.built";

    // Ammunition events
    constexpr const char* AmmoDepleted   = "combat.ammo.depleted";
    constexpr const char* AmmoReloaded   = "combat.ammo.reloaded";

    // Target lock events
    constexpr const char* TargetLocked   = "combat.target.locked";
    constexpr const char* TargetLost     = "combat.target.lost";

    // Anomaly events
    constexpr const char* AnomalyDiscovered = "sector.anomaly.discovered";
    constexpr const char* AnomalyEffect     = "sector.anomaly.effect";

    // Shield events
    constexpr const char* ShieldAbsorbed   = "combat.shield.absorbed";
    constexpr const char* ShieldDepleted   = "combat.shield.depleted";
    constexpr const char* ShieldRestored   = "combat.shield.restored";
    constexpr const char* ShieldOvercharged = "combat.shield.overcharged";

    // Status effect events
    constexpr const char* StatusEffectApplied = "combat.status.applied";
    constexpr const char* StatusEffectExpired = "combat.status.expired";
    constexpr const char* StatusEffectRemoved = "combat.status.removed";
    constexpr const char* StatusEffectTick    = "combat.status.tick";

    // Loot events
    constexpr const char* LootGenerated = "loot.generated";
    constexpr const char* LootCollected = "loot.collected";
    constexpr const char* LootDropped   = "loot.dropped";
    constexpr const char* RareItemFound = "loot.rare_item";

    // Crafting events
    constexpr const char* CraftingStarted   = "crafting.started";
    constexpr const char* CraftingCompleted = "crafting.completed";
    constexpr const char* CraftingFailed    = "crafting.failed";
    constexpr const char* RecipeLearned     = "crafting.recipe.learned";

    // Reputation events
    constexpr const char* ReputationModified  = "reputation.changed";
    constexpr const char* StandingChanged    = "reputation.standing.changed";
    constexpr const char* ReputationDecayed  = "reputation.decayed";

    // Formation events
    constexpr const char* FormationCreated   = "formation.created";
    constexpr const char* FormationDisbanded = "formation.disbanded";
    constexpr const char* FormationChanged   = "formation.changed";
    constexpr const char* MemberJoined       = "formation.member.joined";
    constexpr const char* MemberLeft         = "formation.member.left";
} // namespace GameEvents

} // namespace subspace
