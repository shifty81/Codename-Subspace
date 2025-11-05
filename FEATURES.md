# Avorion-Like Features Implementation

This document details the implementation of all major Avorion-inspired gameplay systems.

## 1. Voxel-Based Ship Building and Customization ✅

### Block Types (9 Types)
The game features a comprehensive block system with functional and structural blocks:

#### Structural Blocks
- **Hull** - Basic ship structure
- **Armor** - Enhanced protection

#### Functional Blocks
- **Engine** - Provides linear thrust for forward/backward movement
- **Thruster** - Omnidirectional movement (strafing, braking)
- **GyroArray** - Rotational control (pitch, yaw, roll)
- **Generator** - Power generation for ship systems
- **ShieldGenerator** - Shield capacity and regeneration

#### System Blocks
- **TurretMount** - Mounting point for weapons
- **HyperdriveCore** - Enables jumping between sectors
- **Cargo** - Storage capacity
- **CrewQuarters** - Housing for crew members

### Material Progression System (7 Tiers)

Materials are tiered based on distance from the galaxy core. Each tier offers improved stats:

| Material | Tech Level | Durability | Mass | Energy | Shield | Color |
|----------|-----------|------------|------|--------|--------|-------|
| **Iron** | 1 (500+ sectors from core) | 1.0x | 1.0x | 0.8x | 0.5x | Gray |
| **Titanium** | 2 (400-500 sectors) | 1.5x | 0.9x | 1.0x | 0.8x | Blue |
| **Naonite** | 3 (300-400 sectors) | 2.0x | 0.8x | 1.2x | 1.2x | Green |
| **Trinium** | 4 (200-300 sectors) | 2.5x | 0.6x | 1.5x | 1.5x | Turquoise |
| **Xanion** | 5 (100-200 sectors) | 3.0x | 0.5x | 1.8x | 2.0x | Gold |
| **Ogonite** | 6 (50-100 sectors) | 4.0x | 0.4x | 2.2x | 2.5x | Orange |
| **Avorion** | 7 (0-50 sectors, core) | 5.0x | 0.3x | 3.0x | 3.5x | Purple |

### Build System Features

- **Build Mode Sessions** - Enter build mode to construct and modify ships
- **Block Placement** - Place blocks with customizable size (scalable in all 3 axes)
- **Material Costs** - Blocks cost materials to place (10 units per cubic unit of volume)
- **Collision Detection** - Prevents overlapping blocks
- **Undo System** - Undo up to 50 block placements
- **Block Removal** - Remove blocks with 50% material refund
- **Material Selection** - Choose from 7 material tiers
- **Block Type Selection** - Choose from 9 functional block types

### Physics Mechanics

#### Mass and Inertia
- **Total Mass** - Calculated from all blocks (volume × material mass multiplier)
- **Center of Mass** - Automatically calculated from weighted block positions
- **Moment of Inertia** - Realistic rotational physics based on mass distribution

#### Maneuverability
- **Linear Thrust** - Engines provide forward/backward thrust (50 N per cubic unit × material efficiency)
- **Omnidirectional Thrust** - Thrusters enable strafing and braking (30 N per cubic unit)
- **Rotational Control** - Gyro arrays provide torque for rotation (20 Nm per cubic unit)
- **Proper Placement** - Block placement relative to center of mass affects handling

### Destructible Ships

- **Block Damage** - Individual blocks can take damage and be destroyed
- **Damage Radius** - Explosive damage affects blocks within a radius with falloff
- **Structural Integrity** - Ship integrity percentage based on remaining durability
- **Salvage Wreckage** - Destroyed ships become salvageable wreckage with resources

## 2. Exploration of Vast, Procedural Galaxy ✅

### Galaxy Structure
- **1000×1000 Sector Map** - Massive procedurally generated galaxy
- **Deterministic Generation** - Same seed generates same sectors
- **Distance-Based Progression** - Materials and difficulty scale with distance from core

### Hyperdrive System

#### Jump Mechanics
- **Jump Range** - Default 5 sectors, upgradeable
- **Jump Cooldown** - 10 seconds between jumps
- **Charging System** - 5-second charge time before jump executes
- **Cancel Ability** - Can cancel jump during charge

#### Navigation
- **Sector Coordinates** - 3D coordinate system (X, Y, Z)
- **Reachable Sectors** - Visual range calculation for jump targets
- **Tech Level Calculation** - Distance from core determines available materials
- **Jump Range Upgrades** - Spend credits to increase jump range

