# Resource Depot Station Guide

## Overview

Resource Depot stations are industrial facilities where miners can sell their collected ore. These stations feature large cylindrical storage silos and are typically surrounded by asteroid fields for convenient mining operations.

**Reference**: Based on the visual style from 1233.PNG showing industrial mining stations.

## Station Features

### Core Facilities

When you visit a Resource Depot station, you'll find these facilities:

- **Ore Receiving Bay**: Where miners dock to offload their cargo
- **Ore Storage Silos**: Massive cylindrical containers for storing raw ore
- **Assay Laboratory**: Tests and verifies ore quality and composition
- **Trading Office**: Where ore purchases are negotiated
- **Mineral Marketplace**: Central trading floor for ore transactions

### Visual Design

Resource Depot stations have a distinctive industrial aesthetic:

#### Ore Storage Silos
- **4-8 large cylindrical silos** arranged around the station core
- **Industrial gray color** (0x4A4A50) with structural detail
- **Orange warning stripes** (0xFFA500) at top and bottom sections
- **Dome tops** for protective covering
- **25-40 blocks tall** with 8-12 block radius
- **Connecting pipes** linking silos to main station

#### Industrial Architecture
- Uses `StationArchitecture.Industrial` style
- Built with Iron or Titanium materials
- Heavy, utilitarian design
- External piping and framework
- Cargo blocks used for storage areas

### Surrounding Asteroid Field

Resource Depot stations are strategically placed near rich asteroid fields:

- **25+ asteroids** orbiting the station
- **Distance**: 100-250 units from station center
- **Size**: 12-30 units diameter
- **Materials**: Iron, Titanium, Naonite (varied distribution)
- Provides convenient mining opportunities for traders

## Creating a Resource Depot Station

### Basic Configuration

```csharp
var config = new StationGenerationConfig
{
    Size = StationSize.Large,           // Large stations for massive storage
    StationType = "Resource Depot",     // or "ResourceDepot" or "Depot"
    Material = "Iron",                  // Industrial material
    Architecture = StationArchitecture.Industrial,
    Seed = 12345,                       // For reproducible generation
    IncludeDockingBays = true,
    MinDockingBays = 8                  // Many bays for miner traffic
};

var station = generator.GenerateStation(config);
```

### With Surrounding Asteroids

```csharp
// Generate the station
var stationPosition = new Vector3(800, -200, -600);
var station = generator.GenerateStation(config);

// Generate asteroids around it
var asteroids = generator.GenerateStationAsteroids(
    stationPosition,
    asteroidCount: 25,      // Number of asteroids
    minDistance: 100f,      // Minimum distance from station
    maxDistance: 250f       // Maximum distance from station
);

// Spawn the asteroids in the world
foreach (var (asteroidPos, size, material) in asteroids)
{
    // Create asteroid entity with specified position, size, and material
    CreateAsteroid(asteroidPos, size, material);
}
```

## Station Types Comparison

| Type | Primary Function | Special Features | Typical Materials |
|------|------------------|------------------|-------------------|
| **Resource Depot** | Ore trading | Storage silos, asteroid field | Iron, Titanium |
| Trading | General commerce | Market halls, cargo bays | Titanium |
| Military | Defense | Armories, barracks | Ogonite, Avorion |
| Refinery | Ore processing | Processing plants, labs | Titanium, Naonite |
| Shipyard | Ship construction | Construction bays, warehouses | Xanion, Avorion |

## Ore Trading Mechanics

### Selling Ore

When you dock at a Resource Depot:

1. **Approach the station** and request docking clearance
2. **Dock at Ore Receiving Bay** (one of 8+ docking points)
3. **Navigate to Trading Office** facility
4. **Sell your ore** at current market prices
5. **Prices vary** based on:
   - Ore type and quality
   - Current supply/demand
   - Station's storage capacity
   - Your reputation with the station

### Ore Types and Values

Common ore types you can sell:

- **Iron**: Basic ore, low value, always in demand
- **Titanium**: Industrial ore, medium value
- **Naonite**: Rare green ore, high value
- **Trinium**: Blue crystalline ore, very high value
- **Xanion**: Golden ore, extremely valuable
- **Ogonite**: Orange-red ore, premium pricing
- **Avorion**: Purple legendary ore, maximum value

## Mining Near Resource Depots

### Efficient Mining Strategy

1. **Dock at Resource Depot** to use as base of operations
2. **Survey asteroid field** - use scanner to find richest asteroids
3. **Mine asteroids** in 100-250 unit radius around station
4. **Return to station** when cargo is full
5. **Sell ore immediately** - no long travel required
6. **Repeat** for continuous income

### Asteroid Distribution

Asteroids around Resource Depots typically contain:
- **40% Iron** - Common, abundant
- **35% Titanium** - Moderate rarity
- **25% Naonite** - Rarer, higher value

This provides a good mix for profitable mining operations.

## Station Architecture Details

### Industrial Architecture Style

Resource Depots use the Industrial architecture:

```csharp
StationArchitecture.Industrial
```

This creates:
- **Complex framework structures**
- **Exposed beams and girders**
- **Utilitarian, functional design**
- **No decorative elements**
- **Heavy, robust appearance**
- **External piping and conduits**

