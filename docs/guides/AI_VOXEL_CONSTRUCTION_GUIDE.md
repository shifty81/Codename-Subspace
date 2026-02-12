# AI-Driven Voxel Construction System

## Overview

This document describes the AI-driven procedural content generation system for voxel-based, modular ship construction with dynamic scaling. The system uses smart design rules to generate functional and aesthetically pleasing ships based on specific goals.

## Core Concepts

### 1. Voxel Grid System

The game world and all constructions are based on a grid of voxels (3D pixels). Each voxel can hold a specific type of block with various properties.

**Key Classes:**
- `VoxelBlock` - Individual voxel with position, size, and material properties
- `VoxelStructureComponent` - Collection of voxels forming a ship/station

### 2. Modular Design

Ships and stations are assembled from distinct functional modules. Each module type has specific properties.

**Block Types:**
- **Structural:** Hull, Armor
- **Propulsion:** Engine, Thruster, GyroArray
- **Power:** Generator
- **Defense:** ShieldGenerator
- **Weapons:** TurretMount
- **Utility:** Cargo, CrewQuarters, HyperdriveCore, PodDocking

### 3. Dynamic Scaling/Stretching

Blocks are not fixed size; they are primitive shapes (cubes, wedges) that can be scaled along any axis (X, Y, Z). This allows for complex, non-uniform shapes without massive block counts.

**Features:**
- Scale blocks along individual axes
- Size affects mass, HP, and functional properties proportionally
- Visual representation stretches seamlessly

### 4. Functional Properties

Ship performance is directly tied to block type, size, and placement.

**Performance Metrics:**
- **Thrust:** Engines and thrusters provide movement
- **Torque:** Gyro arrays control rotation
- **Power:** Generators provide energy, systems consume it
- **Defense:** Shields and armor protect the ship
- **Cargo:** Storage capacity for resources
- **Crew:** Required for ship operation

## Data Structure & Asset Definition

### Block Definition System

The `BlockDefinition` class provides comprehensive block properties:

```csharp
public class BlockDefinition
{
    public string Id { get; set; }
    public string DisplayName { get; set; }
    public BlockType BlockType { get; set; }
    public string Description { get; set; }
    
    // Resource costs per unit volume
    public Dictionary<string, int> ResourceCosts { get; set; }
    
    // Physical properties
    public float HitPointsPerVolume { get; set; }
    public float MassPerUnitVolume { get; set; }
    public bool Scalable { get; set; }
    
    // Functional properties
    public string Function { get; set; }
    public float PowerGenerationPerVolume { get; set; }
    public float PowerConsumptionPerVolume { get; set; }
    public float ThrustPowerPerVolume { get; set; }
    public float ShieldCapacityPerVolume { get; set; }
    public float CargoCapacityPerVolume { get; set; }
    
    // AI placement hints
    public int AiPlacementPriority { get; set; }
    public bool RequiresInternalPlacement { get; set; }
    public bool SuitableForExterior { get; set; }
}
```

### Block Definition Database

The `BlockDefinitionDatabase` class manages all block definitions:

```csharp
// Get all definitions
var definitions = BlockDefinitionDatabase.GetDefinitions();

// Get specific definition
var engineDef = BlockDefinitionDatabase.GetDefinition(BlockType.Engine);

// Export to JSON for modding/configuration
BlockDefinitionDatabase.ExportToJson("block_definitions.json");

// Import from JSON
BlockDefinitionDatabase.ImportFromJson("block_definitions.json");
```

### JSON Export Example

```json
{
  "id": "engine_main",
  "displayName": "Main Engine",
  "blockType": "Engine",
  "description": "Primary propulsion engine providing forward thrust",
  "resourceCosts": {
    "Iron": 20,
    "Titanium": 5
  },
  "hitPointsPerVolume": 80.0,
  "massPerUnitVolume": 1.2,
  "scalable": true,
  "function": "generateThrust",
  "thrustPowerPerVolume": 50.0,
  "powerConsumptionPerVolume": 5.0,
  "aiPlacementPriority": 10,
  "requiresInternalPlacement": false,
  "suitableForExterior": true
}
```

