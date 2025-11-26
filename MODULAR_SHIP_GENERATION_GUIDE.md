# Modular Ship Generation Guide

## Overview

This guide covers the NMS-style modular ship generation system, which builds ships from pre-designed "modules" that snap together rather than generating ships block-by-block. This approach produces more visually distinct and coherent ships.

## Key Concepts

### Modules

Modules are pre-defined ship parts with:
- **Blocks**: The voxel structure of the module
- **Attachment Points**: Connection points for other modules
- **Properties**: Mass, thrust, power, cargo capacity, etc.

### Module Types

| Type | Description |
|------|-------------|
| `Cockpit` | Command center/bridge |
| `CoreHull` | Main body section |
| `EngineNacelle` | Engine pods |
| `Wing` | Wing attachments |
| `CargoContainer` | Cargo storage |
| `WeaponMount` | Turret hardpoints |
| `MidSection` | Connecting sections |
| `SensorArray` | Sensor dishes |

### Module Styles

Each module type has multiple visual variants:

| Style | Characteristics |
|-------|-----------------|
| `Military` | Angular, armored, aggressive |
| `Industrial` | Blocky, utilitarian, exposed frame |
| `Sleek` | Streamlined, curved, elegant |
| `Pirate` | Cobbled, asymmetric, worn |
| `Ancient` | Mysterious, geometric |
| `Organic` | Flowing, biological-inspired |
| `Civilian` | Simple, clean, practical |

## Ship Archetypes

Pre-defined ship configurations:

| Archetype | Size | Role | Engines | Wings | Weapons |
|-----------|------|------|---------|-------|---------|
| Fighter | Fighter | Combat | 1 | Yes | 2 |
| Heavy Fighter | Fighter | Combat | 2 | Yes | 4 |
| Hauler | Frigate | Trading | 2 | No | 1 |
| Explorer | Corvette | Exploration | 1 | Yes | 1 |
| Shuttle | Fighter | Multipurpose | 1 | No | 0 |
| Corvette | Corvette | Combat | 2 | Yes | 4 |
| Frigate | Frigate | Multipurpose | 2 | No | 4 |
| Raider | Corvette | Combat | 2 | Yes | 4 |
| Miner | Frigate | Mining | 2 | No | 2 |

## Usage

### Basic Ship Generation

```csharp
var generator = new ModularShipGenerator(seed);

// Generate from archetype
var fighter = generator.GenerateModularShip("fighter");

// Generate with specific style
var militaryHauler = generator.GenerateModularShip("hauler", ModuleStyle.Military);

// Generate completely random
var randomShip = generator.GenerateRandomModularShip();
```

### Creating Custom Modules

```csharp
// Create a custom cockpit
var cockpit = new ShipModule
{
    Id = "custom_cockpit_001",
    Name = "Custom Cockpit",
    Type = ModuleType.Cockpit,
    Style = ModuleStyle.Military,
    Mass = 50f,
    PowerConsumption = 5f
};

// Add blocks
cockpit.Blocks.Add(new VoxelBlock(
    position: new Vector3(0, 0, 0),
    size: new Vector3(4, 3, 4),
    material: "Titanium",
    blockType: BlockType.Hull
));

// Add attachment point
cockpit.AttachmentPoints.Add(new AttachmentPoint
{
    Id = "rear",
    Position = new Vector3(0, 0, -2),
    Normal = new Vector3(0, 0, -1),
    CompatibleTypes = new List<ModuleType> { ModuleType.CoreHull },
    SizeClass = 2
});
```

### Using ModuleFactory

```csharp
// Factory creates pre-configured modules
var cockpit = ModuleFactory.CreateCockpit(ModuleStyle.Military, "Titanium");
var hull = ModuleFactory.CreateCoreHull(ModuleStyle.Military, sizeClass: 2, "Titanium");
var engine = ModuleFactory.CreateEngineNacelle(ModuleStyle.Military, sizeClass: 1, "Iron");
var wing = ModuleFactory.CreateWing(ModuleStyle.Military, isRightSide: true, "Iron");
```

## Attachment System

Modules connect via attachment points:

1. **Position**: Where on the module the point is located
2. **Normal**: Direction the point faces (outward)
3. **Compatible Types**: What module types can connect here
4. **Size Class**: Must match between connecting modules

```csharp
// Attachment point on hull for cockpit
new AttachmentPoint
{
    Id = "front",
    Position = new Vector3(0, 0, 10),  // Front of hull
    Normal = new Vector3(0, 0, 1),     // Faces forward
    CompatibleTypes = new List<ModuleType> { ModuleType.Cockpit },
    SizeClass = 2
}
```

## Demo Access

Run the game and select option **32** from the main menu to access the interactive modular ship generator demo:

- Generate random ships
- Select specific archetypes and styles
- View archetype details
- Compare styles side-by-side
- Generate entire fleets

## Best Practices

1. **Match Size Classes**: Ensure connecting modules have compatible size classes
2. **Balance Thrust and Mass**: Add engines proportional to ship mass
3. **Distribute Attachment Points**: Place points on all faces for flexibility
4. **Use Consistent Styles**: Mix styles sparingly for visual coherence
5. **Test Connectivity**: Verify all modules connect properly to avoid floating parts
