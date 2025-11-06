# Multi-Layer Gameplay Architecture
## Combining Avorion + Elite Dangerous + X4 Foundations + Stellaris + EVE Online

This document outlines the architectural plan to transform Codename:Subspace into a comprehensive multi-layer space game that integrates mechanics from five legendary space games.

## Executive Summary

**Vision**: Create a multi-layered space game where players can transition from first-person piloting to commanding a galaxy-spanning empire. The game uses **sector-based instances with load screens** between sectors for optimal performance and gameplay.

**Important Note**: This is **NOT an open world game**. The galaxy is divided into **solar systems**, where:
- **Each solar system is a separate map/scene/instance**
- All planets, stations, asteroids, and ships in that system are loaded when you enter
- **Load screens occur when jumping between solar systems** (hyperspace/warp jumps)
- **Within a solar system**: Seamless transitions between cockpit → ship → fleet views (no loading)
- This approach matches Avorion and X4's design philosophy

**Benefits of This Design**:
- Better performance optimization per system
- Enables detailed simulation within each system
- Manageable server load (one system per server instance)
- Allows for large, detailed solar systems
- Clear boundaries for multiplayer instances
- Predictable memory usage

**Current Status**: 
- ✅ Avorion-style voxel building system (IMPLEMENTED)
- ✅ Basic physics and entity system (IMPLEMENTED)
- ✅ Stellaris-style faction system (IMPLEMENTED)
- ⚠️ Elite Dangerous mechanics (PLANNED)
- ⚠️ X4 Foundations economy (PLANNED)
- ⚠️ EVE Online persistence (PLANNED)

## Four Layers of Gameplay

**System-Based Design**: 
- **Between Solar Systems**: Load screens when jumping/warping (hyperspace travel)
- **Within a Solar System**: Seamless transitions between cockpit, ship, and fleet views
- Each system contains: Star(s), planets, moons, asteroid belts, stations, AI ships
- Galaxy map shows all systems; clicking one initiates jump with loading screen

### Layer 1: Local/Tactical - Elite Dangerous Infusion
**Focus**: Ship movement, combat, mining, building, immersive piloting **within a solar system**

**Key Features to Implement**:
```
Core/SolarSystem/
├── SolarSystemManager.cs        # Manages current loaded solar system
├── SystemScene.cs               # Represents a complete solar system instance
├── HyperspaceJump.cs            # Handles jumps between systems with loading
├── HyperspaceAnimation.cs       # Animated hyperspace tunnel during load ✨ NEW
├── LoadingTipManager.cs         # Manages gameplay tips during loading ✨ NEW
├── SystemLoader.cs              # Loads/unloads system data and entities
└── SystemBoundary.cs            # Defines system edges and jump points

Core/Flight/
├── FlightModel.cs               # Physics-based flight mechanics
├── CockpitSystem.cs             # First-person cockpit experience
├── FlightAssist.cs              # Optional flight assistance
├── ManeuveringThrusters.cs      # 6-DOF movement control
└── InertialDampeners.cs         # Momentum and drift simulation

Core/Scale/
├── GalaxyMap.cs                 # Overview of all solar systems
├── SystemGeneration.cs          # Procedural solar system generation
├── OrbitalMechanics.cs          # Realistic orbital physics within system
└── SystemCoordinates.cs         # Local coordinate system per solar system
```

**Implementation Priority**: HIGH
- Elite Dangerous flight model provides the "feel" and immersion
- Solar system instances keep scope manageable
- Seamless first-person to third-person transitions within system

**Technical Challenges**:
- Loading/unloading systems efficiently
- Saving system state when player leaves
- Handling AI ships across system boundaries
- Smooth hyperspace jump transitions with loading screen

### Layer 2: System/Operational - X4 Foundations Integration
**Focus**: Trade routes, station management, local fleet commands, living economy