## Ship Aggregate System

The `ShipAggregate` class calculates all ship properties from voxel blocks:

```csharp
var structure = new VoxelStructureComponent();
// ... add blocks ...

var aggregate = new ShipAggregate(structure);

// Access calculated properties
float totalMass = aggregate.TotalMass;
float maxSpeed = aggregate.MaxSpeed;
float powerEfficiency = aggregate.PowerEfficiency;
float combatRating = aggregate.CombatEffectivenessRating;

// Get comprehensive stats
string statsSummary = aggregate.GetStatsSummary();

// Validate ship is functional
List<string> warnings = aggregate.ValidateRequirements();
```

### Calculated Properties

**Structural:**
- Total mass
- Total hit points
- Center of mass
- Moment of inertia
- Structural integrity percentage

**Power System:**
- Total power generation
- Total power consumption
- Available power
- Power efficiency

**Propulsion:**
- Total thrust
- Total torque
- Max speed (thrust-to-mass ratio)
- Max rotation speed (torque-to-inertia ratio)
- Acceleration

**Defense:**
- Shield capacity
- Armor points

**Utility:**
- Cargo capacity
- Crew capacity
- Weapon mount count

**Ratings (0-100):**
- Maneuverability rating
- Combat effectiveness rating
- Cargo efficiency rating

## AI Generation System

The `AIShipGenerator` uses smart design rules to create functional ships.

### Design Goals

Ships can be generated for specific purposes:

```csharp
public enum ShipDesignGoal
{
    CargoHauler,    // Maximize cargo capacity
    Battleship,     // Heavy armor, weapons, shields
    Scout,          // High speed, maneuverability
    Miner,          // Mining equipment, cargo
    Interceptor,    // Fast, agile, light weapons
    Carrier,        // Large, pod docking, defenses
    Tanker,         // Maximum cargo, minimal combat
    Frigate         // Balanced multi-role
}
```

### AI Generation Parameters

```csharp
var parameters = new AIShipGenerationParameters
{
    Goal = ShipDesignGoal.Battleship,
    Material = "Iron",
    TargetBlockCount = 150,
    Seed = 12345,
    
    // Design constraints
    MinWeaponMounts = 6,
    RequireHyperdrive = true,
    RequireShields = true,
    MinCrewCapacity = 10,
    
    // Aesthetic preferences
    AvoidSimpleBoxes = true,
    DesiredAspectRatio = 2.5f,
    UseAngularDesign = true
};
```

### Generation Process

The AI follows a systematic approach:

1. **Determine Dimensions** - Calculate optimal ship size based on goal and block count
2. **Create Placement Plan** - Prioritize blocks based on design goal
3. **Define Ship Outline** - Use block-out method to create shape framework
4. **Place Internal Components** - Add critical systems (generators, crew) in protected areas
5. **Place Functional Systems** - Add engines, weapons, etc. in optimal positions
6. **Add Armor Shell** - Create protective external layer
7. **Optimize Design** - Remove disconnected blocks, validate connectivity
8. **Calculate Statistics** - Aggregate all properties
9. **Validate Requirements** - Check for functional issues
10. **Rate Quality** - Score the design (0-100)

### Using the AI Generator

```csharp
var generator = new AIShipGenerator(seed: 12345);

var parameters = new AIShipGenerationParameters
{
    Goal = ShipDesignGoal.Scout,
    Material = "Titanium",
    TargetBlockCount = 80,
    RequireHyperdrive = true
};

AIGeneratedShip result = generator.GenerateShip(parameters);

// Access the generated ship
VoxelStructureComponent structure = result.Structure;
ShipAggregate stats = result.Aggregate;
float quality = result.DesignQuality;

// Review design decisions
foreach (var decision in result.DesignDecisions)
{
    Console.WriteLine(decision);
}

// Check for issues
foreach (var warning in result.Warnings)
{
    Console.WriteLine($"Warning: {warning}");
}
```

