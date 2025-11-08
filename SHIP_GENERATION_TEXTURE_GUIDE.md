# Procedural Ship Generation & Texture System Guide

## Overview

This document describes the comprehensive procedural ship generation and texture system implemented for Codename-Subspace, inspired by Avorion's approach of balancing algorithmic creativity with functional player control.

## Ship Generation System

### Core Philosophy

The ship generation system follows Avorion's principle of **"Function Over Form"**:

1. **Functional First**: Generated ships prioritize working systems (engines, shields, generators) over pure aesthetics
2. **Faction Styles**: Each faction has a distinct visual style that guides ship appearance
3. **Block-Based**: Ships are constructed from individual voxel blocks with physical properties
4. **Customizable**: Generated ships serve as starting points that players can extensively modify

### System Components

#### 1. FactionShipStyle (`FactionShipStyle.cs`)

Defines visual and structural characteristics for faction ships:

```csharp
public class FactionShipStyle
{
    public ShipHullShape PreferredHullShape;    // Blocky, Angular, Cylindrical, etc.
    public float SymmetryLevel;                  // 0-1, how symmetric ships are
    public float Sleekness;                      // 0-1, streamlined vs blocky
    public uint PrimaryColor;                    // Main hull color
    public uint SecondaryColor;                  // Accent color
    public float ArmorToHullRatio;              // Armor coverage
    public float ExternalSystemsPreference;      // Exposed vs internal systems
    public float WeaponDensity;                 // How many weapons
    public DesignPhilosophy Philosophy;          // Combat, Utility, Speed-focused, etc.
}
```

**Predefined Faction Styles:**

- **Military**: Angular, armored, weapon-heavy, high symmetry
- **Traders**: Cylindrical, cargo-focused, practical, efficient
- **Pirates**: Irregular, asymmetric, cobbled-together, aggressive
- **Scientists**: Sleek, sensor-heavy, minimal weapons, exploratory
- **Miners**: Blocky, industrial, utility-focused, robust

#### 2. ProceduralShipGenerator (`ProceduralShipGenerator.cs`)

Main generator that creates complete ships from configuration:

**Ship Sizes:**
- Fighter (10-20 blocks)
- Corvette (20-50 blocks)
- Frigate (50-100 blocks)
- Destroyer (100-200 blocks)
- Cruiser (200-400 blocks)
- Battleship (400-800 blocks)
- Carrier (800+ blocks)

**Ship Roles:**
- Multipurpose (balanced)
- Combat (heavy weapons, strong shields)
- Mining (mining lasers, large cargo)
- Trading (huge cargo, minimal weapons)
- Exploration (sensors, hyperdrive)
- Salvage (salvage beams, cargo)

**Generation Pipeline:**

1. **Determine Dimensions**: Calculate ship size based on size category
2. **Generate Hull**: Create basic structure using faction's preferred hull shape
3. **Place Functional Components**: Add engines, generators, shields, thrusters, gyros
4. **Add Weapons**: Place turret mounts based on role and weapon density
5. **Add Utility Blocks**: Cargo, hyperdrive, crew quarters, docking
6. **Apply Armor**: Convert outer hull blocks to armor based on faction style
7. **Color Scheme**: Apply faction colors to different block types
8. **Calculate Stats**: Compute thrust, power, shields, mass, etc.
9. **Validate**: Ensure ship has minimum functional requirements

**Hull Shapes:**

```csharp
public enum ShipHullShape
{
    Blocky,        // Box-like, brick-shaped (functional, simple)
    Angular,       // Wedge-shaped, military style
    Cylindrical,   // Tube-like, cargo-focused
    Sleek,         // Streamlined, exploration ships
    Irregular,     // Asymmetric, pirate ships
    Organic        // Curved, alien designs
}
```

**Example Usage:**