**Key Features to Implement**:
```
Core/Economy/
├── EconomySimulator.cs          # Living, autonomous economy
├── SupplyChain.cs               # Production chains
├── MarketPrices.cs              # Dynamic pricing based on supply/demand
├── TradeRoutes.cs               # AI and player trade routes
└── EconomicZones.cs             # Sector-based economies

Core/Station/
├── ModularStationBuilder.cs     # Module-by-module station construction
├── StationBlueprints.cs         # Station design system
├── ProductionModules.cs         # Manufacturing facilities
├── DefenseModules.cs            # Turrets, shields, hangars
├── LogisticsModules.cs          # Storage, docking bays
└── CrewManagement.cs            # Workforce, food, medical supplies

Core/AI/
├── AITrader.cs                  # Autonomous AI trade ships
├── AIMiner.cs                   # AI mining operations
├── AIFleet.cs                   # AI fleet management
├── BehaviorTrees.cs             # AI decision making
└── AIPathfinding.cs             # Navigation in 3D space
```

**Implementation Priority**: HIGH
- Living economy is the foundation for EVE-style player economy
- Station building provides territory control mechanics
- AI ships create dynamic universe

**Technical Challenges**:
- Performance with thousands of AI ships
- Supply chain simulation complexity
- Market equilibrium algorithms

### Layer 3: Galaxy/Strategic - Stellaris Layer (IMPLEMENTED ✅)
**Focus**: Empire expansion, diplomacy, research, grand strategy

**Already Implemented**:
- ✅ Faction system with ethics and approval
- ✅ Policy management
- ✅ Pop happiness and stability
- ✅ Government types
- ✅ Influence generation

**Additional Features Needed**:
```
Core/Diplomacy/
├── DiplomacySystem.cs           # AI empire relations
├── Treaties.cs                  # Peace, trade, alliance agreements
├── WarDeclaration.cs            # Conflict mechanics
├── Espionage.cs                 # Intelligence gathering
└── FirstContact.cs              # Discovering new empires

Core/Research/
├── TechnologyTree.cs            # Research progression
├── ResearchSystem.cs            # Science production
├── Anomalies.cs                 # Survey and research anomalies
└── TechUnlocks.cs               # Technology effects

Core/Expansion/
├── TerritoryControl.cs          # Sector ownership
├── Starbases.cs                 # Territory claim structures
├── Colonization.cs              # New planet settlement
└── BorderSystem.cs              # Territory boundaries
```

**Implementation Priority**: MEDIUM
- Faction system provides foundation
- Adds strategic depth
- Integrates with economy layer

### Layer 4: Universe/Scale - EVE Online & Elite Dangerous
**Focus**: Massive scale, persistent economy, player-driven politics

**Key Features to Implement**:
```
Core/Persistence/
├── PersistentUniverse.cs        # Server-authoritative world state
├── PlayerCorporations.cs        # Guild/alliance system
├── TerritoryOwnership.cs        # Player-controlled space
├── AssetRegistry.cs             # Track all player assets
└── EconomicHistory.cs           # Price history, market trends

Core/Multiplayer/
├── ShardingSystem.cs            # Distribute load across servers
├── InstanceManager.cs           # Dynamic instance creation
├── LargeScaleBattles.cs         # Handle 100+ player battles
└── ChatSystems.cs               # Alliance, local, system chat

Core/Risk/
├── PermanentLoss.cs             # Assets can be destroyed
├── InsuranceSystem.cs           # Risk mitigation
├── SalvageSystem.cs             # Recover from destroyed ships
└── WarAssets.cs                 # Territory capture mechanics
```

**Implementation Priority**: LOW (Post-MVP)
- Requires robust economy and faction systems first
- Needs significant server infrastructure
- High-stakes PvP is end-game content

## Integration Architecture

### Seamless Transitions
**Problem**: How to transition between layers without loading screens?

**Solution**: Hierarchical Entity System
```csharp
// Core/Integration/LayerManager.cs
public class LayerManager
{
    public enum GameLayer
    {
        Cockpit,        // First-person in ship
        Ship,           // Third-person ship control
        Fleet,          // Local fleet tactical
        System,         // System-wide strategy
        Sector,         // Multi-system operations
        Empire          // Galaxy-wide empire management
    }
    
    private GameLayer _currentLayer;
    private Dictionary<GameLayer, ILayerController> _layerControllers;
    
    public void TransitionToLayer(GameLayer targetLayer)
    {
        // Smooth camera transition
        // Update UI for layer
        // Enable/disable appropriate systems
        // Adjust time scale (faster at higher layers)
    }
}
```

### Time Scale Management
Different layers operate at different time scales:
- **Cockpit/Ship**: Real-time (1:1)
- **Fleet**: Real-time with optional time compression (1-10x)
- **System**: Time compression (10-100x)
- **Empire**: Turn-based or extreme compression (1000x+)

