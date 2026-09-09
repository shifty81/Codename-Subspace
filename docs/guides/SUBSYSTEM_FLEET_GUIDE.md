# Ship Subsystem & Fleet Management System Guide

## Overview

The Subsystem & Fleet Management System provides deep ship customization, autonomous fleet missions, and crew management. Ships can be equipped with upgrades that drop from enemies, found through salvaging, or crafted at late game. Each ship has a class that determines its role and specialization.

## Table of Contents

1. [Ship Subsystem Upgrades](#ship-subsystem-upgrades)
2. [Ship Classes](#ship-classes)
3. [Fleet Mission System](#fleet-mission-system)
4. [Crew & Pilot System](#crew--pilot-system)
5. [Pod Integration](#pod-integration)
6. [UI Controls](#ui-controls)

---

## Ship Subsystem Upgrades

### What are Subsystems?

Subsystems are upgradeable modules that provide percentage-based stat bonuses to ships and pods. They can be found as loot, salvaged from wrecks, or crafted at late game.

### Subsystem Types

#### Engine Subsystems
- **Thrust Amplifier**: Increases thrust power
- **Maneuvering Thrusters**: Improves maneuverability and torque

#### Shield Subsystems
- **Shield Booster**: Increases shield capacity
- **Shield Regenerator**: Improves shield regeneration rate

#### Weapon Subsystems
- **Weapon Amplifier**: Increases weapon damage
- **Targeting Computer**: Improves accuracy and critical hit chance
- **Cooling System**: Improves fire rate

#### Power Subsystems
- **Power Amplifier**: Increases power generation
- **Power Efficiency**: Reduces power consumption
- **Capacitor**: Increases power storage

#### Defense Subsystems
- **Armor Plating**: Reduces incoming damage
- **Structural Reinforcement**: Increases hull durability

#### Utility Subsystems
- **Cargo Expansion**: Increases cargo capacity
- **Scanner Array**: Improves scanner range
- **Jump Drive Enhancer**: Reduces jump cooldown

#### Special Subsystems (Rare)
- **Experience Accelerator**: Increases XP gain (pod only)
- **Efficiency Core**: Reduces efficiency penalty (pod only)
- **Omni Core**: Small bonus to all stats (legendary drops)

### Rarity System

Subsystems come in 5 rarity tiers affecting drop rates and bonuses:

| Rarity    | Drop Rate | Stat Bonus | Color  |
|-----------|-----------|------------|--------|
| Common    | 60%       | 5-10%      | Gray   |
| Uncommon  | 25%       | 10-15%     | Green  |
| Rare      | 10%       | 15-25%     | Blue   |
| Epic      | 4%        | 25-35%     | Purple |
| Legendary | 1%        | 35-50%     | Orange |

### Quality Levels

Quality determines upgrade potential:

- **Standard**: No upgrades possible
- **Enhanced**: Can upgrade 1 level
- **Superior**: Can upgrade 2 levels
- **Masterwork**: Can upgrade 3 levels

### Upgrading Subsystems

Subsystems can be upgraded to increase their bonus by 50% of base per level:
- Costs materials based on rarity and current level
- Requires appropriate tech level materials
- Bonus = Base Bonus + (Bonus Per Upgrade × Upgrade Level)

**Example**: A Rare Thrust Amplifier at +20% base
- +0: 20% thrust bonus
- +1: 30% thrust bonus (20% + 10%)
- +2: 40% thrust bonus (20% + 20%)

### Subsystem Slots

- **Ships**: 8 subsystem slots
- **Pods**: 4 subsystem slots
- Storage capacity: 50 subsystems per entity

---

## Ship Classes

Ships have specialized classes that determine their role and capabilities:

### Combat Class
**Role**: Combat superiority and engagement

**Bonuses:**
- +50% weapon damage
- +30% shield capacity
- +40% armor effectiveness
- +20% critical hit chance

**Preferred Subsystems**: Weapon Amplifier, Shield Booster, Armor Plating, Targeting Computer

### Industrial Class
**Role**: Resource gathering and mining

**Bonuses:**
- +80% mining yield
- +50% cargo capacity
- +40% resource detection
- +30% power efficiency

**Preferred Subsystems**: Cargo Expansion, Power Amplifier, Power Efficiency

### Exploration Class
**Role**: Long-range scouting and discovery

**Bonuses:**
- +100% scanner range
- +50% jump range
- -30% jump cooldown
- +40% fuel efficiency

**Preferred Subsystems**: Scanner Array, Jump Drive Enhancer, Power Efficiency

### Salvaging Class
**Role**: Breaking down wrecks for materials

**Bonuses:**
- +100% salvage yield
- +50% salvage speed
- +30% loot quality
- +30% cargo capacity

**Preferred Subsystems**: Cargo Expansion, Power Amplifier

### Covert Class
**Role**: Stealth operations and reconnaissance

**Features:**
- Cloaking capability
- +50% cloak efficiency
- +100% scanner evasion
- +30% thrust power
- +60% detection range

**Preferred Subsystems**: Power Efficiency, Thrust Amplifier, Maneuvering Thrusters

### Specialization

Ships gain specialization XP from successful missions:
- Level up specialization to increase class bonus multiplier
- XP requirement: Level × 1000
- Each level adds +5% to class bonus multiplier

---

## Fleet Mission System

### Mission Types

#### Explore
- **Duration**: 2 hours
- **Preferred Class**: Exploration
- **Rewards**: High XP, scanner subsystems, blueprints
- **Requirements**: Moderate combat rating

#### Mine
- **Duration**: 4 hours
- **Preferred Class**: Industrial
- **Rewards**: Resources, industrial blueprints
- **Requirements**: Large cargo capacity

#### Salvage
- **Duration**: 3 hours
- **Preferred Class**: Salvaging
- **Rewards**: Mixed materials, high subsystem drop rate (60%)
- **Requirements**: Moderate cargo and combat rating

#### Combat
- **Duration**: 2.5 hours
- **Preferred Class**: Combat
- **Rewards**: High credits, combat subsystems (70% drop rate), weapon blueprints
- **Requirements**: High combat rating, 2+ ships

#### Reconnaissance
- **Duration**: 3 hours
- **Preferred Class**: Covert
- **Rewards**: Double credits, rare covert subsystems, cloaking blueprints
- **Requirements**: High combat rating, solo mission

### Mission Difficulty

Missions scale from Easy to Extreme:
- **Easy**: 5-10% stat bonus requirement
- **Normal**: 10-15% stat bonus requirement
- **Hard**: 15-25% stat bonus requirement
- **Very Hard**: 25-35% stat bonus requirement
- **Extreme**: 35-50% stat bonus requirement

### Success Rate Calculation

Success rate is determined by:
1. **Ship Rating**: Based on class match, specialization, subsystems, and stats
2. **Difficulty Penalty**: Each difficulty level reduces success by 10%
3. **Ship Count Bonus**: Multiple ships on combat missions add +10% per extra ship
4. **Final Range**: 10% minimum, 95% maximum

### Mission Rewards

Rewards scale with difficulty and success rate:
- **Credits**: Difficulty × 1000 × Success Rate
- **Experience**: Difficulty × 500 × Success Rate
- **Subsystems**: Drop chance based on mission type
- **Blueprints**: 20-50% chance depending on mission type
- **Resources**: Type-specific based on mission

### Ship Readiness

Ships have mission readiness (0-100%):
- Missions reduce readiness by 20%
- Ships need 50%+ readiness to accept missions
- Readiness restores at 5% per minute when not on mission
- Ships on missions cannot be assigned to new missions

---

## Crew & Pilot System

### Crew Requirements

Ships require crew based on configuration:
- **Base Crew**: Ship size / 100 (minimum 1)
- **Functional Blocks**: 
  - Engines/Thrusters: +1 each
  - Generators/Shields: +2 each
  - Turret Mounts: +3 each
  - Hyperdrive: +5 each

### Crew Efficiency

Crew efficiency affects ship performance:
- **Undermanned**: Efficiency = Current / Minimum (e.g., 5/10 = 50%)
- **Adequately Crewed**: 100% efficiency
- **Overmanned**: +2% per extra crew, max +20%

### Crew Quarters

- Crew Quarters blocks provide housing: 10 crew per block
- Max crew = max(Minimum Crew, Crew Quarters Capacity)

### Pilots

Ships require a pilot to operate (or player pod docked):

#### Pilot Skills
- **Combat Skill**: Affects weapon accuracy (0-100%)
- **Navigation Skill**: Affects maneuverability (0-100%)
- **Engineering Skill**: Affects power efficiency (0-100%)

#### Pilot Progression
- Gain experience from successful missions
- Level up: Level × 500 XP required
- Each level improves a random skill by 2%
- Skills cap at 100%

#### Pilot Specialization
- 30% of pilots have a class specialization
- Specialized pilots get bonus when piloting their class

#### Hiring Pilots
- Available at stations when docked
- Hiring cost: 1000 × Level + 500-2000
- Daily salary: 100 × Level + 50-200
- Can have unemployed pilots in reserve (half salary)

#### Hiring Crew
- Cost: 500 credits per crew member
- Instant hiring (no individual crew management)
- Crew efficiency calculated automatically

### Operational Status

A ship is operational when:
- Has assigned pilot OR player pod is docked
- Has sufficient crew (≥ minimum requirement)

---

## Pod Integration

### Pod as Pilot

When the player pod docks with a ship:
- **Player takes control** of the ship
- Pod overrides pilot requirement
- Pod skills and upgrades affect ship performance
- Hired pilot can be removed while pod is docked

### Pod Stat Bonuses

When docked, the pod provides:
- All pod skills apply to ship
- Pod subsystems add their bonuses
- Pod level provides +5% per level to all stats
- Pod's inherent stats added to ship

### Pod Subsystems

Pods have 4 subsystem slots (vs ships' 8):
- Can equip any subsystem type
- Pod-specific subsystems available:
  - Experience Accelerator
  - Efficiency Core

### Undocking

When pod undocks:
- Ship loses pod bonuses
- Ship requires hired pilot to remain operational
- Pod can fly independently or dock with another ship

---

## UI Controls

### Subsystem Management UI
**Hotkey**: `U` (for Upgrades/sUbsystems)

**Features**:
- View equipped subsystems on ships and pods
- Manage subsystem storage (50 slots)
- Equip/unequip subsystems
- Upgrade subsystems with materials
- View detailed subsystem stats
- Color-coded by rarity

### Fleet Mission UI
**Hotkey**: `M` (for Missions)

**Tabs**:
1. **Available Missions**: Browse and assign ships
2. **Active Missions**: Monitor progress and success rates
3. **Completed Missions**: View results and collect rewards

**Features**:
- Filter missions by type and difficulty
- Assign multiple ships to missions
- View mission requirements and rewards
- Real-time progress tracking
- Success rate preview

### Crew Management UI
**Hotkey**: `C` (for Crew)

**Tabs**:
1. **Ship Management**: Manage crew and pilot for selected ship
2. **Hire Pilots**: Browse and hire pilots from stations
3. **Available Pilots**: Assign unemployed pilots to ships

**Features**:
- View crew requirements and efficiency
- Hire additional crew members
- View pilot skills and specializations
- Assign pilots to ships
- Monitor operational status
- View daily salary costs

---

## Tips & Strategies

### Subsystem Optimization
1. Match subsystems to ship class for maximum benefit
2. Focus on upgrading high-rarity subsystems first
3. Keep diverse subsystems in storage for different situations
4. Omni Cores are rare but valuable for any ship

### Fleet Mission Strategy
1. Assign ships with matching class to missions for better success rates
2. Keep ship readiness high by rotating ships between missions
3. Higher difficulty missions yield better rewards
4. Salvage missions are best for finding new subsystems

### Crew Management
1. Hire pilots with specializations matching your ship classes
2. Keep extra crew for efficiency bonus (up to +20%)
3. Maintain unemployed pilot reserve for flexibility
4. Budget for daily salary costs

### Ship Building
1. Build crew quarters early to support larger crews
2. Match ship class to intended role
3. Install pod docking port for player control option
4. Consider crew requirements when adding functional blocks

---

## Future Enhancements

### Planned Features
- Subsystem crafting system at late game
- Research system for discovering new subsystem types
- Blueprint crafting for specific subsystem designs
- Mission rewards directly into subsystem storage
- Visual indicators on ships showing equipped subsystems
- Fleet formations and combined missions
- Pilot training facilities
- Crew specialization and veterancy

### Balance Adjustments
- Fine-tune subsystem drop rates based on testing
- Adjust mission difficulty scaling
- Balance crew costs and requirements
- Tune pilot skill progression rates

---

## Technical Details

### Component Architecture
- `ShipSubsystemComponent`: Manages ship subsystem slots
- `PodSubsystemComponent`: Manages pod subsystem slots
- `SubsystemInventoryComponent`: Stores unequipped subsystems
- `ShipClassComponent`: Defines ship class and specialization
- `CrewComponent`: Manages crew and pilot
- `FleetMission`: Represents a mission instance
- `FleetMissionSystem`: Handles mission logic
- `CrewManagementSystem`: Handles hiring and assignment

### Data Persistence
All components implement `ISerializable` for save/load:
- Subsystem upgrades and levels
- Ship class and specialization progress
- Crew assignments and pilot data
- Active and completed missions
- Storage inventory

### Performance Considerations
- Subsystem bonuses cached for efficient lookup
- Mission progress updated on fixed intervals
- UI rendering optimized for large fleets
- Pilot skill calculations pre-computed

---

## Troubleshooting

### Ship Won't Accept Mission
- Check if ship has pilot or pod docked
- Verify ship has sufficient crew
- Ensure ship readiness is 50% or higher
- Ship may already be on another mission

### Subsystem Won't Equip
- Check if slot is already filled
- Verify player meets tech level requirement
- Ensure subsystem is in storage, not equipped elsewhere

### Low Mission Success Rate
- Assign ships with matching class
- Improve ship subsystems
- Increase pilot skills through experience
- Add more ships to combat missions

---

## Credits

System designed to provide deep strategic gameplay while maintaining accessibility. Inspired by games like Avorion, X4, and EVE Online's delegation mechanics.

**Version**: 1.0
**Last Updated**: November 2025