```csharp
var generator = new ProceduralShipGenerator(seed: 12345);

var config = new ShipGenerationConfig
{
    Size = ShipSize.Frigate,
    Role = ShipRole.Combat,
    Material = "Titanium",
    Style = FactionShipStyle.GetDefaultStyle("Military"),
    RequireHyperdrive = true,
    RequireCargo = true,
    MinimumWeaponMounts = 4
};

GeneratedShip ship = generator.GenerateShip(config);

// Ship contains:
// - Complete voxel structure
// - Functional components (engines, shields, etc.)
// - Weapon mounts
// - Statistics (mass, thrust, power)
// - Validation warnings
```

### Functional Component Placement

The generator ensures ships are functional by placing required components:

**Engines** (rear of ship):
- Linear thrust for forward movement
- Count scales with ship size
- Placed at back for realistic thrust vector

**Generators** (core/center):
- Power generation for all systems
- Protected in ship's interior
- Count based on ship size and power needs

**Shield Generators** (distributed):
- Defensive shields
- Placed throughout ship for coverage
- Extra shields for combat ships

**Thrusters** (all sides):
- Omnidirectional movement (strafing, braking)
- Distributed on top, bottom, left, right
- Essential for space maneuverability

**Gyro Arrays** (internal):
- Rotational control (pitch, yaw, roll)
- Placed strategically for balance
- More gyros for larger ships

**Weapon Mounts**:
- Distributed around hull for coverage
- Count based on role and faction weapon density
- Combat ships get significantly more weapons

**Utility Blocks**:
- Hyperdrive (for sector jumping)
- Cargo holds (scaled to role - traders get most)
- Crew quarters (for larger ships)
- Pod docking (player interface)

### Validation & Statistics

Generated ships are validated to ensure functionality:

