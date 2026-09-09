# EVE Online-Inspired Deep Space Mechanics

## Overview

This implementation brings EVE Online's depth to Codename:Subspace, creating a rich, player-driven universe with complex systems that simulate emergent gameplay. These mechanics work together to create a living, breathing galaxy where player actions have consequences and the universe feels alive even when you're not actively playing.

---

## 1. Wormhole System

### Purpose
Wormholes provide dynamic, unpredictable connections between star systems, creating opportunities for exploration, trade routes, and strategic advantage. Unlike static stargates, wormholes are temporary and have limitations.

### Key Features

#### Wormhole Classes
- **Class 1-3**: Easier systems, allow all ship sizes
- **Class 4**: Medium difficulty, capital ship restrictions begin
- **Class 5-6**: High difficulty, strict mass limits on capitals

#### Wormhole Types
- **Wandering**: Random connections, change destinations periodically
- **Static**: Always lead to the same security type (High-sec, Low-sec, Null-sec)

#### Mechanics
- **Lifetime**: Wormholes naturally decay over 18-48 hours
- **Mass Limit**: Each jump consumes mass; wormhole collapses when depleted
- **Ship Restrictions**: Heavy ships cannot use lower-class wormholes
- **Stability States**: Stable → Destabilizing → Critical → Collapsed

### Usage Example
```csharp
// Create a wormhole system
var wormholeSystem = new WormholeSystem(entityManager, seed);

// Spawn a Class 3 wandering wormhole
var sourceSector = new Vector3(100, 50, 25);
var destSector = new Vector3(-50, 100, -75);
var wormholeId = wormholeSystem.CreateWormhole(
    sourceSector, 
    destSector, 
    WormholeClass.Class3, 
    WormholeType.Wandering
);

// Check if a ship can jump
var wormhole = entityManager.GetComponent<WormholeComponent>(wormholeId);
float shipMass = 150000000f; // 150 million kg
if (wormhole.CanShipJump(shipMass))
{
    wormhole.ProcessJump(shipMass);
    // Ship jumps through, wormhole loses mass and stability
}
```

### Design Philosophy
Inspired by EVE Online's wormhole space (J-Space), this system creates emergent gameplay through temporary connections, forcing players to adapt and explore rather than rely on static routes.

---

## 2. Scanning & Exploration System

### Purpose
Scanning allows players to discover hidden wormholes, anomalies, and other signatures that aren't visible on normal sensors. This creates a skill-based gameplay loop for explorers.

### Key Features

#### Directional Scanner
- Scans in a cone or 360° around your ship
- Detects signatures within range
- Cooldown-based to prevent spam
- Range and resolution stats affect effectiveness

#### Probe Scanner
- Deploy up to 8 probes in space
- Triangulation improves scan accuracy
- Multiple scans gradually reveal signature details
- Scan progress: 0% (unknown) → 100% (warpable)

#### Signature Types
- **Wormholes**: Hidden connections between systems
- **Combat Sites**: Encounter NPCs and loot
- **Data/Relic Sites**: Hacking mini-games for rewards
- **Ships**: Detect other players/NPCs
- **Anomalies**: Special cosmic events

### Usage Example
```csharp
// Create a ship with scanning equipment
var scoutShip = entityManager.CreateEntity("Scout Ship");
var scanner = new ScanningComponent
{
    EntityId = scoutShip.Id,
    DirectionalScannerRange = 14000f,
    ScanResolution = 1.5f,
    ProbeStrength = 1.2f,
    AvailableProbes = 8
};
entityManager.AddComponent(scoutShip.Id, scanner);

// Perform directional scan
var signatures = scanningSystem.PerformDirectionalScan(
    scoutShip.Id, 
    Vector3.UnitX,  // Scan direction
    360f            // Scan angle
);

// Deploy probes for precise scanning
var probePositions = new List<Vector3>
{
    shipPosition + new Vector3(1000, 0, 0),
    shipPosition + new Vector3(-500, 866, 0),
    shipPosition + new Vector3(-500, -866, 0),
    shipPosition + new Vector3(0, 0, 1000)
};
scanningSystem.DeployProbes(scoutShip.Id, probePositions);

// Perform probe scan
var probeSignatures = scanningSystem.PerformProbeScan(scoutShip.Id);
// Each scan improves scan progress toward 100%
```