### Galaxy Features
- **Procedural Asteroids** - 5-20 asteroids per sector with varied resources
- **Random Stations** - 20% chance of station in each sector
- **Station Types** - Mining, Trading, Military, Shipyard, Research

## 3. Diverse Activities and Playstyles ✅

### Combat System

#### Weapons
Six weapon types with different characteristics:
- **Chaingun** - Rapid-fire ballistic weapon
- **Laser** - High-accuracy energy weapon
- **Cannon** - High-damage slow-fire weapon
- **Rocket Launcher** - Explosive area damage
- **Railgun** - Long-range piercing weapon
- **Plasma Gun** - Energy projectile weapon

#### Combat Features
- **Turret System** - Mount turrets on turret mount blocks
- **Manual Control** - Manually aim and fire turrets
- **Auto-Targeting** - AI gunners automatically engage targets
- **Projectile Physics** - Leading required for moving targets
- **Energy Management** - Weapons consume energy
- **Shield System** - Shields absorb damage before hull
- **Shield Regeneration** - 10 shields/second when not taking damage
- **Hull Damage** - Damages individual voxel blocks

### Mining & Salvaging

#### Mining
- **Mining Laser** - Extract resources from asteroids
- **Mining Power** - Resources per second based on equipment
- **Mining Range** - 50 units default range
- **Resource Types** - Iron, Titanium, Naonite, Trinium, Xanion, Ogonite, Avorion
- **Automatic Collection** - Resources go directly to inventory

#### Salvaging
- **Salvage Laser** - Extract materials from wreckage
- **Salvage Power** - 8 resources per second default
- **Salvage Range** - 50 units default range
- **Ship Wreckage** - Destroyed ships become salvageable
- **Component Recovery** - Recover ship parts and materials

### Trading & Economy

#### Station-Based Economy
- **Station Types** - Mine, Refinery, Factory, Shipyard, Trading Post
- **Production Chains** - Stations produce goods from inputs
  - Mines produce raw ore
  - Refineries convert ore to refined materials
  - Factories produce ship parts and components
- **Dynamic Pricing** - Supply and demand affects prices
- **12 Trade Goods** - Raw materials, refined goods, components, consumer goods

#### Trading
- **Buy/Sell System** - Trade with stations
- **Price Fluctuation** - Prices based on station inventory
- **Trade Routes** - Set up automated trading routes
- **Trading Reputation** - Affects prices and deals

### Piracy & Smuggling

#### Piracy
- **Attack Freighters** - Raid cargo ships for loot
- **Stolen Goods** - Track pirated goods
- **Pirate Reputation** - Negative reputation with authorities
- **Hostile Status** - Become wanted by factions

#### Smuggling
- **Illegal Goods** - Some goods illegal in certain sectors
- **Black Markets** - Special stations that buy illegal goods
- **Smuggler Hideouts** - Hidden trading posts for contraband
- **Risk/Reward** - Higher profits but risk of faction penalties

## 4. Fleet Management and NPC Crew ✅

### Crew System

#### Crew Types
Eight crew types with different roles:
- **Engineer** - Improves ship systems efficiency
- **Gunner** - Improves weapon accuracy and damage
- **Pilot** - Improves maneuverability
- **Miner** - Improves mining efficiency
- **Trader** - Improves trading deals
- **Sergeant** - Required for larger crews
- **Lieutenant** - Required for very large crews
- **Captain** - Enables ship automation

#### Crew Management
- **Hiring** - Hire crew at stations
- **Salaries** - Pay monthly crew costs
- **Crew Capacity** - Limited by ship size
- **Skill Levels** - Crew have levels affecting efficiency
- **Crew Quarters** - Requires crew quarters blocks

### Captain System

#### Ship Automation
Captains enable autonomous ship operation:
- **Mine Command** - Automatically mine asteroids
- **Salvage Command** - Automatically salvage wreckage
- **Trade Command** - Execute trade routes autonomously
- **Patrol Command** - Patrol sectors
- **Escort Command** - Escort other ships
- **Attack Command** - Engage hostile targets

#### Captain Features
- **High Cost** - 50,000 credits to hire, 5,000/month salary
- **Sector Independence** - Ship operates in different sectors
- **Multi-tasking** - Player can manage multiple automated ships
- **Passive Income** - Automated ships generate resources/credits

### Fleet Management