- ✅ Minimum thrust (can move)
- ✅ Power generation (systems can function)
- ✅ Weapon requirements met
- ✅ Thrust-to-mass ratio (ship isn't too sluggish)
- ⚠️ Warnings generated for deficiencies

**Statistics Calculated:**
- Total Mass (kg)
- Total Thrust (N)
- Power Generation (W)
- Shield Capacity
- Weapon Count
- Cargo Capacity
- Thrust-to-Mass Ratio

## Texture Generation System

### Overview

The texture system uses procedural generation to create diverse, coherent textures for the voxel universe without requiring large texture atlases.

### Key Principles

1. **Tiling Base Textures**: Virtual 16x16 or 32x32 pixel-art style textures
2. **Material IDs**: Each voxel type has associated material properties
3. **Procedural Shading**: Noise functions add variation and detail
4. **World-Space Coordinates**: Textures generated based on position in world
5. **Color Palettes**: Cohesive color schemes for celestial bodies
6. **Contextual Variation**: Textures adapt to environment (altitude, temperature, etc.)

### System Components

#### 1. TextureMaterial (`TextureMaterial.cs`)

Defines material properties for procedural texture generation:

```csharp
public class TextureMaterial
{
    // Base colors
    public Vector3 BaseColor;
    public Vector3 SecondaryColor;
    
    // Physical properties (PBR)
    public float Roughness;      // 0=shiny, 1=matte
    public float Metallic;       // 0=dielectric, 1=metallic
    public float Emissive;       // 0=no glow, 1=full glow
    
    // Procedural properties
    public float NoiseScale;     // Scale of noise texture
    public float NoiseStrength;  // How much noise affects color
    public float BumpStrength;   // Normal map intensity
    
    // Pattern
    public TexturePattern Pattern;
    public float PatternScale;
    
    // Special
    public float Opacity;        // Transparency
    public bool Animated;        // Time-based animation
    public float AnimationSpeed;
}
```

**Available Materials:**
- **Structural**: Hull, Armor
- **Natural**: Rock, Ice, Grass, Sand, Snow, Water, Lava
- **Industrial**: Metal, Titanium, Naonite, Crystal
- **Environmental**: GasCloud, Nebula, Plasma
- **Special**: Energy, Shield, Glow

**Texture Patterns:**
- Uniform (solid with noise)
- Striped (linear bands)
- Banded (concentric circles - gas giants)
- Paneled (hull panels with rivets)
- Hexagonal (hex grid pattern)
- Cracked (fractured surface)
- Crystalline (faceted crystals)
- Swirled (turbulent flow)
- Spotted (random patches)
- Weathered (wear and tear)

#### 2. ProceduralTextureGenerator (`ProceduralTextureGenerator.cs`)

Core noise-based texture generation:

**Noise Functions:**
- **Perlin Noise 3D**: Smooth, natural-looking noise
- **Voronoi/Worley Noise**: Cellular patterns (crystals, spots)
- **Multi-Octave Noise**: Layered noise for detail
- **Domain Warping**: Creates turbulent, swirling effects

**Example Usage:**

```csharp
var generator = new ProceduralTextureGenerator(seed: 12345);
var material = MaterialLibrary.GetMaterial(MaterialType.Hull);

// Generate color for a voxel at world position
Vector3 color = generator.GenerateTextureColor(
    worldPosition: new Vector3(10, 20, 30),
    material: material,
    time: 0f  // For animated materials
);

// Calculate bump/height for normal mapping
float bump = generator.CalculateBumpValue(worldPosition, material);
```

**Pattern Generation:**

Each pattern has specialized generation logic:

```csharp
// Paneled pattern (hull plates with rivets)
float gridX = Abs(worldPos.X % patternScale);
if (gridX < lineThickness) {
    // Panel line - darker
    color *= 0.7f;
}

// Hexagonal pattern
float hex = Sin(x) + Sin(x*0.5 + y*0.866) + Sin(x*0.5 - y*0.866);

// Cracked pattern (asteroids)
float crack1 = PerlinNoise(pos * scale * 2.0);
float crack2 = PerlinNoise(pos * scale * 4.0);
if (Abs(crack1) * Abs(crack2) < 0.1) {
    // Crack line
    color *= 0.5f;
}
```

**Weathering Effects:**

Industrial materials can have weathering applied:

- **Scorch Marks**: Dark, heat-damaged areas (near engines, battle damage)
- **Rust/Oxidation**: Orange-brown stains on metal
- **Scratches**: Surface damage patterns

#### 3. CelestialTextureGenerator (`CelestialTextureGenerator.cs`)

Specialized texture generation for celestial bodies:

**Color Palettes:**

Predefined palettes for different planet/object types:

```csharp
// Jupiter-like gas giant
Colors: [Cream, Light Orange, Dark Orange, Light Brown, Dark Brown]

// Neptune-like gas giant
Colors: [Deep Blue, Medium Blue, Light Blue, Cyan]

// Desert planet
Colors: [Orange Sand, Dark Orange, Light Sand, Reddish Rock]

// Earth-like planet
Colors: [Ocean Blue, Forest Green, Mountain Brown, Snow White]
```

**Gas Giant Generation:**

```csharp
Vector3 color = textureGen.GenerateGasGiantTexture(
    worldPos: new Vector3(x, y, z),
    paletteType: "jupiter",
    time: currentTime
);
```

Features:
- Latitude-based banding (horizontal stripes)
- Turbulent storm systems
- Great red spot / vortex features
- Atmospheric shimmer
- Time-based animation

**Rocky Planet Generation:**

```csharp
Vector3 color = textureGen.GenerateRockyPlanetTexture(
    worldPos: position,
    paletteType: "earthlike",
    altitude: 100f,
    temperature: 0.6f,
    moisture: 0.7f
);
```

Features:
- Altitude-based terrain (water, plains, mountains, snow)
- Temperature affects biomes (hot=desert, cold=ice)
- Moisture affects vegetation
- Smooth transitions via splatmapping

**Asteroid Generation:**

```csharp
Vector3 color = textureGen.GenerateAsteroidTexture(
    worldPos: position,
    resourceDensity: 0.7f  // 0-1, how much ore
);
```

Features:
- Rocky base texture
- Crater detection (darker, shadowed areas)
- Ore veins (metallic glints)
- Resource density affects ore frequency

**Nebula Generation:**

```csharp
Vector3 color = textureGen.GenerateNebulaTexture(
    worldPos: position,
    paletteType: "nebula_pink",
    time: currentTime
);
```

Features:
- Multi-layer turbulence
- Swirling, flowing clouds
- Self-illumination/glow
- Time-based animation
- Semi-transparent

**Station/Ship Hull Generation:**

```csharp
Vector3 color = textureGen.GenerateStationTexture(
    worldPos: position,
    baseColor: new Vector3(0.7f, 0.7f, 0.7f),
    addWeathering: true
);
```

Features:
- Hull panel grid
- Rivet details at corners
- Surface variation
- Optional weathering (scorch, rust)

#### 4. SplatmapManager

Blends multiple materials based on environmental factors:

```csharp
var weights = splatmap.CalculateBlendWeights(
    worldPos: position,
    altitude: 50f,
    temperature: 0.6f,
    moisture: 0.7f
);

// Returns: { Grass: 0.7, Rock: 0.3 }

Vector3 finalColor = splatmap.BlendMaterialColors(
    weights: weights,
    worldPos: position,
    generator: textureGen
);
```

**Blending Rules:**

- Altitude < 0: Water
- Altitude 0-50: Grass/Sand based on climate
- Altitude 50-200: Rock/Grass mix
- Altitude 200-400: Rock/Snow mix
- Altitude > 400: Snow peaks

- High temperature + low moisture = Desert
- Moderate temperature + high moisture = Grass/vegetation
- Low temperature = Ice/snow

### Visual Effects for Space Environments

Additional effects that enhance the voxel universe:

1. **Starfield Background**: Distant stars with parallax
2. **Bloom/Glow**: Emissive materials glow (engines, energy)
3. **Atmospheric Scattering**: Planets have atmospheric halos
4. **Particle Effects**: 
   - Engine exhaust trails
   - Weapon fire
   - Explosion debris
5. **Fog/Nebula Volumes**: Semi-transparent gas clouds
6. **Lighting**:
   - Sun/star directional lighting
   - Point lights from stations/ships
   - Emissive block self-illumination
7. **Post-Processing**:
   - Film grain for texture
   - Vignetting for focus
   - Color grading for mood
8. **Motion Effects**:
   - Motion blur for fast movement
   - Warp/hyperdrive effects

## Integration with Existing Systems

### With Build System

Players can modify generated ships using the existing BuildSystem:

```csharp
// Generate a ship
var generatedShip = generator.GenerateShip(config);

// Player enters build mode
buildSystem.StartBuildSession(shipEntity.Id);

// Player can:
// - Add/remove blocks
// - Change block types
// - Scale blocks (stretching)
// - Paint blocks
// - Add armor
// - Optimize placement
```

### With Faction System

Ship generation respects faction attributes:

```csharp
var faction = factionSystem.GetFaction("Military");
var style = FactionShipStyle.GetDefaultStyle(faction.Name);

// Style influenced by faction ethics:
// - Militarist: More weapons, heavier armor
// - Pacifist: Fewer weapons, more shields
// - Materialist: Advanced materials, efficient
// - Xenophile: Diplomatic, trading-focused
```

### With Combat System

Generated ships have proper combat stats:

- Weapon mounts → Turret placement
- Shield generators → Shield capacity
- Armor blocks → Damage resistance
- Engine placement → Maneuverability

### With Physics System

Ships are physically accurate:

- Mass calculated from all blocks
- Thrust vectors from engine positions
- Center of mass affects rotation
- Gyros provide torque

## Examples

### Example 1: Generate Military Frigate

```csharp
var generator = new ProceduralShipGenerator(seed: 12345);
var style = FactionShipStyle.GetDefaultStyle("Military");

var config = new ShipGenerationConfig
{
    Size = ShipSize.Frigate,
    Role = ShipRole.Combat,
    Material = "Titanium",
    Style = style
};

var ship = generator.GenerateShip(config);

Console.WriteLine($"Blocks: {ship.Structure.Blocks.Count}");
Console.WriteLine($"Mass: {ship.TotalMass} kg");
Console.WriteLine($"Thrust: {ship.TotalThrust} N");
Console.WriteLine($"Weapons: {ship.WeaponMountCount}");
```

Output:
```
Blocks: 87
Mass: 12,500 kg
Thrust: 18,000 N
Weapons: 8
Thrust/Mass: 1.44
```

### Example 2: Generate Planet Texture

```csharp
var textureGen = new CelestialTextureGenerator(seed: 67890);

// Earth-like planet surface
var color = textureGen.GenerateRockyPlanetTexture(
    worldPos: new Vector3(100, 50, 200),
    paletteType: "earthlike",
    altitude: 50f,      // Plains level
    temperature: 0.6f,  // Moderate
    moisture: 0.8f      // Wet
);

// Result: Green vegetation color
// RGB: (0.25, 0.52, 0.23)
```

### Example 3: Texture a Generated Ship

```csharp
var shipGen = new ProceduralShipGenerator(seed: 111);
var textureGen = new ProceduralTextureGenerator(seed: 111);

var ship = shipGen.GenerateShip(config);

// Apply textures to each block
foreach (var block in ship.Structure.Blocks)
{
    MaterialType matType = block.BlockType switch
    {
        BlockType.Hull => MaterialType.Hull,
        BlockType.Armor => MaterialType.Armor,
        BlockType.Engine => MaterialType.Metal,
        _ => MaterialType.Metal
    };
    
    var material = MaterialLibrary.GetMaterial(matType);
    var color = textureGen.GenerateTextureColor(
        block.Position, 
        material, 
        time: 0f
    );
    
    // Apply color to block for rendering
    block.ColorRGB = ColorToUInt(color);
}
```

## Running the Examples

Use the ShipGenerationExample class to see the system in action:

```csharp
var entityManager = new EntityManager();
var example = new ShipGenerationExample(entityManager, seed: 12345);

// Generate example ships
example.GenerateExampleShips();

// Show texture generation
example.DemonstrateTextureGeneration();

// Show material library
example.DemonstrateMaterialLibrary();

// Show ship with textures
example.DemonstrateShipWithTextures();

// List available palettes
example.ShowAvailablePalettes();
```

## Performance Considerations

1. **Texture Caching**: Generated textures should be cached per-block
2. **LOD (Level of Detail)**: Distant ships use simplified textures
3. **Instancing**: Identical blocks share texture data
4. **Lazy Generation**: Only generate textures for visible blocks
5. **Spatial Hashing**: Efficiently find nearby blocks for rendering

## Future Enhancements

1. **Blueprint System**: Save/load generated ships
2. **Ship Templates**: Pre-defined templates for quick generation
3. **Procedural Damage**: Realistic battle damage patterns
4. **Dynamic Textures**: Textures change based on ship state
5. **Material Tiers**: Higher-tier materials have better visuals
6. **Faction Variations**: Sub-styles within factions
7. **Modular Ships**: Generate ships in sections (bridge, engines, etc.)
8. **AI Designer**: Neural network learns player preferences

## Summary

The procedural ship generation and texture system provides:

✅ **Functional Ships**: Every ship can fly, fight, and function
✅ **Faction Diversity**: Each faction has distinct visual style
✅ **Extensive Customization**: Players can modify any generated ship
✅ **Rich Textures**: Procedural textures create varied, coherent visuals
✅ **Scalability**: System handles ships from fighters to carriers
✅ **Flexibility**: Easy to add new factions, materials, and patterns

The system balances Avorion's philosophy of "function over form" with the need for visual diversity, creating a universe that feels vast and varied while maintaining deep functional gameplay.
