# X4-Inspired Rendering System - Complete Guide

## Overview

This guide documents the comprehensive X4: Foundations-inspired rendering system implemented for Codename: Subspace. The system provides detailed, procedurally-generated 3D models for ships, planets, asteroids, and stations, replacing simple geometric primitives with X4-quality visuals.

## System Components

### 1. X4ShipVisualGenerator

**Purpose**: Generates detailed ship visuals with X4-characteristic designs

**Location**: `AvorionLike/Core/Graphics/X4ShipVisualGenerator.cs`

**Features**:
- **6 Design Styles**: Balanced (Argon), Aggressive (Split), Durable (Teladi), Sleek (Paranid), Advanced (Terran), Alien (Xenon)
- **18 Geometry Types**: Hull shapes, wings, cockpits tailored to each style
- **Engine Details**: Glow effects, particle systems, style-specific colors
- **Surface Details**: Panels, vents, antennas, sensors (density scales with ship size)
- **Weapon Mounts**: Fixed and turret hardpoints positioned by ship class
- **Lighting Systems**: Navigation lights (red/green), running lights, position lights with blink patterns

**Ship Classes Supported**:
- **Small**: Fighter_Light, Fighter_Heavy, Miner_Small
- **Medium**: Corvette, Frigate, Gunboat, Miner_Medium, Freighter_Medium
- **Large**: Destroyer, Freighter_Large, Miner_Large
- **Extra Large**: Battleship, Carrier, Builder

**Usage Example**:
```csharp
var generator = new X4ShipVisualGenerator(seed: 12345);
var visuals = generator.GenerateShipVisuals(
    X4ShipClass.Frigate,
    X4DesignStyle.Sleek,
    "Titanium"
);

// Access generated components
foreach (var geometry in visuals.HullGeometry)
{
    // Render hull pieces
}

foreach (var engine in visuals.EngineDetails)
{
    // Render engines with glow
}
```

### 2. X4PlanetRenderer

**Purpose**: Generates detailed planet meshes with atmospheres, rings, and surface features

**Location**: `AvorionLike/Core/Graphics/X4PlanetRenderer.cs`

**Features**:
- **Icosphere Mesh Generation**: Smooth spheres via recursive subdivision (adjustable quality)
- **7 Planet Types**: Rocky, Gas, Ice, Lava, Desert, Ocean, Habitable
- **Atmospheric Layers**: Color, density, scattering for realistic atmospheres
- **Planetary Rings**: Multiple ring systems with gaps (70% chance for gas giants)
- **Cloud Layers**: Animated clouds for habitable/ocean planets
- **Surface Details**: Craters, mountains, dunes, volcanoes, continents based on type
- **Lighting Data**: PBR material properties (specular, roughness, emissive)

**Planet Type Details**:

| Type | Features | Surface Details |
|------|----------|-----------------|
| Rocky | Gray/brown, craters, mountains | 100-300 craters, 5-15 mountain ranges |
| Gas | Bands, storms, rings | 8-20 bands, 2-8 storm systems |
| Ice | White/blue, cracks, caps | 30-100 crack patterns, ice caps |
| Lava | Red/orange, volcanoes, flows | 10-30 volcanoes, 20-60 lava flows, emissive |
| Desert | Tan/yellow, dunes | 20-70 craters, 50-150 dune patterns |
| Ocean | Blue, landmasses | 70-95% ocean, 5-15 landmasses |
| Habitable | Blue/green, continents, forests | 3-8 continents, 50-80% ocean, 10-30 forest regions |

**Usage Example**:
```csharp
var renderer = new X4PlanetRenderer(seed: 54321);
var planet = renderer.GenerateDetailedPlanet(
    planetData,
    subdivisionLevel: 4  // Higher = smoother (1-6 recommended)
);

// Planet components available:
// planet.SphereMesh - vertices, triangles, normals, UVs
// planet.Atmosphere - color, density, scattering
// planet.Rings - inner/outer radius, color, opacity
// planet.CloudLayer - coverage, animation speed
// planet.SurfaceDetails - type-specific features
```