### Design Philosophy
Scanning rewards player skill and patience. Good probe placement and systematic scanning reveals hidden opportunities, creating a mini-game within the larger exploration gameplay.

---

## 3. CONCORD & Security System

### Purpose
Creates meaningful consequences for player actions in different security zones. High-security space is safe but restrictive; low-security and null-security offer greater rewards but higher risk.

### Key Features

#### Security Zones
- **High-Sec (1.0 - 0.5)**: CONCORD response in seconds, extremely dangerous for criminals
- **Low-Sec (0.4 - 0.1)**: CONCORD responds slowly (~30-60s), some protection
- **Null-Sec (0.0)**: No CONCORD, lawless space
- **Wormhole Space**: No security rating, no CONCORD

#### Security Status
- Ranges from -10.0 (maximum criminal) to +10.0 (maximum lawful)
- Lost through illegal attacks on lawful targets
- Affects docking rights, mission availability, and bounties

#### Flags & Consequences
- **Aggression Flag**: 60 seconds, prevents docking
- **Criminal Flag**: 15 minutes, makes you a legal target
- **CONCORD Target**: Active law enforcement pursuit

### Usage Example
```csharp
var concordSystem = new CONCORDSystem(entityManager);

// Check sector security
var sectorCoords = new Vector3(50, 50, 50);
var securityData = concordSystem.GetSectorSecurity(sectorCoords);
Console.WriteLine($"Security Level: {securityData.SecurityLevel}");
Console.WriteLine($"CONCORD Response: {securityData.GetCONCORDResponseTime()}s");

// Register illegal attack
concordSystem.RegisterIllegalAttack(attackerId, victimId, sectorCoords);

// Check attacker status
var status = entityManager.GetComponent<SecurityStatusComponent>(attackerId);
if (status.IsCONCORDTarget)
{
    Console.WriteLine($"CONCORD arriving in {status.CONCORDResponseTimer}s");
}
```

### Design Philosophy
Inspired by EVE's security system, this creates a risk/reward balance. Players can choose safe but limited high-sec, or dangerous but lucrative null-sec/wormhole space.

---

## 4. NPC Economic Simulation

### Purpose
Creates a living economy driven by autonomous NPC agents. Without thousands of players, NPCs fill the gap by mining, trading, producing, and consuming resources, creating realistic market dynamics.

### Key Features

#### NPC Agent Types
- **Miners**: Extract resources from asteroids
- **Traders**: Buy low, sell high between stations
- **Haulers**: Transport large volumes of goods
- **Producers**: Consume raw materials to create products

#### Economic Mechanics
- NPCs generate supply and demand
- Market prices fluctuate based on NPC activity
- Production chains consume inputs and create outputs
- Autonomous behavior creates emergent trade routes

### Usage Example
```csharp
var economySystem = new EconomySystem(entityManager);
var npcSystem = new NPCEconomicAgentSystem(entityManager, economySystem, seed);

// System automatically spawns NPC agents
npcSystem.Update(deltaTime);

// NPCs mine resources
// NPCs trade between stations
// NPCs produce goods
// Market prices adjust dynamically

// View active agents
var agents = entityManager.GetAllComponents<NPCEconomicAgentComponent>();
foreach (var agent in agents)
{
    Console.WriteLine($"{agent.AgentType}: {agent.Credits} credits");
}
```

### Design Philosophy
Borrowed from X4: Foundations and EVE's market simulation, this creates a background economy that feels alive. Players can participate in or ignore this system, but it always provides opportunities.

---

## 5. Manufacturing & Blueprint System

### Purpose
Allows players to research, copy, and manufacture items from blueprints. Research improves efficiency, creating a long-term progression system.

### Key Features

