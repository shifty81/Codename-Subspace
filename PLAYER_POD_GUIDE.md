# Player Pod System Guide

## Overview

The Player Pod system is the core character mechanic in AvorionLike. The pod functions as your playable character - a multi-purpose utility ship that can operate independently or dock into larger ships to pilot them.

## Key Features

### 1. Pod as Character
- The player IS the pod - it's your character in the game
- Visually appears as a single-block ship
- Has all necessary ship systems built-in
- Can operate independently as a small utility ship

### 2. Base Efficiency
- **50% efficiency** compared to built ships (0.5x multiplier)
- Base stats:
  - Thrust Power: 50 N (effective: 25 N at base efficiency)
  - Power Generation: 100 W (effective: 50 W at base efficiency)
  - Shield Capacity: 200 (effective: 100 at base efficiency)
  - Torque: 20 Nm (effective: 10 Nm at base efficiency)

### 3. Upgrade System
The pod has **5 upgrade slots** for equipping rare upgrades found throughout the galaxy.

#### Upgrade Types:
- **ThrustBoost**: Increases thrust power
- **ShieldBoost**: Increases shield capacity
- **PowerBoost**: Increases power generation
- **EfficiencyBoost**: Reduces the efficiency penalty (can reach up to 100% efficiency)
- **ExperienceBoost**: Increases experience gain multiplier
- **SkillBoost**: Grants additional skill points per level

#### Rarity System:
Upgrades have a rarity rating from 1-5, with higher rarities providing better bonuses.

### 4. Progression System
- The pod levels up like an RPG character
- Gains experience from activities
- Each level grants skill points
- Level bonuses considerably affect any ship the pod is piloting
- **Level Bonus Formula**: +5% to all stats per level when docked

### 5. Docking System

#### Ship Requirements:
Ships must have a **PodDocking** block type to accommodate the pod.

#### Docking Benefits:
When docked to a ship:
- Pod's inherent stats are added to ship stats
- Pod's level provides a percentage bonus to all ship systems
- Pod upgrades enhance the ship's capabilities
- The combination creates a considerably more powerful vessel

#### Example Bonuses:
```
Level 2 Pod with 3 upgrades:
- Ship thrust: +24.1% improvement
- Additional thrust from pod: +45 N
- Additional shields from pod: +180
- Additional power from pod: +60 W
- Level bonus: +10% to all ship stats
```

## Usage

### Creating a Pod
```csharp
var pod = entityManager.CreateEntity("Player Pod");
var podComponent = new PlayerPodComponent
{
    EntityId = pod.Id,
    BaseEfficiencyMultiplier = 0.5f,
    MaxUpgradeSlots = 5
};
entityManager.AddComponent(pod.Id, podComponent);
```

### Equipping Upgrades
```csharp
var upgrade = new PodUpgrade(
    "Advanced Thruster Module",
    "Increases thrust power by 25N",
    PodUpgradeType.ThrustBoost,
    25f,
    3 // Rarity
);
podComponent.EquipUpgrade(upgrade);
```

### Docking to a Ship
```csharp
// Ship needs a DockingComponent
var dockingComponent = new DockingComponent
{
    EntityId = shipId,
    HasDockingPort = true
};
entityManager.AddComponent(shipId, dockingComponent);

// Dock the pod
bool success = podDockingSystem.DockPod(podId, shipId);

// Get effective stats
var stats = podDockingSystem.GetEffectiveShipStats(shipId);
```

## Future Enhancements

The pod system is designed to be extensible. Future additions may include:

1. **More Upgrade Types**: Weapon bonuses, cargo capacity, jump range
2. **Skill Trees**: Specialized progression paths
3. **Pod Customization**: Visual customization options
4. **Multiple Pods**: Managing a fleet of specialized pods
5. **Pod Trading**: Trading or selling pods with rare upgrades
6. **Pod Abilities**: Active abilities the pod can use
7. **Enhanced Docking**: Multiple docking modes (pilot, passenger, storage)

## Game Design Philosophy

The pod system creates a unique progression mechanic where:
- **The player character has tangible in-game form** (not abstract)
- **Character progression directly impacts gameplay** (level bonuses)
- **Rare loot has meaningful purpose** (upgrades)
- **Ship building integrates with character** (docking port requirement)
- **Risk/reward is balanced** (pod can be destroyed, upgrades lost)

This makes every ship you pilot feel personal and every upgrade you find valuable, while maintaining the core Avorion-like ship building experience.

## Demo

To see the pod system in action:
1. Run the game
2. Select option **12. Player Pod Demo - Character System**
3. The demo will showcase:
   - Pod creation
   - Base stats at 50% efficiency
   - Equipping upgrades
   - Level up system
   - Creating a ship with docking port
   - Docking the pod
   - Stat bonuses from pod and levels

## Technical Details

### Components
- **PlayerPodComponent**: Core pod data and functionality
- **DockingComponent**: Ships that can accept pods
- **ProgressionComponent**: Level, XP, and skill points
- **PodUpgrade**: Individual upgrade data

### Systems
- **PodDockingSystem**: Manages docking logic and stat calculations

### Block Types
- **PodDocking**: Special block type for pod docking ports
