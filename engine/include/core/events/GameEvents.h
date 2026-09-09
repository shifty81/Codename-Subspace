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

    // Capability events
    constexpr const char* CapabilityEvaluated   = "capability.evaluated";
    constexpr const char* CapabilityDegraded    = "capability.degraded";
    constexpr const char* CapabilityRestored    = "capability.restored";

    // Debug visualization events
    constexpr const char* DebugOverlayToggled   = "debug.overlay.toggled";
    constexpr const char* DebugCommandQueued    = "debug.command.queued";

    // Performance monitoring events
    constexpr const char* PerfFrameRecorded     = "perf.frame.recorded";
    constexpr const char* PerfSectionRecorded   = "perf.section.recorded";
    constexpr const char* PerfCounterRecorded   = "perf.counter.recorded";

    // Diplomacy events
    constexpr const char* WarDeclared            = "diplomacy.war.declared";
    constexpr const char* PeaceProposed          = "diplomacy.peace.proposed";
    constexpr const char* TreatyProposed         = "diplomacy.treaty.proposed";
    constexpr const char* TreatySigned           = "diplomacy.treaty.signed";
    constexpr const char* TreatyBroken           = "diplomacy.treaty.broken";
    constexpr const char* TreatyExpired          = "diplomacy.treaty.expired";
    constexpr const char* DiplomaticStatusChanged = "diplomacy.status.changed";

    // Research events
    constexpr const char* ResearchStarted        = "research.started";
    constexpr const char* ResearchCompleted       = "research.completed";
    constexpr const char* ResearchCancelled       = "research.cancelled";
    constexpr const char* TechUnlocked            = "research.tech.unlocked";

    // Notification events
    constexpr const char* NotificationAdded       = "notification.added";
    constexpr const char* NotificationRead        = "notification.read";
    constexpr const char* NotificationExpired     = "notification.expired";
    constexpr const char* NotificationDismissed   = "notification.dismissed";
    constexpr const char* CriticalAlert           = "notification.critical";

    // Inventory events
    constexpr const char* ItemAdded               = "inventory.item.added";
    constexpr const char* ItemRemoved             = "inventory.item.removed";
    constexpr const char* ItemTransferred         = "inventory.item.transferred";
    constexpr const char* InventoryOverweight     = "inventory.overweight";
    constexpr const char* InventorySorted         = "inventory.sorted";

    // Trade route events
    constexpr const char* TradeRouteStarted       = "trade_route.started";
    constexpr const char* TradeRouteStopped       = "trade_route.stopped";
    constexpr const char* TradeRouteCompleted     = "trade_route.completed";
    constexpr const char* TradeWaypointReached    = "trade_route.waypoint.reached";
    constexpr const char* TradeBuyCompleted       = "trade_route.buy.completed";
    constexpr const char* TradeSellCompleted      = "trade_route.sell.completed";

    // Hangar/Docking events
    constexpr const char* DockingRequested        = "hangar.docking.requested";
    constexpr const char* DockingCompleted        = "hangar.docking.completed";
    constexpr const char* DockingCancelled        = "hangar.docking.cancelled";
    constexpr const char* LaunchRequested         = "hangar.launch.requested";
    constexpr const char* LaunchCompleted         = "hangar.launch.completed";
    constexpr const char* ShipStored              = "hangar.ship.stored";
    constexpr const char* ShipRetrieved           = "hangar.ship.retrieved";

    // Wormhole events
    constexpr const char* WormholeActivated        = "wormhole.activated";
    constexpr const char* WormholeCollapsed         = "wormhole.collapsed";
    constexpr const char* WormholeTraversalStarted  = "wormhole.traversal.started";
    constexpr const char* WormholeTraversalCompleted = "wormhole.traversal.completed";
    constexpr const char* WormholeDestabilizing     = "wormhole.destabilizing";
    constexpr const char* WormholeLinkAdded         = "wormhole.link.added";

    // Ship class events
    constexpr const char* ShipClassAssigned         = "ship_class.assigned";
    constexpr const char* ShipClassChanged          = "ship_class.changed";
    constexpr const char* ShipClassUpgraded         = "ship_class.upgraded";

    // Refinery events
    constexpr const char* RefiningStarted           = "refinery.job.started";
    constexpr const char* RefiningCompleted          = "refinery.job.completed";
    constexpr const char* RefiningCancelled          = "refinery.job.cancelled";
    constexpr const char* RefiningCollected           = "refinery.job.collected";
    constexpr const char* RefineryTierChanged        = "refinery.tier.changed";

    // Scanning events
    constexpr const char* ScanStarted               = "scanning.scan.started";
    constexpr const char* ScanCompleted              = "scanning.scan.completed";
    constexpr const char* ScanCancelled              = "scanning.scan.cancelled";
    constexpr const char* SignatureClassified        = "scanning.signature.classified";
    constexpr const char* ScannerTypeChanged         = "scanning.scanner.type_changed";

    // Salvage events
    constexpr const char* SalvageStarted             = "salvage.operation.started";
    constexpr const char* SalvageCompleted           = "salvage.operation.completed";
    constexpr const char* SalvageCancelled           = "salvage.operation.cancelled";
    constexpr const char* SalvageCollected           = "salvage.materials.collected";
    constexpr const char* SalvageTierChanged         = "salvage.tier.changed";

    // Fleet command events
    constexpr const char* FleetOrderIssued           = "fleet.order.issued";
    constexpr const char* FleetOrderCompleted        = "fleet.order.completed";
    constexpr const char* FleetOrderCancelled        = "fleet.order.cancelled";
    constexpr const char* FleetMemberAdded           = "fleet.member.added";
    constexpr const char* FleetMemberRemoved         = "fleet.member.removed";

    // Post-processing events
    constexpr const char* PostProcessEffectEnabled   = "rendering.postprocess.effect_enabled";
    constexpr const char* PostProcessEffectDisabled  = "rendering.postprocess.effect_disabled";
    constexpr const char* PostProcessPresetApplied   = "rendering.postprocess.preset_applied";

    // Shadow events
    constexpr const char* ShadowQualityChanged       = "rendering.shadow.quality_changed";
    constexpr const char* ShadowMapInvalidated       = "rendering.shadow.map_invalidated";
    constexpr const char* ShadowLightAdded           = "rendering.shadow.light_added";
} // namespace GameEvents

} // namespace subspace