#### Blueprint Types
- **Blueprint Original (BPO)**: Infinite uses, can be researched
- **Blueprint Copy (BPC)**: Limited runs, cheaper to buy

#### Research
- **Material Efficiency**: Reduces material costs (up to 10% at max)
- **Time Efficiency**: Reduces production time (up to 20% at max)
- Research takes time and points to level up

#### Manufacturing
- Time-based production (minutes to hours)
- Consumes materials based on blueprint requirements
- Facility bonuses can improve efficiency
- Manufacturing queue system

### Usage Example
```csharp
var manufacturingSystem = new ManufacturingSystem(entityManager);

// Create a blueprint
var blueprint = new BlueprintComponent
{
    Name = "Combat Frigate",
    Type = BlueprintType.Ship,
    IsOriginal = true,
    BaseProductionTime = 3600f, // 1 hour
    MaterialRequirements = new Dictionary<ResourceType, int>
    {
        { ResourceType.Iron, 1000 },
        { ResourceType.Titanium, 500 }
    }
};

// Research the blueprint
manufacturingSystem.ResearchMaterialEfficiency(blueprintId, 250);
// Material efficiency improves to level 2-3

// Start manufacturing
bool success = manufacturingSystem.StartManufacturingJob(
    facilityId,
    blueprintId,
    ownerId,
    runs: 5  // Produce 5 units
);
```

### Design Philosophy
Directly inspired by EVE Online's industry system, this adds depth to crafting and creates specialization opportunities. Research creates long-term investment and expertise.

---

## 6. Ship Fitting System

### Purpose
Creates meaningful choices in ship customization through resource constraints. Not all modules can fit on every ship, forcing players to make trade-offs.

### Key Features

#### Resource Constraints
- **Power Grid (MW)**: Required for active modules
- **CPU (tf)**: Required for all modules
- **Capacitor (GJ)**: Energy pool for module activation

#### Module Categories
- **High Slots**: Weapons and utility modules
- **Medium Slots**: Defense and propulsion
- **Low Slots**: Engineering and passive bonuses
- **Rigs**: Permanent modifications (not yet implemented)

#### Module Types
- Weapons (Turrets, Launchers)
- Defense (Shield Boosters, Armor Repairers)
- Propulsion (Afterburners, Micro Warp Drives)
- Electronic Warfare (Sensor Dampeners, Scramblers)
- Engineering (Capacitor Boosters, Power Diagnostics)

### Usage Example
```csharp
var fittingSystem = new FittingSystem(entityManager);

// Create ship with fitting component
var fitting = new FittingComponent
{
    MaxPowerGrid = 1500f,
    MaxCPU = 750f,
    MaxCapacitor = 2000f,
    MaxModuleSlots = 12
};

// Create and fit modules
var shieldBooster = fittingSystem.CreateModule(
    "Large Shield Booster",
    FittingModuleType.ShieldBooster,
    ModuleSlot.Medium,
    powerGrid: 100f,
    cpu: 50f,
    capCost: 150f
);

// Try to fit
if (fitting.CanFitModule(shieldBooster))
{
    fittingSystem.FitModule(shipId, shieldBooster);
}

// Activate module
fittingSystem.ActivateModule(shipId, shieldBooster.ModuleId);

// Validate fitting
var (isValid, errors) = fittingSystem.ValidateFitting(shipId);
```

### Design Philosophy
EVE Online's fitting system is legendary for its depth. This implementation captures the core constraint-based gameplay that makes fitting feel like solving a puzzle.

---

## 7. AI Scanning & Exploration Behavior

### Purpose
NPCs don't just mine and trade—they also explore, scan for wormholes, and investigate new space, making the universe feel alive.

### Key Features

#### Explorer Personality
- Prioritizes scanning and exploration
- Deploys probes systematically
- Investigates wormholes
- Maps cosmic signatures

#### Scanning Behaviors
- Periodic directional scans
- Strategic probe deployment
- Signature investigation
- Wormhole evaluation (risk vs reward)