#### Fleet System
- **Fleet Creation** - Organize ships into fleets
- **Flagship** - Designate fleet leader
- **Fleet Commands** - Give orders to entire fleet
- **Formation Flight** - Ships maintain formation
- **Shared Resources** - Fleet can share resources

## 5. Co-op Multiplayer ✅

### Server Infrastructure
- **TCP-Based** - Reliable client-server architecture
- **Port 27015** - Default multiplayer port
- **Sector Servers** - Each sector managed independently
- **Multi-threaded** - Scalable for multiple sectors

### Multiplayer Features
- **Shared Galaxy** - All players in same procedural galaxy
- **Faction System** - Join or create factions
- **Shared Fleets** - Faction members share fleet control
- **Cooperative Building** - Build ships together
- **PvP Support** - Optional player-versus-player combat

## Implementation Details

### Components

All features are implemented as ECS components:
- `VoxelStructureComponent` - Ship structure with blocks
- `PhysicsComponent` - Movement and physics
- `CombatComponent` - Weapons and shields
- `MiningComponent` - Mining capabilities
- `SalvagingComponent` - Salvage capabilities
- `CrewComponent` - Crew management
- `AutomationComponent` - Captain automation
- `FleetMemberComponent` - Fleet membership
- `HyperdriveComponent` - Jump capabilities
- `SectorLocationComponent` - Current sector
- `BuildModeComponent` - Build mode session
- `TraderComponent` - Trading capabilities
- `PirateComponent` - Piracy activities

### Systems

All features use these systems:
- `PhysicsSystem` - Newtonian physics simulation
- `CombatSystem` - Weapon firing and damage
- `MiningSystem` - Resource extraction
- `FleetManagementSystem` - Fleet and crew management
- `NavigationSystem` - Hyperdrive jumps
- `BuildSystem` - Ship construction
- `EconomySystem` - Trading and production

## Usage Examples

### Creating a Ship with Functional Blocks

```csharp
var ship = engine.EntityManager.CreateEntity("My Ship");
var structure = new VoxelStructureComponent();

// Core hull (Titanium for better stats)
structure.AddBlock(new VoxelBlock(
    new Vector3(0, 0, 0),
    new Vector3(3, 3, 3),
    "Titanium",
    BlockType.Hull
));

// Add engines for thrust
structure.AddBlock(new VoxelBlock(
    new Vector3(-4, 0, 0),
    new Vector3(2, 2, 2),
    "Iron",
    BlockType.Engine
));

// Add shield generator
structure.AddBlock(new VoxelBlock(
    new Vector3(0, 3, 0),
    new Vector3(2, 2, 2),
    "Titanium",
    BlockType.ShieldGenerator
));

engine.EntityManager.AddComponent(ship.Id, structure);
```

### Starting a Mining Operation

```csharp
var miningSystem = engine.MiningSystem;
var minerComponent = engine.EntityManager.GetComponent<MiningComponent>(shipId);

// Add asteroid to sector
var asteroid = new Asteroid(asteroidData);
miningSystem.AddAsteroid(asteroid);

// Start mining
miningSystem.StartMining(minerComponent, asteroid.Id, shipPosition);
```

### Hiring a Captain for Automation

```csharp
var fleetSystem = engine.FleetManagementSystem;
var captain = new Captain { Name = "John Smith" };

// Hire captain
if (fleetSystem.HireCaptain(shipId, captain, playerCredits))
{
    // Give mining command
    fleetSystem.GiveCaptainCommand(shipId, CaptainCommand.Mine);
}
```

### Jumping to Another Sector

```csharp
var navSystem = engine.NavigationSystem;
var targetSector = new SectorCoordinate(10, 5, 0);

// Start jump charge
if (navSystem.StartJumpCharge(shipId, targetSector))
{
    // Jump will execute after 5 seconds
    // Ship will arrive at target sector
}
```

## Performance Characteristics

- **Entity-Component System** - Efficient for thousands of entities
- **Procedural Generation** - On-demand sector generation
- **Spatial Partitioning** - Optimized collision detection (planned)
- **System Updates** - Delta-time based updates
- **Multi-threading** - Sector servers run independently

## Future Enhancements

- Advanced AI behavior trees
- More complex production chains
- Quest and mission systems
- Faction wars and territory control
- Ship blueprints and templates
- More weapon and equipment types
- Voxel damage visualization
- Advanced rendering (textures, shadows, effects)
