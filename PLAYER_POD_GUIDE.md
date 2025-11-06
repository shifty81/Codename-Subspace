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

### 4. Skill Tree System **NEW** 🎉

The pod features a comprehensive skill tree with **18+ skills** across **5 categories**:

#### Combat Skills
- **Weapon Mastery**: +10% weapon damage per rank (5 ranks)
- **Critical Strike**: +5% critical hit chance per rank (5 ranks)
- **Rapid Fire**: +8% fire rate per rank (5 ranks)

#### Defense Skills
- **Shield Fortification**: +15% shield capacity per rank (5 ranks)
- **Shield Regeneration**: +20% shield regen per rank (5 ranks)
- **Reinforced Armor**: -5% incoming damage per rank (5 ranks)

#### Engineering Skills
- **Advanced Propulsion**: +12% thrust power per rank (5 ranks)
- **Power Optimization**: +15% power generation per rank (5 ranks)
- **System Efficiency**: -5% efficiency penalty per rank (5 ranks)

#### Exploration Skills
- **Enhanced Scanners**: +25% scanner range per rank (5 ranks)
- **Resource Detection**: +10% rare resource find chance per rank (5 ranks)
- **Jump Drive Mastery**: -15% jump cooldown per rank (3 ranks)

#### Leadership Skills
- **Fast Learner**: +10% experience gain per rank (5 ranks)
- **Trade Expertise**: +5% better trade prices per rank (5 ranks)
- **Fleet Commander**: +8% bonus to fleet stats per rank (5 ranks)

**Skill Features**:
- Prerequisites: Some skills require other skills first
- Level Requirements: Higher-tier skills unlock at specific levels
- Skill Points: Earned from leveling up
- Rank-based progression: Each skill can be upgraded multiple times

### 5. Active Abilities System **NEW** ⚡

The pod can equip up to **4 active abilities** from a collection of **8+ unique abilities**:

#### Shield Abilities
- **Shield Overcharge**: +50% shield capacity for 10s (50 energy, 30s cooldown)
- **Emergency Shields**: Instantly restore 30% shields (40 energy, 45s cooldown)

#### Weapon Abilities
- **Weapon Overload**: +75% weapon damage for 8s (60 energy, 40s cooldown)
- **Precision Strike**: Next shot deals 200% damage and crits (30 energy, 25s cooldown)

#### Mobility Abilities
- **Afterburner**: +100% thrust for 5s (35 energy, 20s cooldown)
- **Emergency Warp**: Teleport 500m instantly (70 energy, 60s cooldown)

#### Utility Abilities
- **Energy Drain**: Drain 50 energy from target within 200m (20 energy, 15s cooldown)
- **Scan Pulse**: Reveal all objects within 1000m for 15s (25 energy, 30s cooldown)

**Ability Features**:
- Energy cost management
- Cooldown timers
- Duration-based effects
- Strategic ability selection

### 6. Progression System
- The pod levels up like an RPG character
- Gains experience from activities
- Each level grants skill points
- Level bonuses considerably affect any ship the pod is piloting
- **Level Bonus Formula**: +5% to all stats per level when docked

### 7. Docking System

#### Ship Requirements:
Ships must have a **PodDocking** block type to accommodate the pod.

#### Docking Benefits:
When docked to a ship:
- Pod's inherent stats are added to ship stats
- Pod's level provides a percentage bonus to all ship systems
- Pod upgrades enhance the ship's capabilities
- **Pod skills provide permanent passive bonuses**
- **Pod abilities provide temporary power-ups**
- The combination creates a considerably more powerful vessel

#### Example Bonuses:
```
Level 5 Pod with 5 skills learned, 3 upgrades, and active abilities:
- Ship thrust: +65% improvement from level and skills
- Additional thrust from pod: +75 N
- Additional shields from pod: +350 (or +525 with Shield Overcharge active)
- Additional power from pod: +115 W
- Level bonus: +25% to all ship stats
- Weapon damage: +20% from Weapon Mastery skill
- Critical hit chance: 10% from Critical Strike skill
```

### 8. Loot System Integration **NEW** 💎

Pod upgrades and abilities can be found as loot drops:

