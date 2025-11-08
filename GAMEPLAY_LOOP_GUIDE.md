# Gameplay Loop Guide - Codename:Subspace

## Overview

Codename:Subspace implements a complete progression-based gameplay loop inspired by Avorion. The core cycle involves:

1. **Acquire Resources** → Mining, Salvaging, Trading, Combat
2. **Build & Upgrade** → Use resources to improve ships and fleet
3. **Progress** → Move from galaxy rim to center
4. **Automate** → Hire captains to manage fleet operations
5. **Repeat at Greater Scale** → Tackle harder content with better rewards

---

## Material Tier System

### Material Progression

The game features 7 material tiers, each unlocking at different distances from the galactic center:

| Material | Tier | Unlock Distance | Stat Multiplier | Key Features |
|----------|------|----------------|-----------------|--------------|
| **Iron** | 0 | Everywhere | 1.0x | Basic starting material |
| **Titanium** | 1 | < 350 sectors | 1.3x | Improved hull, better weapons |
| **Naonite** | 2 | < 250 sectors | 1.6x | **Unlocks shields**, salvaging |
| **Trinium** | 3 | < 150 sectors | 2.0x | Energy systems, jump drives |
| **Xanion** | 4 | < 75 sectors | 2.5x | Advanced mining, refineries |
| **Ogonite** | 5 | < 50 sectors | 3.0x | Fleet management, **captain automation** |
| **Avorion** | 6 | < 25 sectors | 4.0x | **Endgame**, barrier access, boss fights |

### Using Materials

- **Building**: Use higher-tier materials for better ship stats
- **Upgrading**: Replace lower-tier blocks with higher-tier ones
- **Trading**: Sell excess materials for profit
- **Tech Points**: Higher tiers provide more tech points for research

### Material Colors

Each material has a distinctive color for easy identification:
- **Iron**: Gray
- **Titanium**: Silver-Blue
- **Naonite**: Bright Green
- **Trinium**: Blue
- **Xanion**: Gold
- **Ogonite**: Orange-Red
- **Avorion**: Purple

---

## Galaxy Progression

### Journey to the Center

The galaxy is organized in concentric zones around the galactic core:

```
Galaxy Rim (Iron Zone)         ← Start Here
    ↓ 350+ sectors from center
Frontier (Titanium Zone)
    ↓ 250-350 sectors
Outer Regions (Naonite Zone)
    ↓ 150-250 sectors
Mid-Galaxy (Trinium Zone)
    ↓ 75-150 sectors
Core Sectors (Xanion Zone)
    ↓ 50-75 sectors
Inner Core (Ogonite Zone)
    ↓ 25-50 sectors
Galactic Core (Avorion Zone)   ← Endgame
```

### Zone Properties

Each zone has distinct characteristics:

| Property | Rim | Mid | Core |
|----------|-----|-----|------|
| **Difficulty** | 1.0x | 2.0x | 10.0x |
| **Enemy Spawn Rate** | 1.0x | 1.5x | 3.0x |
| **Loot Quality** | 1.0x | 2.0x | 5.0x |
| **Available Materials** | Iron only | Up to Trinium | All materials |

### Progression Gates

- You can only venture **one tier deeper** than your best material
- Example: With Naonite ship, you can enter Trinium zones but not Xanion zones
- Build better ships to progress further

### Milestones

The system tracks your progression:
- Furthest zone reached
- Sectors explored
- Closest approach to galactic center
- Material tiers unlocked

---

## Fleet Automation with Captains

### Hiring Captains

Once you reach **Ogonite zones**, you can hire captains to automate ship operations:

```csharp
// Hire a captain
var captain = fleetAutomationSystem.HireCaptain(
    shipId: myShip.Id,
    name: "Captain Steel",
    specialization: CaptainSpecialization.Miner
);
```

### Captain Specializations