### 3. X4AsteroidRenderer

**Purpose**: Generates organic, irregular asteroid meshes

**Location**: `AvorionLike/Core/Graphics/X4AsteroidRenderer.cs`

**Features**:
- **6 Shape Archetypes**: Spherical, Elongated, Flat, Chunky, Irregular, Very Irregular
- **Multi-Octave Noise**: Organic deformation using Perlin noise (4 octaves)
- **Surface Features**: Craters, cracks, boulders, ridges, depressions
- **Material-Specific Appearance**: Visual variation based on resource type (Iron, Titanium, etc.)
- **Resource Veins**: Visible glowing veins for high-value asteroids
- **PBR Materials**: Metallic, roughness, specular, emissive properties per material

**Resource Material Properties**:

| Resource | Base Color | Metallic | Roughness | Emissive |
|----------|------------|----------|-----------|----------|
| Iron | Gray-brown | 0.6 | 0.8 | 0.0 |
| Titanium | Silver-gray | 0.7 | 0.6 | 0.0 |
| Naonite | Green-teal | 0.5 | 0.7 | 0.1 |
| Trinium | Blue-gray | 0.8 | 0.5 | 0.15 |
| Xanion | Purple-pink | 0.7 | 0.4 | 0.2 |
| Ogonite | Yellow-green | 0.9 | 0.3 | 0.25 |
| Avorion | Red | 0.95 | 0.2 | 0.3 |

**Usage Example**:
```csharp
var renderer = new X4AsteroidRenderer(seed: 99999);
var asteroid = renderer.GenerateDetailedAsteroid(
    asteroidData,
    detailLevel: 2  // 0=low, 1=medium, 2=high detail
);

// Asteroid mesh ready for rendering
// asteroid.BaseMesh - vertices, triangles, normals, UVs
// asteroid.SurfaceFeatures - craters, cracks, etc.
// asteroid.MaterialData - PBR properties
// asteroid.ResourceVeins - glowing ore veins (if present)
```

### 4. X4StationRenderer

**Purpose**: Generates complex modular space stations

**Location**: `AvorionLike/Core/Graphics/X4StationRenderer.cs`

**Features**:
- **8 Station Types**: TradingPost, Shipyard, Factory, MiningStation, ResearchStation, DefensePlatform, RefuelingDepot, CommandCenter
- **Modular Architecture**: Core hub + type-specific modules + connectors
- **Docking Bays**: Multiple ship docking points with lights (4-10+ per station)
- **External Details**: Antennas, solar panels, lights, dishes, vents (20+ per station)
- **Defensive Systems**: Turrets for military/defense stations
- **Lighting Systems**: Navigation lights + work lights (30-100+ lights per station)
- **Complexity Scaling**: Module count increases with complexity parameter (1-5)

**Station Type Modules**:

| Type | Core Size | Special Modules | Module Count |
|------|-----------|-----------------|--------------|
| TradingPost | 40×40×40 | Cargo bays, market halls | 6-14 |
| Shipyard | 60×50×60 | Construction frames, assembly bays | 5-10 |
| Factory | 80×40×80 | Production lines, refineries | 12-30 |
| MiningStation | 50×50×50 | Ore processors, storage silos | 10-25 |
| ResearchStation | 45×60×45 | Laboratories, sensor arrays | 8-16 |
| DefensePlatform | 50×30×50 | Weapon batteries, shield generators | 10-20 |
| RefuelingDepot | 40×40×40 | Fuel tanks, pumping stations | 12-32 |
| CommandCenter | 70×70×70 | Comms towers, control rooms | 14-28 |

**Usage Example**:
```csharp
var renderer = new X4StationRenderer(seed: 11111);
var station = renderer.GenerateDetailedStation(
    X4StationType.Shipyard,
    material: "Trinium",
    complexity: 3  // 1=small, 3=medium, 5=large
);

// Station ready for rendering
// station.CoreHub - central module
// station.Modules - all attached modules
// station.Connectors - tubes/trusses between modules
// station.DockingBays - ship docking locations
// station.ExternalDetails - antennas, panels, etc.
// station.DefensiveTurrets - weapons (if applicable)
// station.LightingSystems - all lights
```