## AI Design Rules

### Goal-Oriented Design

The AI prioritizes different block types based on the ship's purpose:

**Cargo Hauler:**
- Priority: Cargo (10), Generator (9), Engine (8)
- Maximizes cargo space and efficient propulsion
- Minimal weapons and armor

**Battleship:**
- Priority: Weapons (10), Armor (10), Generator (10), Shields (9)
- Heavy defenses and firepower
- Reduced cargo and speed

**Scout:**
- Priority: Engine (10), Thruster (10), Hyperdrive (10), Gyro (9)
- Maximum speed and maneuverability
- Minimal mass and good jump capability

### Block Placement Strategy

**Internal Components (Protected):**
- Generators
- Shield generators
- Crew quarters
- Cargo bays
- Gyro arrays
- Hyperdrive cores

Placed in central protected area, surrounded by structure and armor.

**External Components:**
- Engines (rear)
- Thrusters (distributed)
- Weapon mounts (top/sides for coverage)
- Armor (outer shell)

**Strategic Placement:**
- Engines at rear for forward thrust
- Gyros near center of mass for efficient rotation
- Weapons positioned for coverage
- Armor on exterior for protection

### Aesthetic Guidelines

To avoid simple box ships:

1. **Block-Out Method** - Define silhouette first, then fill
2. **Aspect Ratios** - Ships longer than wide (1.5x to 3x)
3. **Angular Design** - Use varied block sizes and shapes
4. **Layered Structure** - Frame → Internal → Functional → Armor
5. **Connected Design** - Remove isolated blocks

## Example Usage

### Simple Ship Building

```csharp
var structure = new VoxelStructureComponent();

// Add hull
structure.AddBlock(new VoxelBlock(
    position: new Vector3(0, 0, 0),
    size: new Vector3(2, 2, 2),
    material: "Iron",
    blockType: BlockType.Hull
));

// Add engine
structure.AddBlock(new VoxelBlock(
    position: new Vector3(0, 0, -5),
    size: new Vector3(3, 2, 2), // Scaled wider
    material: "Iron",
    blockType: BlockType.Engine
));

// Calculate properties
var aggregate = new ShipAggregate(structure);
Console.WriteLine(aggregate.GetStatsSummary());
```

### AI Generation Example

```csharp
// Create generator
var generator = new AIShipGenerator(12345);

// Set parameters
var parameters = new AIShipGenerationParameters
{
    Goal = ShipDesignGoal.Frigate,
    Material = "Titanium",
    TargetBlockCount = 120,
    MinWeaponMounts = 4,
    RequireHyperdrive = true,
    AvoidSimpleBoxes = true
};

// Generate ship
var result = generator.GenerateShip(parameters);

// Use the ship
var myShip = result.Structure;
Console.WriteLine($"Generated {myShip.Blocks.Count} block ship");
Console.WriteLine($"Quality: {result.DesignQuality}/100");
Console.WriteLine(result.Aggregate.GetStatsSummary());
```

### Running the Demo

```csharp
// Run comprehensive demonstration
AIShipGenerationExample.RunComprehensiveTest();
```

This will:
1. Show all block definitions and export to JSON
2. Demonstrate ship aggregate calculations
3. Generate ships for different goals (Scout, Cargo, Battleship, Frigate)
4. Display statistics and analysis

## Integration with Game

### Adding to Game Loop