| Specialization | Best For | Efficiency Bonus |
|----------------|----------|------------------|
| **Miner** | Mining operations | +50% mining speed |
| **Salvager** | Salvaging wreckage | +50% salvage speed |
| **Trader** | Trading runs | +30% profit |
| **Commander** | Combat/patrol | +25% combat effectiveness |
| **Explorer** | Scouting | +40% exploration rewards |
| **Engineer** | Production | +20% production speed |

### Captain Orders

Give orders to captains for automated operations:

```csharp
// Give mining order
fleetAutomationSystem.GiveOrder(
    captain,
    CaptainOrder.Mine,
    targetLocation: asteroidField
);

// Give trading order
fleetAutomationSystem.GiveOrder(
    captain,
    CaptainOrder.Trade,
    tradeResource: ResourceType.Iron
);
```

Available orders:
- **Mine**: Automatically mine asteroids
- **Salvage**: Collect wreckage
- **Trade**: Buy low, sell high
- **Patrol**: Defend an area
- **Attack**: Engage enemies
- **Escort**: Protect another ship
- **Scout**: Explore new sectors
- **Refine**: Deliver to refineries
- **Transport**: Move goods

### Captain Progression

Captains level up through experience (Level 1-10):
- **Level 1**: 100% efficiency
- **Level 5**: 140% efficiency
- **Level 10**: 190% efficiency

Each level increases:
- Efficiency: +10% per level
- Salary: Scales with level

---

## Complete Gameplay Loop Example

### Phase 1: Early Game (Iron/Titanium Zones)

1. **Start**: Begin at galaxy rim with basic Iron ship
2. **Mine**: Collect Iron from asteroids
3. **Build**: Construct mining ship with cargo capacity
4. **Trade**: Sell Iron at stations for credits
5. **Upgrade**: Buy Titanium blocks, improve ship
6. **Progress**: Move toward center to Titanium zones

### Phase 2: Mid Game (Naonite/Trinium Zones)

1. **Shields**: Unlock shield generators in Naonite zone
2. **Combat**: Fight pirates for better loot
3. **Salvage**: Collect materials from wrecks
4. **Fleet**: Build multiple specialized ships
5. **Jump Drives**: Unlock in Trinium zone for faster travel
6. **Automation**: Prepare for captain system

### Phase 3: Late Game (Xanion/Ogonite Zones)

1. **Captains**: Hire captains in Ogonite zones
2. **Automate**: Assign mining, trading, patrol orders
3. **Empire**: Build production chains and stations
4. **Fleet Management**: Command multiple automated ships
5. **Refineries**: Process raw materials efficiently
6. **Push Core**: Prepare for endgame content

### Phase 4: Endgame (Avorion Zone)

1. **Avorion**: Reach galactic core
2. **Ultimate Materials**: Mine Avorion for best stats
3. **Boss Fights**: Encounter faction bosses and AI
4. **Barriers**: Access protected core sectors
5. **Ultimate Ship**: Build the most powerful vessel
6. **Domination**: Command vast automated empire

---

## Tips for Success

### Resource Management
- Always keep mining ships working
- Stockpile materials before pushing to new zones
- Trade excess materials for credits
- Invest in larger cargo capacity

### Ship Building
- Specialize ships (miner, fighter, trader, explorer)
- Match material tier to zone difficulty
- Balance speed vs. armor
- Always include shields (Naonite+)

### Progression Strategy
- Don't rush to core - gear up first
- Match recommended ship value for zone
- Test new zones with scouts first
- Build multiple ships for different roles

### Captain Management
- Hire specialized captains for each ship type
- Level up captains in safe zones first
- Match captain specialization to ship role
- Monitor salary costs vs. profits

### Automation Tips
- Start with 2-3 mining captains
- Add traders once you have production
- Use patrol captains to protect assets
- Scout captains find new opportunities

---

## Progression Checklist

### Beginner Goals
- [ ] Build first mining ship
- [ ] Reach 100k credits
- [ ] Obtain Titanium blocks
- [ ] Explore 20 sectors

