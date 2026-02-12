# Space Exploration & Mining Systems Guide

This guide covers the comprehensive space exploration and mining systems implemented for Codename:Subspace, an Avorion-inspired space game engine.

## Overview

The space exploration system provides a complete implementation of procedural world generation, voxel-based asteroids, dynamic chunk management, multithreaded processing, optimized rendering, and interactive mining mechanics.

## System Architecture

### 1. Procedural Generation System

#### NoiseGenerator
**Location:** `AvorionLike/Core/Procedural/NoiseGenerator.cs`

Provides noise functions for procedural content generation:

```csharp
// Generate Perlin noise
float value = NoiseGenerator.PerlinNoise3D(x, y, z);

// Generate fractal noise (multiple octaves)
float fractal = NoiseGenerator.FractalNoise3D(x, y, z, octaves: 4, persistence: 0.5f);

// Generate turbulence
float turbulence = NoiseGenerator.Turbulence3D(x, y, z, octaves: 4);
```

**Features:**
- Perlin noise (3D)
- Fractal/octave noise
- Simplex-style noise
- Turbulence generation
- Signed distance fields (SDF) for shapes
- Smooth SDF union operations

#### AsteroidVoxelGenerator
**Location:** `AvorionLike/Core/Procedural/AsteroidVoxelGenerator.cs`

Generates voxel-based asteroids with embedded resources:

```csharp
var generator = new AsteroidVoxelGenerator(seed: 12345);

// Generate full asteroid with resources
var blocks = generator.GenerateAsteroid(asteroidData, voxelResolution: 8);

// Generate simple spherical asteroid (fast)
var simpleBlocks = generator.GenerateSimpleAsteroid(
    position, size: 50f, material: "Iron", segments: 6
);

// Generate custom-shaped asteroid using SDF
var shapedBlocks = generator.GenerateShapedAsteroid(
    position, size: 50f, material: "Iron",
    sdfFunction: (pos) => NoiseGenerator.SDF_Sphere(pos, Vector3.Zero, 25f),
    resolution: 8
);
```

**Features:**
- Procedural irregular asteroid shapes using noise
- Resource vein generation (primary, secondary, tertiary resources)
- Multiple generation strategies (full, simple, custom SDF)
- Configurable voxel resolution

### 2. Dynamic Chunk Management

#### ChunkManager
**Location:** `AvorionLike/Core/Procedural/ChunkManager.cs`

Manages dynamic loading/unloading of voxel chunks based on player position:

```csharp
var chunkManager = new ChunkManager(
    chunkSize: 100,
    loadRadius: 500f,
    unloadRadius: 750f,
    maxLoadedChunks: 100
);

// Update based on player position (call every frame/tick)
chunkManager.Update(playerPosition);

// Add voxel blocks to appropriate chunks
chunkManager.AddBlock(voxelBlock);

// Get all loaded chunks
var chunks = chunkManager.GetLoadedChunks();

// Get dirty chunks (need mesh rebuild)
var dirtyChunks = chunkManager.GetDirtyChunks();

// Get statistics
var stats = chunkManager.GetStats();
```

**Features:**
- Automatic chunk loading/unloading based on distance
- Spatial partitioning for efficient block storage
- Dirty chunk tracking for mesh rebuilding
- LRU-based chunk limit enforcement
- Thread-safe concurrent access

### 3. Multithreaded World Generation

#### ThreadedWorldGenerator
**Location:** `AvorionLike/Core/Procedural/ThreadedWorldGenerator.cs`

Generates world content on background threads to prevent gameplay hitching:

```csharp
var worldGen = new ThreadedWorldGenerator(
    seed: 12345,
    chunkManager: chunkManager,
    threadCount: 0 // Auto-detect CPU cores
);

// Start worker threads
worldGen.Start();

// Queue generation tasks
worldGen.RequestSectorGeneration(x: 0, y: 0, z: 0);
worldGen.RequestAsteroidGeneration(asteroidData, resolution: 8);

// Process results on main thread
worldGen.ProcessResults(); // Call every frame

// Get statistics
int pendingTasks = worldGen.GetPendingTaskCount();
int pendingResults = worldGen.GetPendingResultCount();

// Shutdown when done
worldGen.Stop();
```

**Features:**
- Multi-threaded generation (configurable thread count)
- Task queue system
- Result queue for main thread processing
- Sector and asteroid generation
- Prevents gameplay stuttering

### 4. Optimized Voxel Meshing

#### GreedyMeshBuilder
**Location:** `AvorionLike/Core/Graphics/GreedyMeshBuilder.cs`

Generates optimized meshes from voxel blocks with face culling:

```csharp
// Build mesh with face culling (only render visible faces)
var mesh = GreedyMeshBuilder.BuildMesh(voxelBlocks);

// Build with greedy meshing (combines adjacent faces)
var greedyMesh = GreedyMeshBuilder.BuildGreedyMesh(voxelBlocks);

// Access mesh data
int vertexCount = mesh.VertexCount;
int faceCount = mesh.FaceCount;
var vertices = mesh.Vertices; // List<Vector3>
var normals = mesh.Normals;   // List<Vector3>
var colors = mesh.Colors;     // List<uint>
var indices = mesh.Indices;   // List<int>
```

**Features:**
- Face culling (hidden faces not generated)
- Neighbor detection for internal face removal
- Greedy meshing algorithm (future enhancement)
- Per-vertex color support
- Normal generation for lighting

**Performance:**
- Reduces polygon count by ~80% with face culling
- Significantly improves rendering performance
- Enables rendering of large voxel structures

#### ThreadedMeshBuilder
**Location:** `AvorionLike/Core/Graphics/ThreadedMeshBuilder.cs`

Builds meshes on background threads:

```csharp
var meshBuilder = new ThreadedMeshBuilder(threadCount: 2);
meshBuilder.Start();

// Request mesh build for chunk
meshBuilder.RequestMeshBuild(chunk, useGreedyMeshing: true);

// Process results on main thread
var results = meshBuilder.ProcessResults();
foreach (var result in results)
{
    if (result.Success)
    {
        var mesh = result.Mesh;
        // Upload to GPU...
    }
}

// Shutdown
meshBuilder.Stop();
```

**Features:**
- Background mesh generation
- Prevents frame rate drops during mesh building
- Configurable thread count
- Error handling

### 5. Lighting & Visual Effects

#### LightingSystem
**Location:** `AvorionLike/Core/Graphics/LightingSystem.cs`

Manages dynamic lighting in space:

```csharp
var lightingSystem = new LightingSystem();

// Create star light
var starLight = LightingSystem.CreateStarLight(
    position: new Vector3(5000, 5000, 5000),
    color: new Vector3(1.0f, 0.95f, 0.8f),
    intensity: 2.0f
);
lightingSystem.AddLight(starLight);

// Create nebula light (soft area light)
var nebulaLight = LightingSystem.CreateNebulaLight(
    position: new Vector3(0, 1000, 0),
    color: new Vector3(0.5f, 0.2f, 0.8f),
    intensity: 0.5f
);
lightingSystem.AddLight(nebulaLight);

// Calculate lighting at a point
Vector3 lighting = lightingSystem.CalculateLighting(position, normal);

// Set ambient color
lightingSystem.SetAmbientColor(new Vector3(0.1f, 0.1f, 0.15f));
```

**Light Types:**
- Point lights (stars, explosions)
- Directional lights (distant stars)
- Spot lights (searchlights)
- Area lights (nebulae)

#### VisualEffectsSystem
**Location:** `AvorionLike/Core/Graphics/LightingSystem.cs`

Creates visual effects for gameplay events:

```csharp
var effectsSystem = new VisualEffectsSystem();

// Create mining beam
var beam = effectsSystem.CreateMiningBeam(
    start: shipPosition,
    end: asteroidPosition,
    color: new Vector3(0, 1, 1) // Cyan
);

// Create explosion
var explosion = effectsSystem.CreateExplosion(
    position: explosionPos,
    radius: 50f,
    color: new Vector3(1, 0.5f, 0) // Orange
);

// Create resource glow (glowy effect for resource-rich areas)
var glow = effectsSystem.CreateResourceGlow(
    position: resourcePos,
    color: new Vector3(1, 1, 0), // Yellow
    intensity: 1.0f
);

// Update effects (call every frame)
effectsSystem.Update(deltaTime);

// Stop an effect
effectsSystem.StopEffect(beam);
```

**Effect Types:**
- Mining beams (continuous)
- Explosions (animated)
- Resource glows (pulsing)

### 6. Voxel-Based Destruction

#### DestructionSystem
**Location:** `AvorionLike/Core/Combat/DestructionSystem.cs`

Handles dynamic destruction of voxel structures:

```csharp
var destructionSystem = new DestructionSystem(entityManager, eventSystem);

// Damage a specific block
destructionSystem.DamageBlock(entityId, block, damage: 100f);

// Damage in radius (explosion)
destructionSystem.DamageRadius(
    position: explosionCenter,
    radius: 50f,
    damage: 200f
);

// Damage along ray (laser/projectile)
destructionSystem.DamageRay(
    start: weaponPosition,
    direction: Vector3.Normalize(targetPos - weaponPosition),
    length: 100f,
    damage: 50f
);

// Create debris from destroyed blocks
destructionSystem.CreateDebris(entityId, block, velocity);

// Update (processes pending destructions)
destructionSystem.Update(deltaTime);
```

