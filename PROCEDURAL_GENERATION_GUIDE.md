# Procedural Generation System Guide

## Overview

This document describes the comprehensive procedural generation system implemented for Codename-Subspace. The system generates asteroid fields, space stations, and Stargates across a large-scale voxel solar system using deterministic algorithms.

## System Architecture

The procedural generation follows a **layered approach** with three levels:

1. **Macro Level (System/Galaxy)** - Solar systems and their connectivity
2. **Meso Level (Solar)** - Planets, asteroid belts, stations, and gates within systems
3. **Micro Level (Voxel)** - Individual asteroids and structures

### Key Design Principles

- **Deterministic Generation**: Everything is generated from seeds, ensuring consistency
- **Lazy/On-Demand**: Objects are only generated when needed (player approaches)
- **Performance Optimized**: Instancing, LOD, spatial hashing, and chunking
- **Scalable**: Handles vast distances with floating-origin coordinate system

## Core Components

### 1. Floating Origin Coordinate System

**File**: `FloatingOriginCoordinates.cs`

Handles the immense scale of space without floating-point precision errors.

```csharp
// Create coordinates from world position
var coords = FloatingOriginCoordinates.FromWorldPosition(worldPosition);

// Access sector and local position
var sector = coords.Sector;         // (x, y, z) sector indices
var localPos = coords.LocalPosition; // Position within sector

// Calculate distance safely
var distance = coords.DistanceTo(otherCoords);
```

**Features**:
- Sectors are 100,000 units (100km) each
- Separates coordinates into sector (integer) and local (float)
- Prevents precision loss at large distances
- Provides normalized coordinates and distance calculations

### 2. Solar System Generation

**File**: `SolarSystemData.cs`, `StarSystemGenerator.cs`

Generates complete solar systems with all celestial objects.

```csharp
// Create system generator
var generator = new StarSystemGenerator(galaxySeed);

// Generate a system
var coords = new Vector3Int(0, 0, 0);
var system = generator.GenerateSystem(coords);

// Access system components
foreach (var planet in system.Planets)
    Console.WriteLine($"{planet.Name}: {planet.Type}");

foreach (var belt in system.AsteroidBelts)
    Console.WriteLine($"{belt.Name}: {belt.PrimaryResource}");

foreach (var station in system.Stations)
    Console.WriteLine($"{station.Name}: {station.StationType}");
```

**System Types**:
- `Core` - Center of galaxy, high resources, high danger
- `Civilized` - Many stations, low danger
- `Frontier` - Few stations, medium danger
- `Contested` - Faction warfare, high danger
- `Empty` - Few objects, low resources
- `AsteroidRich` - High asteroid density
- `Nebula` - Special effects, medium resources

**Generated Content**:
- Central star (Red Dwarf, Yellow Star, Blue Giant, etc.)
- 2-8 planets with orbits
- 0-4 asteroid belts
- 0-6 space stations
- Stargates (added via galaxy network)

### 3. Galaxy Network

**File**: `GalaxyNetwork.cs`

Manages the graph of connected solar systems with jump gates.

```csharp
// Initialize network
var network = new GalaxyNetwork(galaxySeed);

// Get or generate a system
var system = network.GetOrGenerateSystem(new Vector3Int(0, 0, 0));

// Find path between systems
var path = network.FindPath(startSystemId, endSystemId);

// Get systems in range
var nearbySystemIds = network.GetSystemsInRange(systemId, maxJumps: 3);

// Get gate to destination
var gate = network.GetGateToDestination(currentSystemId, destSystemId);
```

**Features**:
- Graph-based system connectivity
- Breadth-first pathfinding for routes
- Systems have 1-7 connections based on type
- Bidirectional connections
- Route planning support

### 4. Asteroid Fields

**File**: `AsteroidField.cs`

Optimized asteroid field generation with spatial hashing and LOD.

```csharp
// Create asteroid field from belt data
var field = new AsteroidField(fieldId, beltData, seed);

// Configure LOD distances
field.HighDetailDistance = 1000f;
field.MediumDetailDistance = 5000f;
field.LowDetailDistance = 15000f;
field.CullDistance = 25000f;

// Get asteroids in region (lazy generation)
var asteroids = field.GetAsteroidsInRegion(playerPosition, radius);

// Update LOD based on viewer position
field.UpdateLOD(viewerPosition, asteroids);

// Clean up distant cells to save memory
field.ClearDistantCells(playerPosition, keepRadius: 50000f);

// Get statistics
var stats = field.GetStats();
```

**Features**:
- **Spatial Hashing**: 5000-unit cells for efficient queries
- **Lazy Generation**: Only generates asteroids when requested
- **LOD System**: 4 levels (High, Medium, Low, Billboard)
- **Deterministic**: Same cell always generates same asteroids
- **Memory Efficient**: Can clear distant cells
- **Instanced Rendering Ready**: Provides mesh variant indices