### Intermediate Goals
- [ ] Unlock shields (Naonite zone)
- [ ] Build combat ship
- [ ] Defeat first pirate boss
- [ ] Build 5-ship fleet

### Advanced Goals
- [ ] Reach Ogonite zone
- [ ] Hire first captain
- [ ] Automate 3+ ships
- [ ] Establish station network

### Endgame Goals
- [ ] Reach galactic core
- [ ] Obtain Avorion materials
- [ ] Defeat faction boss
- [ ] Build ultimate capital ship
- [ ] Command 10+ automated ships

---

## API Reference

### Material Tier Functions

```csharp
// Check unlock distance
int distance = MaterialTierInfo.GetUnlockDistance(MaterialTier.Avorion); // 25

// Get stat multiplier
float mult = MaterialTierInfo.GetStatMultiplier(MaterialTier.Ogonite); // 3.0f

// Check unlocked features
var features = MaterialTierInfo.GetUnlockedFeatures(MaterialTier.Naonite);
// Returns: Shield Generators, Advanced Weapons, Salvaging, etc.

// Get display color
var (r, g, b) = MaterialTierInfo.GetMaterialColor(MaterialTier.Avorion); // Purple
```

### Galaxy Progression Functions

```csharp
// Calculate distance from center
int distance = GalaxyProgressionSystem.GetDistanceFromCenter(sectorCoord);

// Get available material tier
var tier = GalaxyProgressionSystem.GetAvailableMaterialTier(distance);

// Check zone access
bool canAccess = GalaxyProgressionSystem.CanAccessZone(playerTier, targetDistance);

// Get difficulty
float difficulty = GalaxyProgressionSystem.GetDifficultyMultiplier(distance);
```

### Fleet Automation Functions

```csharp
// Hire captain
var captain = system.HireCaptain(shipId, "Name", CaptainSpecialization.Miner);

// Give order
system.GiveOrder(captain, CaptainOrder.Mine, targetLocation);

// Stop order
system.StopOrder(captain);

// Get efficiency
float efficiency = captain.GetTotalEfficiency(); // Includes spec + level bonuses
```

---

## Integration Example

Here's how to integrate all three systems:

```csharp
// In GameEngine initialization
var galaxyProgression = new GalaxyProgressionSystem(EntityManager);
var fleetAutomation = new FleetAutomationSystem(EntityManager);

EntityManager.RegisterSystem(galaxyProgression);
EntityManager.RegisterSystem(fleetAutomation);

// Create player with progression tracking
var player = EntityManager.CreateEntity("Player Ship");
var progressionComp = new PlayerProgressionComponent
{
    EntityId = player.Id,
    ClosestDistanceToCenter = 400, // Starting at rim
    HighestMaterialTierAcquired = MaterialTier.Iron
};
EntityManager.AddComponent(player.Id, progressionComp);

// Track location
var locationComp = new SectorLocationComponent
{
    EntityId = player.Id,
    CurrentSector = new SectorCoordinate { X = 400, Y = 0, Z = 0 }
};
EntityManager.AddComponent(player.Id, locationComp);

// Later: Hire captain when reaching Ogonite zone
if (progressionComp.AvailableMaterialTier >= MaterialTier.Ogonite)
{
    var captain = fleetAutomation.HireCaptain(
        miningShipId,
        fleetAutomation.GenerateRandomCaptainName(),
        CaptainSpecialization.Miner
    );
    
    fleetAutomation.GiveOrder(captain, CaptainOrder.Mine);
}
```

---

## Conclusion

The gameplay loop creates a satisfying progression cycle:

**Acquire → Build → Automate → Progress → Repeat**

Each tier of materials unlocks new possibilities, and the captain system allows players to scale their empire. The journey from the galaxy's rim to its core provides clear goals and escalating challenges, with endgame content offering the ultimate test of your fleet-building skills.

Start small, grow your fleet, automate operations, and conquer the galaxy!