**Features:**
- Block-by-block destruction
- Radius damage (explosions)
- Ray damage (lasers, projectiles)
- Debris generation
- Entity property updates (mass, thrust, etc.)
- Event publishing for destruction

### 7. Integration Managers

#### WorldManager
**Location:** `AvorionLike/Core/Procedural/WorldManager.cs`

Integrates world generation, chunk management, and mining:

```csharp
var worldManager = new WorldManager(
    entityManager,
    miningSystem,
    seed: 12345,
    chunkSize: 100
);

// Update (call every frame)
worldManager.Update(deltaTime, playerPosition);

// Generate asteroids in area
worldManager.GenerateAsteroidsInRadius(position, radius: 500f);

// Generate specific sector
worldManager.GenerateSector(x: 0, y: 0, z: 0);

// Get statistics
var stats = worldManager.GetStats();

// Shutdown
worldManager.Shutdown();
```

#### RenderingManager
**Location:** `AvorionLike/Core/Graphics/RenderingManager.cs`

Integrates chunk management with mesh building:

```csharp
var renderingManager = new RenderingManager(
    chunkManager,
    meshThreadCount: 2
);

// Update (call every frame)
renderingManager.Update();

// Get renderable meshes
foreach (var (chunk, mesh) in renderingManager.GetRenderableMeshes())
{
    // Render mesh...
}

// Toggle greedy meshing
renderingManager.SetGreedyMeshing(enabled: true);

// Get statistics
var stats = renderingManager.GetStats();

// Shutdown
renderingManager.Shutdown();
```

## Usage Example

See `AvorionLike/Examples/SpaceExplorationExample.cs` for a comprehensive example:

```csharp
var example = new SpaceExplorationExample();
example.RunExample();
```

This example demonstrates:
1. System initialization
2. Player ship creation with voxels
3. World generation around player
4. Physics simulation
5. Mining operations
6. Destruction mechanics
7. Statistics reporting

## Performance Considerations

### Chunk Size
- Smaller chunks (50-100): Better culling, more overhead
- Larger chunks (100-200): Less overhead, worse culling
- Recommended: 100 for balanced performance

### Thread Count
- World Generation: Use CPU cores - 1 (leave one for main thread)
- Mesh Building: Use CPU cores / 2 (share with other tasks)

### Voxel Resolution
- Low (4-6): Fast generation, blocky appearance
- Medium (8-10): Balanced
- High (12-16): Detailed, slow generation

### Update Intervals
- Chunk updates: 1 second (throttled)
- Mesh building: Every frame
- World generation: Continuous background

## Best Practices

1. **Always run generation on background threads** - Use ThreadedWorldGenerator
2. **Throttle chunk updates** - Don't update every frame
3. **Use face culling** - Reduces polygon count by ~80%
4. **Limit loaded chunks** - Set maxLoadedChunks appropriately
5. **Process results gradually** - Limit processing per frame to avoid hitching
6. **Use appropriate voxel resolution** - Balance detail vs performance

## Troubleshooting

### Poor Performance
- Reduce voxel resolution
- Increase chunk size
- Reduce load radius
- Enable greedy meshing
- Reduce thread count (oversubscription)

### Stuttering
- Increase update intervals
- Reduce max processing per frame
- Check thread synchronization

### Memory Issues
- Reduce maxLoadedChunks
- Increase unload radius gap
- Clear old meshes regularly

## Future Enhancements

1. **Advanced Greedy Meshing** - Full implementation for better optimization
2. **LOD System** - Multiple detail levels based on distance
3. **Texture Atlas** - Proper texture support instead of colors
4. **Shadow Mapping** - Dynamic shadows
5. **Occlusion Culling** - Additional rendering optimization
6. **Streaming** - Save/load chunks from disk
7. **Network Synchronization** - Multiplayer chunk streaming

## API Reference

See individual class documentation in source files for detailed API reference.

## Related Systems

- **Physics System** (`AvorionLike/Core/Physics/`) - Handles movement and collisions
- **Mining System** (`AvorionLike/Core/Mining/`) - Resource extraction
- **ECS** (`AvorionLike/Core/ECS/`) - Entity management
- **Events** (`AvorionLike/Core/Events/`) - Event publishing

## Credits

Inspired by Avorion's voxel-based ship building and procedural galaxy generation.