**LOD Levels**:
- `High` (< 1000 units): Full voxel detail, interactive
- `Medium` (< 5000 units): Simplified mesh
- `Low` (< 15000 units): Very simple mesh
- `Billboard` (< 25000 units): 2D sprite
- Beyond cull distance: Not rendered

### 5. Stargates

**File**: `StargateGenerator.cs`, `SolarSystemData.cs`

Jump gates connecting solar systems with trigger zones.

```csharp
// Create stargate generator
var stargateGen = new StargateGenerator(galaxyNetwork, entityManager);

// Generate stargates for a system
var stargateEntities = stargateGen.GenerateStargatesForSystem(system);

// Each gate has:
// - Position and rotation
// - Destination system ID
// - Gate type (Standard, Ancient, Unstable, Military)
// - Trigger zone for player detection
```

**Gate Types**:
- `Standard` - Normal jump gate (200 unit radius)
- `Ancient` - Faster, larger (300 unit radius)
- `Unstable` - Smaller, risky (150 unit radius)
- `Military` - Faction controlled (250 unit radius)

**Trigger System**:
- Cylindrical trigger zones
- Automatic detection of player ships
- Calls hyperspace jump system
- No fuel cost for gate jumps

### 6. Hyperspace Jump System

**File**: `HyperspaceJump.cs` (Enhanced)

Manages jumps between systems with gate integration.

```csharp
// Initialize jump system
var hyperspaceJump = new HyperspaceJump();
hyperspaceJump.SetGalaxyNetwork(galaxyNetwork);
hyperspaceJump.SetCurrentSystem(currentSystemId);

// Initiate gate jump (no cost!)
bool success = hyperspaceJump.InitiateGateJump(
    destinationSystemId,
    destinationGateId,
    LoadSystemCallback
);

// Update jump state
hyperspaceJump.Update(deltaTime);

// Check jump state
if (hyperspaceJump.State == JumpState.Complete)
{
    // Position player at exit gate
    var exitPos = hyperspaceJump.ExitGatePosition;
    hyperspaceJump.Reset();
}
```

**Jump States**:
- `Ready` - Can initiate jump
- `Initiating` - Starting sequence
- `Loading` - Loading destination
- `Emerging` - Exiting hyperspace
- `Complete` - Jump finished
- `Failed` - Error occurred

## Usage Examples

### Example 1: Generate a Complete Galaxy Region

```csharp
// Initialize galaxy
var galaxySeed = 42;
var network = new GalaxyNetwork(galaxySeed);

// Generate starting system
var homeCoords = new Vector3Int(0, 0, 0);
var homeSystem = network.GetOrGenerateSystem(homeCoords);

// Generate nearby systems (for map display)
for (int x = -2; x <= 2; x++)
{
    for (int y = -2; y <= 2; y++)
    {
        for (int z = -2; z <= 2; z++)
        {
            var coords = new Vector3Int(x, y, z);
            var system = network.GetOrGenerateSystem(coords);
            // Render on galaxy map...
        }
    }
}
```

### Example 2: Load System and Generate All Content

```csharp
// Generate system
var system = network.GetOrGenerateSystem(coordinates);

// Create entity manager
var entityManager = new EntityManager();

// Generate stargates
var stargateGen = new StargateGenerator(network, entityManager);
var gates = stargateGen.GenerateStargatesForSystem(system);

// Generate space stations (custom implementation needed)
foreach (var stationData in system.Stations)
{
    // Create station entity with prefab model
    // Position at stationData.Position
}

// Generate asteroid fields
foreach (var beltData in system.AsteroidBelts)
{
    var field = new AsteroidField(
        $"{system.SystemId}-Belt-{index}",
        beltData,
        system.Seed
    );
    // Register field for updates
}
```

### Example 3: Player Movement and Asteroid Loading

```csharp
// Player update loop
void Update(float deltaTime)
{
    var playerPos = GetPlayerPosition();
    
    // Update each asteroid field
    foreach (var field in asteroidFields)
    {
        // Get asteroids in player's view range
        var asteroids = field.GetAsteroidsInRegion(playerPos, viewDistance);
        
        // Update LOD
        field.UpdateLOD(playerPos, asteroids);
        
        // Render visible asteroids
        foreach (var asteroid in asteroids.Where(a => a.IsVisible))
        {
            RenderAsteroid(asteroid);
        }
        
        // Clean up distant cells periodically
        if (frameCount % 300 == 0) // Every 5 seconds at 60fps
        {
            field.ClearDistantCells(playerPos, keepRadius: 50000f);
        }
    }
}
```

### Example 4: Route Planning

```csharp
// Find path between systems
var path = network.FindPath(currentSystemId, targetSystemId);

if (path != null)
{
    Console.WriteLine($"Route requires {path.Count - 1} jumps:");
    
    for (int i = 0; i < path.Count - 1; i++)
    {
        var fromId = path[i];
        var toId = path[i + 1];
        
        // Get the gate
        var gate = network.GetGateToDestination(fromId, toId);
        
        Console.WriteLine($"  {i + 1}. {fromId} → {toId} via {gate?.Name}");
    }
}
else
{
    Console.WriteLine("No route found!");
}
```