#### Pod Upgrade Drops
- **15% drop chance** from enemies
- Rarity scales with enemy level
- Higher rarity = better bonuses
- Named upgrades (Basic/Improved/Advanced/Superior/Legendary)

#### Ability Unlock Drops
- **5% drop chance** from level 5+ enemies
- Very rare but powerful
- Unlock new abilities to equip
- Build your ultimate ability loadout

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

// Add skill tree
var skillTree = new PodSkillTreeComponent { EntityId = pod.Id };
entityManager.AddComponent(pod.Id, skillTree);

// Add abilities
var abilities = new PodAbilitiesComponent { EntityId = pod.Id };
entityManager.AddComponent(pod.Id, abilities);
```

### Learning Skills
```csharp
int skillPoints = progressionComponent.SkillPoints;
bool learned = skillTree.LearnSkill("combat_weapon_damage", 
                                     progressionComponent.Level, 
                                     ref skillPoints);
progressionComponent.SkillPoints = skillPoints;
```

### Equipping Abilities
```csharp
abilitiesComponent.EquipAbility("shield_overcharge");
abilitiesComponent.EquipAbility("afterburner");
abilitiesComponent.EquipAbility("overload_weapons");
```

### Using Abilities
```csharp
float availableEnergy = podComponent.GetTotalPowerGeneration(skillTree);
bool used = abilitiesComponent.UseAbility("afterburner", availableEnergy);
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

// Get effective stats (includes all bonuses)
var stats = podDockingSystem.GetEffectiveShipStats(shipId);
```

### Generating Loot
```csharp
// Generate loot with pod upgrades
var loot = lootSystem.GenerateLoot(enemyLevel, includePodLoot: true);

// Process loot and collect upgrades
var foundUpgrades = lootSystem.ProcessLootWithPodUpgrades(loot, inventory);

// Equip found upgrades
foreach (var upgrade in foundUpgrades)
{
    if (podComponent.EquipUpgrade(upgrade))
    {
        Console.WriteLine($"Equipped: {upgrade.Name}");
    }
}
```

## Future Enhancements

The pod system is designed to be extensible. Future additions may include:

1. **More Skills**: Specialized skills for different playstyles
2. **Skill Specializations**: Choose between different skill tree paths
3. **More Abilities**: Expand ability pool with unique effects
4. **Pod Customization**: Visual customization options
5. **Multiple Pods**: Managing a fleet of specialized pods
6. **Pod Trading**: Trading or selling pods with rare upgrades
7. **Enhanced Docking**: Multiple docking modes (pilot, passenger, storage)
8. **Ability Crafting**: Combine abilities to create new ones

## Game Design Philosophy

The pod system creates a unique progression mechanic where:
- **The player character has tangible in-game form** (not abstract)
- **Character progression directly impacts gameplay** (level, skills, abilities)
- **Rare loot has meaningful purpose** (upgrades and ability unlocks)
- **Ship building integrates with character** (docking port requirement)
- **Risk/reward is balanced** (pod can be destroyed, upgrades lost)
- **Deep customization** (skill trees, abilities, upgrades combine for unique builds)

This makes every ship you pilot feel personal, every upgrade you find valuable, every skill you learn meaningful, and every ability you unlock exciting, while maintaining the core Avorion-like ship building experience.

## Demo

To see the pod system in action:
1. Run the game
2. Select option **12. Player Pod Demo - Character System** (basic pod system)
3. Select option **13. Enhanced Pod Demo - Skills & Abilities [NEW]** (full feature showcase)

The enhanced demo showcases:
- Pod creation with all components
- Skill learning and progression
- Ability equipping and activation
- Combined stat bonuses
- Docking mechanics with full bonuses
- Path to becoming unstoppable force

## Technical Details

### Components
- **PlayerPodComponent**: Core pod data and functionality
- **PodSkillTreeComponent**: Skill tree management
- **PodAbilitiesComponent**: Active abilities management
- **DockingComponent**: Ships that can accept pods
- **ProgressionComponent**: Level, XP, and skill points

### Systems
- **PodDockingSystem**: Manages docking logic and stat calculations
- **PodAbilitySystem**: Updates ability cooldowns and states

### Block Types
- **PodDocking**: Special block type for pod docking ports
