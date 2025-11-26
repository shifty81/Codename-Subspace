# Core Gameplay Mechanics - Codename:Subspace

> Inspired by [Avorion](https://www.avorion.net/) - A cooperative space sandbox and action RPG game where you explore a procedurally generated galaxy, build custom ships out of dynamically scalable blocks, and fight, trade, or mine your way to the galactic core.

## Overview

Codename:Subspace is a game engine designed to implement Avorion-style gameplay mechanics. This document outlines the core gameplay systems and design philosophy that guide development.

---

## Implementation Status

All core gameplay mechanics from the Avorion-inspired design have been implemented and verified:

| Feature | Status | Verified |
|---------|--------|----------|
| Entity-Component System (ECS) | ✅ Complete | 32/32 tests pass |
| Voxel-Based Building | ✅ Complete | Block placement, materials, physics |
| Exploration & Progression | ✅ Complete | Galaxy generation, zone progression |
| Combat System | ✅ Complete | Weapons, shields, damage |
| Mining & Salvaging | ✅ Complete | Resource extraction |
| Trading & Economy | ✅ Complete | Dynamic pricing, production chains |
| Fleet Management (RTS Mode) | ✅ Complete | Captain automation, orders |
| AI System | ✅ Complete | States, perception, decisions |
| Faction System | ✅ Complete | Politics, policies, relations |
| Multiplayer Server | ✅ Complete | TCP infrastructure |
| Navigation & Hyperdrive | ✅ Complete | Sector jumps |
| Save/Load Persistence | ✅ Complete | JSON serialization |

**Run `dotnet run` and select option 17 to verify all systems.**

---

## Core Gameplay Pillars

### 1. Voxel-Based Building

A central feature is the highly customizable, voxel-based ship and station building system. Players can create any design they imagine, limited only by their creativity and the materials they gather.

**Key Features:**
- **Dynamic Block Scaling**: Blocks can be scaled in all three axes for precise designs
- **Functional Blocks**: Each block type serves a purpose (engines, generators, weapons, etc.)
- **Material Progression**: 7 material tiers with improving stats
- **Real-time Construction**: Build and modify ships on the fly
- **Physics Integration**: Ship mass, center of mass, and inertia calculated from blocks

**Implemented Systems:**
- `VoxelBlock` - Individual block representation with position, size, and material
- `VoxelStructureComponent` - Ship/station structure containing blocks
- `BuildSystem` - Handles block placement, removal, and validation
- Material types: Iron, Titanium, Naonite, Trinium, Xanion, Ogonite, Avorion

### 2. Exploration & Progression

Starting at the edge of the galaxy, the primary goal is to journey toward the center, where challenges are greater but rewards (better materials and weapons) are better.

**Key Features:**
- **Procedural Galaxy**: 1000×1000 sector map with deterministic generation
- **Distance-Based Difficulty**: Zones get harder as you approach the galactic core
- **Material Unlocks**: Higher-tier materials available closer to the center
- **Sector Variety**: Asteroids, stations, enemies, and special encounters
- **In-Game Guidance**: Intuitive design minimizing need for external guides

**Progression Zones:**
| Zone | Distance from Core | Materials | Difficulty |
|------|-------------------|-----------|------------|
| Rim | 350+ sectors | Iron | 1.0x |
| Frontier | 250-350 sectors | Titanium | 1.3x |
| Outer Regions | 150-250 sectors | Naonite | 1.6x |
| Mid-Galaxy | 75-150 sectors | Trinium | 2.0x |
| Core Sectors | 50-75 sectors | Xanion | 2.5x |
| Inner Core | 25-50 sectors | Ogonite | 3.0x |
| Galactic Core | 0-25 sectors | Avorion | 4.0x+ |

### 3. Sandbox Freedom

Players choose their playstyle without restrictions. The game supports multiple viable paths to success:

#### Trader Playstyle
Build a trading empire through commerce and industry.

**Activities:**
- Find profitable trade routes between stations
- Establish factories and production chains
- Create passive income through automated trading
- Build large cargo vessels for maximum profit

**Implemented Systems:**
- `TradingSystem` - Buy/sell mechanics with dynamic pricing
- `EconomySystem` - Station production chains and supply/demand
- `TraderComponent` - Trading capabilities for entities

#### Miner/Scavenger Playstyle
Gather resources from the galaxy to fund operations.

**Activities:**
- Extract resources from asteroids
- Salvage ship wrecks for materials
- Process raw materials into refined goods
- Build specialized mining fleets

**Implemented Systems:**
- `MiningSystem` - Resource extraction from asteroids
- `MiningComponent` - Mining capabilities and equipment
- `SalvagingSystem` - Wreck salvaging mechanics
- Multiple resource types with varying rarities

#### Pirate Playstyle
Take what you want through force and cunning.

**Activities:**
- Raid freighters for cargo
- Smuggle illegal goods
- Become a headhunter for bounties
- Build fast attack ships

**Implemented Systems:**
- `PirateComponent` - Piracy tracking and reputation
- Combat mechanics for ship boarding and capture
- Black market and smuggling goods
- Faction hostility management

#### Warlord Playstyle
Build military power and dominate through force.

**Activities:**
- Build powerful battleship fleets
- Wage war on enemy factions
- Capture territory and stations
- Lead massive fleet battles

**Implemented Systems:**
- `CombatSystem` - Weapons, shields, and damage
- `FleetManagementSystem` - Fleet organization
- `FactionSystem` - Faction relations and warfare
- Multiple weapon types and combat tactics

### 4. Fleet Management (RTS Mode)

Once you hire captains, you can automate tasks for your ships and command your fleet from a strategic perspective, effectively turning the game into a real-time strategy experience.

**Key Features:**
- **Captain System**: Hire captains to automate individual ships
- **Ship Automation**: Assign mining, trading, patrol, and attack orders
- **Fleet Commands**: Give orders to entire fleets
- **Strategic View**: Top-down fleet management interface
- **Passive Operations**: Ships generate resources while you're busy elsewhere

**Captain Orders:**
| Order | Description | Specialization Bonus |
|-------|-------------|---------------------|
| Mine | Automatically mine asteroids | +50% with Miner captain |
| Salvage | Collect and salvage wreckage | +50% with Salvager captain |
| Trade | Execute trading routes | +30% with Trader captain |
| Patrol | Defend an area | +25% with Commander captain |
| Attack | Engage hostile targets | +25% with Commander captain |
| Escort | Protect another ship | +25% with Commander captain |
| Scout | Explore new sectors | +40% with Explorer captain |

**Implemented Systems:**
- `FleetManagementSystem` - Fleet organization and control
- `FleetAutomationSystem` - Captain hiring and orders
- `Captain` - Captain entity with specializations and leveling
- `AutomationComponent` - Ship automation state
- Strategy grid for RTS-style management

### 5. Cooperative Multiplayer

The game supports co-op multiplayer, allowing you and friends to team up for various activities.

**Multiplayer Activities:**
- Build stations together
- Fight pirates and hostile factions
- Engage in PvP battles
- Share fleet control and resources
- Explore the galaxy cooperatively

**Implemented Systems:**
- `GameServer` - TCP-based server infrastructure
- `ClientConnection` - Client connection management
- `SectorServer` - Per-sector multiplayer handling
- `NetworkMessage` - Communication protocol
- Faction system for multiplayer groups

---

## Gameplay Loop

The core gameplay loop follows a satisfying progression cycle:

```
┌─────────────────────────────────────────────────────────────┐
│                                                             │
│   ACQUIRE          BUILD           AUTOMATE       PROGRESS  │
│   Resources   →    Ships      →    Fleet     →   to Core   │
│                                                             │
│   Mining           Better          Captain       Harder     │
│   Trading          Design          Orders        Content    │
│   Combat           Materials       Fleets        Rewards    │
│                                                             │
└─────────────────────────────────────────────────────────────┘
                           ↑                            │
                           │                            │
                           └────── REPEAT AT ───────────┘
                                  GREATER SCALE
```

### Early Game (Iron/Titanium Zones)
1. Start at the galaxy rim with a basic Iron ship
2. Mine asteroids and trade to earn credits
3. Build better mining/trading ships
4. Upgrade to Titanium materials
5. Explore toward the galaxy center

### Mid Game (Naonite/Trinium Zones)
1. Unlock shields with Naonite technology
2. Engage in combat for better loot
3. Build a small fleet of specialized ships
4. Unlock jump drives for faster travel
5. Establish trade routes and production

### Late Game (Xanion/Ogonite Zones)
1. Hire captains to automate ships
2. Build an empire with multiple automated fleets
3. Establish station networks and production chains
4. Push toward the galactic core
5. Prepare for endgame content

### Endgame (Avorion Zone)
1. Reach the galactic core
2. Mine the ultimate material: Avorion
3. Fight faction bosses and AI threats
4. Build the most powerful ships
5. Command a vast automated empire

---

## Technical Implementation

### Entity-Component System
All gameplay elements use the ECS architecture:
- **Entities**: Game objects (ships, stations, asteroids)
- **Components**: Data containers (physics, inventory, combat)
- **Systems**: Logic processors (physics, AI, combat)

### Key Systems Reference

| System | Purpose | Related Components |
|--------|---------|-------------------|
| `PhysicsSystem` | Newtonian physics simulation | `PhysicsComponent` |
| `CombatSystem` | Weapon firing and damage | `CombatComponent` |
| `MiningSystem` | Resource extraction | `MiningComponent` |
| `TradingSystem` | Buy/sell mechanics | `TraderComponent` |
| `BuildSystem` | Ship construction | `BuildModeComponent` |
| `FleetManagementSystem` | Fleet control | `FleetMemberComponent` |
| `FleetAutomationSystem` | Captain automation | `AutomationComponent` |
| `AISystem` | NPC behavior | `AIComponent` |
| `NavigationSystem` | Hyperdrive jumps | `HyperdriveComponent` |
| `EconomySystem` | Production chains | Station entities |
| `FactionSystem` | Politics and relations | `FactionComponent` |

---

## Design Philosophy

### Emergent Gameplay
The game creates emergent gameplay through the interaction of simple systems:
- Combat creates wreckage → Salvagers profit
- Trading moves goods → Price fluctuations create opportunities
- Mining depletes asteroids → Competition for resources
- Faction relations shift → New allies and enemies

### Player Agency
Players are never locked into a single path:
- Switch playstyles freely
- Combine approaches (trader who hires combat escorts)
- Scale up or down as desired
- Multiple valid strategies to succeed

### Progression Through Content
New capabilities unlock as players progress:
- Shields (Naonite zone)
- Jump drives (Trinium zone)  
- Captain automation (Ogonite zone)
- Ultimate materials (Avorion zone)

### Skill and Strategy Matter
Success requires both:
- **Skill**: Ship building, combat maneuvering, efficient mining
- **Strategy**: Fleet composition, trade routes, faction relations

---

## Related Documentation

- [GAMEPLAY_LOOP_GUIDE.md](GAMEPLAY_LOOP_GUIDE.md) - Detailed progression and automation
- [FEATURES.md](FEATURES.md) - Complete feature implementation details
- [BUILDING_GUIDE.md](BUILDING_GUIDE.md) - Ship building mechanics
- [AI_SYSTEM_GUIDE.md](AI_SYSTEM_GUIDE.md) - AI behavior and tactics
- [FLEET_ROLES_SYSTEM.md](FLEET_ROLES_SYSTEM.md) - Fleet management details
- [SUBSYSTEM_FLEET_GUIDE.md](SUBSYSTEM_FLEET_GUIDE.md) - Subsystems and upgrades

---

## Acknowledgments

This project is inspired by [Avorion](https://www.avorion.net/) developed by Boxelware. Avorion is a cooperative space sandbox and action RPG game that pioneered many of the gameplay concepts implemented here.

**Note:** This project is not affiliated with, endorsed by, or connected to Boxelware or the official Avorion game. This is a fan-made educational implementation.

---

**Document Version:** 1.0  
**Last Updated:** November 2025  
**Status:** Active Development