## Performance Considerations

### Memory Management

1. **Asteroid Fields**: Clear distant cells regularly
```csharp
field.ClearDistantCells(playerPosition, keepRadius);
```

2. **Systems**: Only generate systems near player or on route
3. **LOD**: Use appropriate LOD distances for your GPU capabilities

### Optimization Tips

1. **Spatial Queries**: Use `GetAsteroidsInRegion` with reasonable radius
2. **Update Frequency**: Don't update asteroid LOD every frame
3. **Instancing**: Group asteroids by mesh variant and LOD for instanced rendering
4. **Culling**: Check `IsVisible` flag before rendering
5. **Generation**: Spread generation across frames if needed

### Recommended Settings

```csharp
// For typical gameplay
field.HighDetailDistance = 1000f;    // Interactive range
field.MediumDetailDistance = 5000f;  // Combat range
field.LowDetailDistance = 15000f;    // Visual range
field.CullDistance = 25000f;         // Beyond this, don't render

// Update frequencies
asteroidLODUpdate = 0.1f;  // 10 times per second
cellCleanup = 5.0f;        // Every 5 seconds
chunkUpdate = 1.0f;        // Once per second
```

## Integration with Existing Systems

### With Voxel System

The existing `AsteroidVoxelGenerator` can generate detailed voxel data on-demand:

```csharp
if (asteroid.LODLevel == LODLevel.High && !asteroid.IsGenerated)
{
    var asteroidData = new AsteroidData
    {
        Position = asteroid.Position,
        Size = asteroid.Size,
        ResourceType = asteroid.ResourceType
    };
    
    var voxelGen = new AsteroidVoxelGenerator(seed);
    asteroid.VoxelPositions = voxelGen.GenerateAsteroid(asteroidData);
    asteroid.IsGenerated = true;
}
```

### With Mining System

```csharp
// When player mines an asteroid
if (asteroid.IsGenerated && asteroid.VoxelPositions != null)
{
    // Use existing mining system
    miningSystem.MineAsteroid(asteroid);
}
```

### With Physics System

```csharp
// Add collision for nearby asteroids
foreach (var asteroid in nearbyAsteroids)
{
    if (asteroid.LODLevel <= LODLevel.Medium)
    {
        physicsSystem.AddCollider(asteroid);
    }
}
```

## Testing

Run the comprehensive example:

```csharp
var example = new ProceduralGenerationExample();
example.RunAllExamples();
```

This demonstrates:
- Solar system generation
- Galaxy network exploration
- Pathfinding
- Asteroid field generation with LOD
- Stargate jumps
- Floating origin coordinates

## Future Enhancements

Potential additions:
1. **Nebulae**: Volumetric gas clouds with special rendering
2. **Wormholes**: Dynamic connections between distant systems
3. **Black Holes**: Gravity wells and special physics
4. **Faction Territory**: Systems owned by factions
5. **Dynamic Events**: Temporary phenomena (meteor showers, etc.)
6. **Procedural Stations**: Generate station interiors
7. **Anomalies**: Rare, special locations with unique content

## Technical Details

### Determinism

All generation uses deterministic seeds:
- Galaxy seed → System coordinates → System seed
- System seed → All content in that system
- Cell coordinates + field seed → Asteroids in that cell

This ensures:
- Same seed always produces same universe
- Multiplayer consistency
- No need to store generated data
- Can regenerate anything on demand

### Coordinate System

```
World Space (float, limited precision)
    ↓
Floating Origin Coordinates
    ├─ Sector (Vector3Int) - Coarse grid
    └─ Local Position (Vector3) - Fine position within sector
```

Benefits:
- Handles distances up to ~2 billion sectors
- Each sector is 100km
- Total range: ~200 million km in each direction
- No precision loss in local coordinates

### Network Graph

```
Systems (Nodes)
    ├─ Type determines connection count
    ├─ Distance from center affects danger
    └─ Generates connections to nearby systems
        
Stargates (Edges)
    ├─ Bidirectional connections
    ├─ Physical structures at system edge
    └─ Trigger zones for player detection
```

## Troubleshooting

**Issue**: Asteroid fields have gaps
- **Solution**: Increase belt density or reduce cell size

**Issue**: LOD popping is noticeable
- **Solution**: Adjust LOD distance thresholds or add hysteresis

**Issue**: Memory usage too high
- **Solution**: Reduce keepRadius in ClearDistantCells

**Issue**: Frame drops during generation
- **Solution**: Spread generation across frames using coroutines

**Issue**: Floating-point precision errors
- **Solution**: Use FloatingOriginCoordinates for large distances

## Conclusion

This procedural generation system provides a complete, deterministic, and performant solution for generating large-scale space environments. It follows industry best practices and is designed to scale from small sectors to entire galaxies.

For more examples, see `ProceduralGenerationExample.cs`.