### Data Architecture
```csharp
// Core/Integration/UnifiedEntitySystem.cs
public class SpaceEntity
{
    // All entities (ships, stations, planets) share base properties
    public Guid Id { get; set; }
    public Vector3D GalacticPosition { get; set; }  // 64-bit for scale
    public EntityType Type { get; set; }
    
    // Layer-specific components
    public VoxelStructureComponent? VoxelData { get; set; }      // Layer 1
    public EconomicComponent? Economy { get; set; }              // Layer 2
    public TerritoryComponent? Territory { get; set; }           // Layer 3
    public PersistenceComponent? Persistence { get; set; }       // Layer 4
}
```

## Implementation Roadmap

### Phase 1: Core Layer Integration (4-6 weeks)
- [ ] Implement LayerManager for seamless transitions
- [ ] Add 64-bit galactic coordinate system
- [ ] Create basic flight model (Elite Dangerous style)
- [ ] Implement camera transition system
- [ ] Test layer switching performance

### Phase 2: Living Economy (6-8 weeks)
- [ ] Implement EconomySimulator with supply/demand
- [ ] Add AI trader ships
- [ ] Create modular station building
- [ ] Implement production chains
- [ ] Test with 1000+ AI ships

### Phase 3: Strategic Layer Enhancement (4-5 weeks)
- [ ] Add diplomacy system
- [ ] Implement research/technology tree
- [ ] Create AI empires with behaviors
- [ ] Add territory control mechanics
- [ ] Integrate with existing faction system

### Phase 4: Scale & Polish (6-8 weeks)
- [ ] Implement 1:1 scale galaxy generation
- [ ] Add realistic star systems
- [ ] Create seamless planetary transitions
- [ ] Performance optimization for scale
- [ ] Visual polish and effects

### Phase 5: Persistence & Multiplayer (8-12 weeks)
- [ ] Implement server-authoritative architecture
- [ ] Add player corporations/alliances
- [ ] Create persistent economy tracking
- [ ] Implement large-scale battle handling
- [ ] Add high-stakes PvP mechanics

## Technical Considerations

### Performance Targets
- **60 FPS** in cockpit view with detailed graphics
- **Handle 10,000+ entities** in economy simulation
- **Support 100+ players** in same sector
- **Galaxy size**: 400 billion star systems (like Elite Dangerous)

### Networking Architecture
```
Client Side:
- Local physics prediction
- Input buffering
- Interpolation for smooth movement

Server Side:
- Authoritative game state
- Sector sharding (distribute load)
- Economy tick rate: 1-10 Hz
- Combat tick rate: 20-60 Hz
```

### Data Storage
```
PostgreSQL:
- Player accounts
- Persistent universe state
- Corporation data
- Market history

Redis:
- Real-time market prices
- Active player sessions
- Chat messages

MongoDB:
- Ship/station blueprints
- Event logs
- Analytics data
```

## Example User Journey

**Hour 1-5: Tutorial & Local Play**
- Player starts in cockpit (Layer 1)
- Learns flight controls
- Mines asteroids, builds first ship
- Fights pirates

**Hour 5-20: Economic Expansion**
- Transitions to Fleet control (Layer 2)
- Builds first station
- Sets up automated trade routes
- Manages production chains

**Hour 20-50: Empire Building**
- Zooms out to System view (Layer 3)
- Establishes colonies
- Researches technologies
- Negotiates with AI empires
- Manages faction approval

**Hour 50+: Endgame Politics**
- Joins player corporation (Layer 4)
- Participates in territory wars
- Controls key trade hubs
- Influences galactic economy

## Success Metrics

1. **Immersion**: Players feel present at all scales
2. **Seamlessness**: Transitions between layers feel natural
3. **Depth**: Each layer provides meaningful decisions
4. **Integration**: Actions at one layer affect others
5. **Performance**: Maintains targets across all layers

## Conclusion

This architecture creates a truly unique space game by combining the best elements of five legendary titles into a seamless whole. The key is **gradual implementation** - start with core layers and expand outward, ensuring each layer is solid before moving to the next.

**Current Status**: Foundation is strong with ECS, voxel system, and faction mechanics. Next priority is implementing the economy simulation and flight model to enable the first two gameplay layers.