```csharp
// In your game initialization:
var generator = new AIShipGenerator();

// When spawning enemy ships:
var enemyParams = new AIShipGenerationParameters
{
    Goal = ShipDesignGoal.Battleship,
    Material = "Titanium",
    TargetBlockCount = 150
};
var enemyShip = generator.GenerateShip(enemyParams);

// Add to entity system
var entity = entityManager.CreateEntity("Enemy Battleship");
entityManager.AddComponent(entity.Id, enemyShip.Structure);
```

### Dynamic Scaling in Build Mode

The existing `BuildSystem` already supports dynamic scaling:

```csharp
buildSystem.StartBlockStretch(shipId, startPosition, BlockStretchAxis.X);
Vector3 newSize = buildSystem.UpdateBlockStretch(shipId, currentPosition);
buildSystem.FinishBlockStretch(shipId, inventory);
```

## Performance Considerations

### Block Count Guidelines

- **Fighter:** 50-100 blocks
- **Frigate:** 100-200 blocks
- **Cruiser:** 200-400 blocks
- **Battleship:** 400-800 blocks

### Optimization Tips

1. Use standard 2x2x2 block size for consistent spacing
2. Limit to ~500 blocks per ship for good performance
3. Cache ship aggregate calculations
4. Use block stretching instead of many small blocks
5. Validate connectivity to avoid orphaned blocks

## Modding Support

### Custom Block Definitions

Mod creators can define new block types in JSON:

```json
{
  "id": "plasma_cannon",
  "displayName": "Plasma Cannon",
  "blockType": "TurretMount",
  "resourceCosts": {
    "Xanion": 50,
    "Avorion": 10
  },
  "hitPointsPerVolume": 200,
  "massPerUnitVolume": 1.8,
  "function": "mountWeapon",
  "aiPlacementPriority": 9,
  "minTechLevel": 5
}
```

Load custom definitions:
```csharp
BlockDefinitionDatabase.ImportFromJson("mods/my_mod/blocks.json");
```

### Custom Ship Templates

AI generation parameters can be saved and shared:

```csharp
// Save template
var template = new AIShipGenerationParameters
{
    Goal = ShipDesignGoal.Battleship,
    // ... configure ...
};
string json = JsonSerializer.Serialize(template);
File.WriteAllText("templates/battleship.json", json);

// Load template
string json = File.ReadAllText("templates/battleship.json");
var template = JsonSerializer.Deserialize<AIShipGenerationParameters>(json);
var ship = generator.GenerateShip(template);
```

## Future Enhancements

1. **Machine Learning** - Train AI on player-designed ships
2. **Genetic Algorithms** - Evolve optimal designs
3. **Weapon Hardpoints** - Specific mount points for weapons
4. **Module Templates** - Reusable component groups
5. **Visual Themes** - Faction-specific aesthetics
6. 3D model export** - Convert to standard formats
7. **Blueprint System** - Save and share designs
8. **Performance Profiling** - Simulate combat effectiveness

## Troubleshooting

### Ship has insufficient power
- Add more generators
- Reduce power-hungry systems
- Use higher-tier materials with better efficiency

### Ship is too slow
- Add more engines/thrusters
- Reduce mass (less armor, smaller blocks)
- Improve thrust-to-mass ratio

### Ship lacks maneuverability
- Add gyro arrays near center of mass
- Reduce moment of inertia (compact design)
- Add more thrusters

### Design quality is low
- Check warnings in result.Warnings
- Ensure all requirements are met
- Balance block types for goal
- Add proper armor and defenses

## Summary

The AI-driven voxel construction system provides:

✅ **Data-driven block definitions** with JSON export/import
✅ **Comprehensive ship aggregation** calculating all properties
✅ **Smart AI generation** with goal-oriented design rules
✅ **Dynamic scaling** for complex shapes
✅ **Functional validation** ensuring ships work properly
✅ **Quality rating** to evaluate designs
✅ **Modding support** through JSON configuration
✅ **Complete examples** demonstrating all features

The system enables both manual building and AI-driven procedural generation, creating functional and aesthetically interesting ships for any purpose.