### Storage Silo Construction

Each silo is built as:

1. **Vertical cylinder** (12 segments around circumference)
2. **25-40 blocks tall**
3. **8-12 block radius**
4. **Dome top** (tapered cap)
5. **Orange warning stripes** at top (5 blocks) and bottom (5 blocks)
6. **Gray industrial hull** for main body
7. **Connecting pipe** to main station

### Color Scheme

- **Primary**: Industrial Gray (0x4A4A50)
- **Accent**: Safety Orange (0xFFA500)
- **Pipes**: Dark Brown (0x4A3F30)
- **Structure**: Iron/Titanium material color

## Example: Complete Resource Depot Setup

```csharp
// 1. Create the station
var stationGen = new ProceduralStationGenerator(seed: 42);
var asteroidGen = new AsteroidVoxelGenerator();

var stationConfig = new StationGenerationConfig
{
    Size = StationSize.Large,
    StationType = "Resource Depot",
    Material = "Iron",
    Architecture = StationArchitecture.Industrial,
    Seed = 42,
    IncludeDockingBays = true,
    MinDockingBays = 8
};

var station = stationGen.GenerateStation(stationConfig);
var stationPosition = new Vector3(800, -200, -600);

// Spawn station in world
SpawnStation(station, stationPosition);

// 2. Generate asteroids around it
var asteroids = stationGen.GenerateStationAsteroids(
    stationPosition,
    asteroidCount: 25,
    minDistance: 100f,
    maxDistance: 250f
);

// 3. Spawn asteroids in world
foreach (var (asteroidPos, size, material) in asteroids)
{
    var asteroidData = new AsteroidData
    {
        Position = Vector3.Zero,
        Size = size,
        ResourceType = material
    };
    
    var asteroidBlocks = asteroidGen.GenerateAsteroid(asteroidData, voxelResolution: 6);
    
    // Create asteroid entity and add to world
    var asteroidEntity = CreateAsteroidEntity(asteroidPos, asteroidBlocks);
}

Console.WriteLine($"Created Resource Depot at {stationPosition}");
Console.WriteLine($"  - {station.BlockCount} blocks");
Console.WriteLine($"  - {station.DockingPoints.Count} docking bays");
Console.WriteLine($"  - {asteroids.Count} surrounding asteroids");
Console.WriteLine($"  - Facilities: {string.Join(", ", station.Facilities)}");
```

## Customization Options

### Adjusting Silo Count

Modify the silo count in `AddOreStorageContainers`:

```csharp
// In ProceduralStationGenerator.cs, line ~1650
int siloCount = 4 + _random.Next(5); // 4-8 silos

// Change to:
int siloCount = 6 + _random.Next(7); // 6-12 silos for mega depot
```

### Adjusting Asteroid Field Density

```csharp
var asteroids = generator.GenerateStationAsteroids(
    stationPosition,
    asteroidCount: 50,       // Increase from 25 for denser field
    minDistance: 80f,        // Decrease for closer asteroids
    maxDistance: 300f        // Increase for wider field
);
```

### Custom Colors

```csharp
// After generating station, customize colors
foreach (var block in station.Structure.Blocks)
{
    if (block.BlockType == BlockType.Cargo)
    {
        // Custom silo color
        block.ColorRGB = 0x3A4A3A; // Dark olive-green
    }
}
```

## Performance Considerations

### Station Block Count

Resource Depot stations with silos can have:
- **Base station**: 5000-8000 blocks (Large size)
- **Storage silos**: 2000-4000 blocks (4-8 silos × 300-500 blocks each)
- **Total**: 7000-12000 blocks

### Asteroid Count

Balance performance vs visual impact:
- **Minimum**: 15 asteroids (sparse field)
- **Recommended**: 25 asteroids (good balance)
- **Maximum**: 50 asteroids (dense field, may impact performance)

Each asteroid has 50-150 blocks depending on size.

## Integration with Game Systems

### Trading System

Resource Depots should integrate with:
- **Economy system**: Dynamic ore pricing
- **Reputation system**: Better prices for higher reputation
- **Quest system**: Delivery missions to depot
- **Market system**: Supply and demand mechanics

### Mining System

Depots support mining gameplay:
- **Scanner integration**: Detect ore types in asteroids
- **Cargo management**: Store mined ore in ship cargo
- **Mining laser system**: Extract ore from asteroids
- **Resource persistence**: Asteroids regenerate over time

## Future Enhancements

Planned improvements for Resource Depot stations:

1. **Animated conveyor systems** moving ore between silos
2. **Docking queue management** for busy stations
3. **Refinery modules** for on-site ore processing
4. **Price ticker displays** showing current ore values
5. **NPC miner ships** flying to/from asteroids
6. **Station reputation system** affecting buy prices
7. **Storage capacity limits** affecting supply/demand
8. **Asteroid respawn mechanics** for renewable resources

## See Also

- `ProceduralStationGenerator.cs` - Station generation code
- `AsteroidVoxelGenerator.cs` - Asteroid generation
- `SHIP_GENERATION_GUIDE.md` - Ship creation
- `TEXTURE_CUSTOMIZATION_GUIDE.md` - Visual customization
- `DECAL_SYSTEM_GUIDE.md` - Decal application