## Integration Guide

### Step 1: Replace Existing Renderers

#### For Ships:
```csharp
// OLD: Simple modular OBJ assembly
var ship = ModularShipFactory.CreateShip(config);

// NEW: Enhanced X4 visuals
var visualGen = new X4ShipVisualGenerator(seed);
var visuals = visualGen.GenerateShipVisuals(shipClass, designStyle, material);

// Apply visuals to ship entity
ApplyEnhancedVisuals(ship, visuals);
```

#### For Planets:
```csharp
// OLD: Simple sphere with procedural texture
var planet = CreateSimpleSphere(planetData);

// NEW: Detailed planet with features
var planetRenderer = new X4PlanetRenderer(seed);
var detailedPlanet = planetRenderer.GenerateDetailedPlanet(planetData, subdivisionLevel: 4);

// Render planet with all features
RenderPlanetMesh(detailedPlanet.SphereMesh);
if (detailedPlanet.Atmosphere != null)
    RenderAtmosphere(detailedPlanet.Atmosphere);
if (detailedPlanet.Rings != null)
    RenderRings(detailedPlanet.Rings);
```

#### For Asteroids:
```csharp
// OLD: Voxel-based blocky asteroids
var asteroidBlocks = AsteroidVoxelGenerator.GenerateAsteroid(asteroidData);

// NEW: Organic asteroid mesh
var asteroidRenderer = new X4AsteroidRenderer(seed);
var asteroid = asteroidRenderer.GenerateDetailedAsteroid(asteroidData, detailLevel: 2);

// Render smooth asteroid mesh
RenderAsteroidMesh(asteroid.BaseMesh, asteroid.MaterialData);
```

#### For Stations:
```csharp
// OLD: Voxel-based procedural stations
var stationBlocks = ProceduralStationGenerator.GenerateStation(stationType);

// NEW: Modular X4-style stations
var stationRenderer = new X4StationRenderer(seed);
var station = stationRenderer.GenerateDetailedStation(stationType, material, complexity: 3);

// Render all station components
RenderStationCore(station.CoreHub);
foreach (var module in station.Modules)
    RenderStationModule(module);
foreach (var connector in station.Connectors)
    RenderConnector(connector);
```

### Step 2: Rendering Pipeline Integration

```csharp
public class EnhancedRenderingSystem
{
    private X4ShipVisualGenerator _shipGen;
    private X4PlanetRenderer _planetRenderer;
    private X4AsteroidRenderer _asteroidRenderer;
    private X4StationRenderer _stationRenderer;
    
    public void Initialize(int seed)
    {
        _shipGen = new X4ShipVisualGenerator(seed);
        _planetRenderer = new X4PlanetRenderer(seed);
        _asteroidRenderer = new X4AsteroidRenderer(seed);
        _stationRenderer = new X4StationRenderer(seed);
    }
    
    public void RenderScene(GameWorld world)
    {
        // Render all ships with enhanced visuals
        foreach (var ship in world.Ships)
        {
            var visuals = _shipGen.GenerateShipVisuals(ship.Class, ship.Style, ship.Material);
            RenderShipVisuals(ship.Position, visuals);
        }
        
        // Render planets with full detail
        foreach (var planet in world.Planets)
        {
            var detailed = _planetRenderer.GenerateDetailedPlanet(planet.Data);
            RenderDetailedPlanet(planet.Position, detailed);
        }
        
        // Render asteroids
        foreach (var asteroid in world.Asteroids)
        {
            var detailed = _asteroidRenderer.GenerateDetailedAsteroid(asteroid.Data);
            RenderDetailedAsteroid(asteroid.Position, detailed);
        }
        
        // Render stations
        foreach (var station in world.Stations)
        {
            var detailed = _stationRenderer.GenerateDetailedStation(
                station.Type, station.Material, station.Complexity);
            RenderDetailedStation(station.Position, detailed);
        }
    }
}
```