### Usage Example
```csharp
var aiScanningBehavior = new AIScanningBehavior(
    entityManager, 
    scanningSystem, 
    seed
);

// Update NPC scanning behavior
foreach (var ai in aiComponents)
{
    if (ai.Personality == AIPersonality.Explorer)
    {
        aiScanningBehavior.UpdateScanningBehavior(ai, deltaTime);
    }
}

// NPCs automatically:
// - Perform directional scans
// - Deploy and manage probes
// - Investigate wormholes
// - Record discoveries
```

### Design Philosophy
Inspired by emergent AI in games like X4, this makes NPCs feel like real players exploring the universe, not just background decoration.

---

## Integration & Architecture

### ECS Integration
All systems integrate seamlessly with the existing Entity-Component-System architecture:
- Components store state
- Systems process logic
- Entities link everything together

### Modularity
Each system is independent and can be:
- Enabled/disabled individually
- Extended with new features
- Modified without breaking others

### Performance
- Systems use efficient data structures
- Update loops are optimized
- No blocking operations in critical paths
- Scalable to thousands of entities

---

## Reference Games

These mechanics are inspired by:

1. **EVE Online**
   - Wormhole mechanics and classes
   - CONCORD and security status
   - Manufacturing and blueprints
   - Ship fitting system
   - Scanning with probes

2. **X4: Foundations**
   - NPC economic simulation
   - Background production chains
   - Autonomous agent behavior

3. **Astrox Imperium**
   - Single-player EVE experience
   - Mining and resource gathering
   - Station interactions

4. **Starsector**
   - Fleet management
   - Exploration and discovery
   - Market dynamics

---

## Getting Started

### Quick Test
Run the comprehensive example:
```csharp
EVEInspiredMechanicsExample.Run();
```

This demonstrates all systems working together in a single cohesive demo.

### Gradual Integration
1. Start with wormholes in procedural generation
2. Add scanning equipment to player ships
3. Enable CONCORD in high-sec zones
4. Activate NPC economic agents
5. Introduce manufacturing stations
6. Implement ship fitting requirements

### Recommended Configuration
```csharp
// Enable all systems
var wormholeSystem = new WormholeSystem(entityManager, seed);
var scanningSystem = new ScanningSystem(entityManager);
var concordSystem = new CONCORDSystem(entityManager);
var economySystem = new EconomySystem(entityManager);
var npcEconomySystem = new NPCEconomicAgentSystem(entityManager, economySystem);
var manufacturingSystem = new ManufacturingSystem(entityManager);
var fittingSystem = new FittingSystem(entityManager);

// Update in game loop
wormholeSystem.Update(deltaTime);
scanningSystem.Update(deltaTime);
concordSystem.Update(deltaTime);
economySystem.Update(deltaTime);
npcEconomySystem.Update(deltaTime);
manufacturingSystem.Update(deltaTime);
fittingSystem.Update(deltaTime);
```

---

## Future Enhancements

### Potential Additions
- **Player-owned Structures**: Deployable stations in wormhole space
- **Corporation System**: Player organizations with shared assets
- **Contract System**: Player-to-player trading and hauling contracts
- **Invention**: Tech 2/Tech 3 blueprint creation
- **Planetary Interaction**: Resource extraction from planets
- **Capital Ships**: Massive ships with special mechanics
- **Jump Drives**: Alternative FTL system for capitals
- **Sovereignty Mechanics**: Territory control in null-sec

### Community Contributions
This system is designed to be extended. Contributions welcome for:
- New module types
- Additional NPC behaviors
- More complex manufacturing chains
- Enhanced wormhole mechanics
- Advanced AI strategies

---

## Credits

Implementation inspired by:
- **CCP Games** - EVE Online game design
- **Egosoft** - X4: Foundations economic simulation
- **Momenta Studios** - Astrox Imperium single-player adaptation
- **Fractal Softworks** - Starsector exploration mechanics

Built with love for deep space simulation games. 🚀

---

**Last Updated:** December 2025  
**Version:** 1.0  
**Status:** Production Ready