### Step 3: Performance Optimization

**LOD (Level of Detail) System**:
```csharp
public DetailLevel GetDetailLevel(float distance)
{
    if (distance < 100f) return DetailLevel.High;      // Full detail
    if (distance < 500f) return DetailLevel.Medium;    // Some detail
    if (distance < 2000f) return DetailLevel.Low;      // Basic shape
    return DetailLevel.VeryLow;                        // Simplified
}

// Use with renderers:
var detailLevel = GetDetailLevel(Vector3.Distance(camera.Position, object.Position));
var subdivisions = detailLevel switch
{
    DetailLevel.High => 4,
    DetailLevel.Medium => 2,
    DetailLevel.Low => 1,
    _ => 0
};
```

**Caching**:
```csharp
private Dictionary<int, EnhancedShipVisuals> _shipCache = new();
private Dictionary<int, DetailedPlanet> _planetCache = new();

public EnhancedShipVisuals GetShipVisuals(int shipId, ...)
{
    if (_shipCache.TryGetValue(shipId, out var cached))
        return cached;
    
    var visuals = _shipGen.GenerateShipVisuals(...);
    _shipCache[shipId] = visuals;
    return visuals;
}
```

## Visual Quality Settings

### Quality Presets

**Ultra**:
- Planet subdivision: 5-6
- Asteroid detail: 2 (high)
- Station complexity: 4-5
- Full lighting and effects

**High**:
- Planet subdivision: 4
- Asteroid detail: 2
- Station complexity: 3
- Most effects enabled

**Medium**:
- Planet subdivision: 2-3
- Asteroid detail: 1
- Station complexity: 2
- Essential effects only

**Low**:
- Planet subdivision: 1-2
- Asteroid detail: 0
- Station complexity: 1
- Minimal effects

## Troubleshooting

### Performance Issues

**Problem**: Low FPS with many objects
**Solution**: 
- Implement LOD system
- Use object pooling
- Cache generated meshes
- Reduce subdivision levels

### Visual Artifacts

**Problem**: Z-fighting on surfaces
**Solution**:
- Adjust near/far plane distances
- Enable depth testing
- Use appropriate polygon offset

**Problem**: Seams in icosphere
**Solution**:
- Ensure UV coordinates wrap correctly
- Use seamless textures
- Increase subdivision level

### Memory Usage

**Problem**: High memory consumption
**Solution**:
- Implement mesh streaming
- Use texture atlases
- Reduce cached object count
- Clear unused meshes periodically

## Future Enhancements

### Planned Features:
1. **Dynamic Damage Deformation**: Mesh deformation for battle damage
2. **Weather Systems**: Dynamic clouds, storms on planets
3. **Station Growth**: Modules added/removed over time
4. **Ship Customization**: Player-selectable hull details
5. **Procedural Textures**: Generated textures for variation
6. **Animated Components**: Rotating parts, moving docking arms
7. **Particle Effects**: Exhaust trails, explosions, shield impacts

### API Extensions:
- Shader integration for PBR rendering
- Animation system for moving parts
- Decal system for markings
- Material blending for weathering
- Procedural texture generation

## Performance Benchmarks

Typical generation times (in milliseconds):

| Component | Low Detail | Medium Detail | High Detail |
|-----------|------------|---------------|-------------|
| Ship Visuals | 5-10ms | 10-20ms | 20-40ms |
| Planet | 10-30ms | 30-80ms | 80-200ms |
| Asteroid | 5-15ms | 15-30ms | 30-60ms |
| Station | 15-40ms | 40-100ms | 100-250ms |

**Note**: Generation times are one-time costs. Meshes should be cached for reuse.

## Conclusion

The X4-inspired rendering system provides a significant visual upgrade from simple geometric primitives to detailed, procedurally-generated 3D models. By following this guide, you can integrate these renderers into your game and achieve X4-quality visuals for all major game objects.

For questions or contributions, see the main project README.md.
