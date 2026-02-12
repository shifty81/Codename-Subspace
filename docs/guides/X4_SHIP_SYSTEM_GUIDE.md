# X4-Inspired Ship System - Complete Guide

This guide explains the new X4-inspired ship generation, customization, and gameplay systems.

## Overview

The game now features a comprehensive ship system inspired by X4: Foundations, including:
- **Ship Classes**: From fighters to battleships (XS to XL equivalents)
- **Equipment System**: Weapons, mining lasers, salvage beams, turrets
- **Paint Customization**: Color schemes and visual customization
- **Player Character**: Walk around ship interiors in first-person
- **Interior Building**: Place objects inside your ship like No Man's Sky
- **Starter Ship**: The Ulysses corvette as the default player ship

## Quick Start

### Creating a Starter Ship

```csharp
// Simplest way - create Ulysses for new player
var ship = StarterShipFactory.CreateUlyssesStarterShip("PlayerName");

// Access components
var hull = ship.Ship;              // ModularShipComponent
var equipment = ship.Equipment;    // ShipEquipmentComponent
var paint = ship.Paint;           // ShipPaintScheme
var stats = ship.Stats;           // X4ShipStats
```

### Using the Ulysses Model

The system automatically looks for the Ulysses 3D model in multiple locations:
1. `Assets/Models/ships/hulls/ulysses.blend` (preferred)
2. `Assets/Models/ships/hulls/ulysses.obj`
3. `Assets/Models/ships/hulls/ulysses.fbx`
4. `Assets/Models/ships/hulls/ulysses.gltf`

If no model is found, it generates a procedural ship using modular parts.

```csharp
// Check for model
var modelInfo = UlyssesModelLoader.GetModelInfo();
if (modelInfo.IsAvailable)
{
    Console.WriteLine($"Ulysses model: {modelInfo.Format}");
}

// Load model directly
var meshes = UlyssesModelLoader.LoadUlyssesModel();
if (meshes != null)
{
    // Use meshes for rendering
}
```

## Ship Classes

### X4 Ship Classification

Based on X4's ship class system:

#### Small (S) Class
- **Fighter_Light**: Fast interceptor (~Scout)
- **Fighter_Heavy**: Combat fighter (~Heavy Fighter)  
- **Miner_Small**: Small mining ship

#### Medium (M) Class
- **Corvette**: Light warship
- **Frigate**: Medium warship
- **Gunboat**: Heavy gunboat
- **Miner_Medium**: Medium miner (optimal for most mining)
- **Freighter_Medium**: Medium cargo

#### Large (L) Class
- **Destroyer**: Heavy combat ship
- **Freighter_Large**: Large cargo hauler
- **Miner_Large**: Heavy mining vessel

#### Extra Large (XL) Class
- **Battleship**: Capital warship
- **Carrier**: Fleet carrier
- **Builder**: Construction ship

### Design Styles

Visual and stat variations:

- **Balanced**: All-around (like Argon) - 1.0x all stats
- **Aggressive**: Speed-focused (like Split) - 1.3x speed, 0.8x hull
- **Durable**: Tank-focused (like Teladi) - 1.4x hull, 0.8x speed
- **Sleek**: Fast & elegant (like Paranid) - 1.2x speed, 0.9x hull
- **Advanced**: High-tech (like Terran) - 1.1x all stats
- **Alien**: Unique (like Xenon) - balanced with special aesthetics

### Ship Variants

- **Standard**: Balanced build
- **Sentinel**: +20% hull/cargo, -15% speed (tanky)
- **Vanguard**: +15% speed, -10% hull/cargo (fast)
- **Military**: +10% hull, -5% speed (combat)

## Equipment System

### Equipment Types

#### Weapons
- **PrimaryWeapon**: Forward-facing guns (pulse lasers, bolt repeaters)
- **Turret**: 360° turrets for M/L/XL ships
- **Missile**: Missile launchers

#### Utility
- **MiningLaser**: Break up asteroids
- **SalvageBeam**: Salvage wrecks
- **TractorBeam**: Manipulate cargo
- **Scanner**: Enhanced sensors
- **Shield**: Shield generators
- **CounterMeasure**: Flares, chaff

### Using Equipment

```csharp
// Get ship equipment
var equipment = ship.Equipment;

// Create equipment items
var laser = EquipmentFactory.CreatePulseLaser(tier: 2);
var miningLaser = EquipmentFactory.CreateMiningLaser(tier: 1);
var turret = EquipmentFactory.CreateBeamTurret(tier: 1);

// Find slots
var weaponSlot = equipment.EquipmentSlots
    .FirstOrDefault(s => s.AllowedType == EquipmentType.PrimaryWeapon && !s.IsOccupied);

// Equip item
if (weaponSlot != null)
{
    equipment.EquipItem(weaponSlot.Id, laser);
}

// Unequip item
var removed = equipment.UnequipItem(weaponSlot.Id);

// Get stats
var totalDamage = equipment.GetTotalWeaponDamage();
var totalMining = equipment.GetTotalMiningPower();
var powerUse = equipment.TotalPowerConsumption;
```

### Equipment Slots by Ship Class

| Ship Class | Primary Weapons | Turrets | Utility Slots |
|------------|----------------|---------|---------------|
| Fighter_Light | 2 | 0 | 1 |
| Fighter_Heavy | 4 | 0 | 2 |
| Corvette | 2 | 0 | 2 |
| Frigate | 2 | 2 | 3 |
| Gunboat | 4 | 2 | 2 |
| Destroyer | 2 | 4 | 3 |
| Battleship | 4 | 8 | 4 |
| Carrier | 0 | 6 | 5 |
| Miner_Medium | 1 | 0 | 3 |
| Miner_Large | 0 | 2 | 4 |

## Paint Customization

### Paint Schemes

Paint schemes have multiple color channels:
- **Primary**: Main hull color
- **Secondary**: Secondary panels
- **Accent**: Highlights and details
- **Glow**: Engine glow and lights

```csharp
// Get available paints
var allPaints = PaintLibrary.GetAllPaints();
var exceptionalPaints = PaintLibrary.GetPaintsByQuality(PaintQuality.Exceptional);

// Apply paint to ship
var paintComponent = new ShipPaintComponent 
{ 
    EntityId = ship.Ship.EntityId 
};
paintComponent.ApplyPaint(PaintLibrary.GetPaintsByQuality(PaintQuality.Exceptional)[0]);

// Create custom paint
var customPaint = PaintLibrary.CreateCustomPaint(
    name: "My Colors",
    primaryColor: (255, 0, 0),    // Red
    secondaryColor: (0, 0, 0),    // Black
    accentColor: (255, 255, 0),   // Yellow
    pattern: "Striped"
);

// Add to library
PaintLibrary.AddPaint(customPaint);
```

### Built-in Paint Schemes

- Military Gray (Basic)
- Space Black (Basic)
- Pristine White (Advanced)
- Combat Red (Advanced)
- Navy Blue (Advanced)
- Merchant Gold (Exceptional)
- Stealth Camo (Exceptional)
- Pirate (Advanced)
- Explorer (Advanced)
- Racing Orange (Exceptional)

## Player Character

### Character Movement

```csharp
// Create player character
var character = new PlayerCharacterComponent
{
    EntityId = Guid.NewGuid(),
    Position = new Vector3(0, 0, 0),
    WalkSpeed = 3.0f,
    RunSpeed = 6.0f,
    CurrentShipId = ship.Ship.EntityId
};

// Create camera
var camera = new PlayerCameraComponent
{
    EntityId = character.EntityId,
    FieldOfView = 90f
};

// Update movement
var characterSystem = new PlayerCharacterSystem();
var moveDir = new Vector3(1, 0, 0); // Move right
characterSystem.UpdateMovement(character, moveDir, deltaTime: 0.016f);
characterSystem.UpdateCamera(character, camera, deltaTime: 0.016f);

// Jump
characterSystem.Jump(character);

// Teleport (for entering ships)
characterSystem.TeleportTo(character, new Vector3(0, 1, 0), shipId: ship.Ship.EntityId);
```

### Character States

- Walking: Default movement
- Running: Hold shift (2x walk speed)
- Crouching: Crouch key (0.5x walk speed, lower height)
- Zero-G: When in space (6DOF movement with drag)

### Interactions

```csharp
// Register interactable object
var terminal = new InteractableObject
{
    Position = new Vector3(1, 0, 1),
    Name = "Computer Terminal",
    InteractionPrompt = "Press E to access",
    Type = InteractionType.Terminal,
    OnInteract = (playerId) => 
    {
        Console.WriteLine("Terminal accessed!");
    }
};

characterSystem.RegisterInteractable(terminal);

// Find nearby interactables
var lookDir = camera.Forward;
var nearby = characterSystem.FindInteractableInRange(character, lookDir);

if (nearby != null)
{
    // Show prompt to player
    // On interact key press:
    characterSystem.Interact(character, nearby);
}
```

## Interior Building

### Interior System

Ships have interior cells (rooms) that can be decorated with objects.

```csharp
// Generate interior for ship
var interiorSystem = new InteriorGenerationSystem();
var moduleIds = ship.Ship.Modules.Select(m => m.Id).ToList();
var interior = interiorSystem.GenerateInterior(ship.Ship.EntityId, moduleIds);

Console.WriteLine($"Generated {interior.Cells.Count} interior cells");
```

### Placing Objects

```csharp
// Create interior objects
var terminal = InteriorObjectLibrary.CreateTerminal();
var storage = InteriorObjectLibrary.CreateStorage();
var chair = InteriorObjectLibrary.CreateChair();
var workbench = InteriorObjectLibrary.CreateWorkbench();

// Place objects (snap to grid)
var position = new Vector3(1, 0, 1);
var success = interior.PlaceObject(terminal, position);

if (success)
{
    Console.WriteLine("Object placed successfully!");
}

// Remove object
interior.RemoveObject(terminal.Id);

// Check object count
var count = interior.GetTotalObjectCount();
var max = interior.MaxObjects; // Default: 100
```

### Interior Object Types

#### Functional
- Terminal: Computer access
- Storage: Item storage
- Workbench: Crafting
- MedicalStation: Healing
- TurretControl: Manual turret
- TeleportPad: Teleporter

#### Furniture
- Chair, Table, Bed, Sofa, Desk

#### Decoration
- Plant, Poster, Light, Locker, Crate

#### Technical
- PowerNode, LifeSupportNode, DataNode

## Advanced: Custom Ship Generation

### Generate Custom X4 Ship

```csharp
// Setup
var library = new ModuleLibrary();
library.InitializeBuiltInModules();
var generator = new X4ShipGenerator(library);

// Configure ship
var config = new X4ShipConfig
{
    ShipClass = X4ShipClass.Destroyer,
    DesignStyle = X4DesignStyle.Advanced,
    Variant = X4ShipVariant.Military,
    ShipName = "U.S.S. Enterprise",
    Seed = 12345,
    PrimaryColor = (50, 100, 200),    // Blue
    SecondaryColor = (30, 60, 120),
    AccentColor = (255, 255, 255)     // White
};

// Generate
var ship = generator.GenerateX4Ship(config);

// View stats
Console.WriteLine(ship.Stats.ToString());
/*
Output:
Mass: 5000t | Hull: 8000 | Shield: 3000
Speed: 45.5 m/s | Thrust: 12000N
Cargo: 200 units | Power: 800/650W
Weapons: 450 DPS | Mining: 0
Equipment: 9/9 slots
*/
```

### Create Custom Template

```csharp
var templateManager = new ShipTemplateManager();

var customTemplate = new ShipTemplate
{
    Id = "custom_corvette",
    Name = "Custom Corvette",
    Description = "My custom ship design",
    ModelPath = "Models/ships/hulls/my_ship.obj",
    ShipClass = X4ShipClass.Corvette,
    DesignStyle = X4DesignStyle.Sleek,
    Variant = X4ShipVariant.Vanguard,
    
    // Override base stats
    BaseHull = 2000f,
    BaseSpeed = 100f,
    
    // Equipment slots
    PrimaryWeaponSlots = 4,
    TurretSlots = 1,
    UtilitySlots = 2,
    
    // Default equipment
    DefaultEquipment = new List<EquipmentLoadout>
    {
        new() { SlotName = "Primary 1", Item = EquipmentFactory.CreatePulseLaser(2) },
        new() { SlotName = "Primary 2", Item = EquipmentFactory.CreatePulseLaser(2) }
    },
    
    // Paint
    DefaultPaint = new ShipPaintScheme
    {
        Name = "Custom Paint",
        PrimaryColor = (100, 100, 150),
        SecondaryColor = (50, 50, 75),
        AccentColor = (200, 200, 255)
    }
};

templateManager.AddTemplate(customTemplate);

// Generate from template
var ship = templateManager.GenerateFromTemplate(customTemplate, "My Ship");
```

## File Locations

### Assets Structure

```
Assets/
├── Models/
│   └── ships/
│       ├── hulls/           # Complete ship hull models
│       │   ├── ulysses.blend    # Ulysses starter ship
│       │   ├── ulysses.obj
│       │   └── README.md
│       └── modules/         # Modular ship parts (legacy/fallback)
│           ├── cockpit_basic.obj
│           ├── engine_main.obj
│           └── ...
```

### Code Structure

```
AvorionLike/Core/
├── Modular/
│   ├── X4ShipClasses.cs          # Ship class definitions
│   ├── ShipEquipmentSystem.cs    # Equipment system
│   ├── ShipPaintSystem.cs        # Paint customization
│   ├── X4ShipGenerator.cs        # Ship generation
│   ├── ShipTemplateManager.cs    # Ship templates
│   └── UlyssesModelLoader.cs     # Model loading
├── RPG/
│   └── PlayerCharacterSystem.cs  # Player movement
├── Building/
│   └── ShipInteriorSystem.cs     # Interior building
└── Examples/
    └── UlyssesShipExample.cs     # Usage examples
```

## Tips & Best Practices

### Performance
- Ship models are cached after first load
- Interior objects have collision limits (100 max by default)
- Use LOD (Level of Detail) models for distant ships

### Customization
- Always check equipment slot compatibility before equipping
- Paint schemes don't affect performance
- Interior objects can require power - check power budget

### Modding
- Add custom ship templates to ShipTemplateManager
- Create custom equipment in EquipmentFactory
- Add custom paint schemes to PaintLibrary
- Place custom models in Assets/Models/ships/hulls/

## Troubleshooting

### Ulysses Model Not Loading

1. Check file location: `Assets/Models/ships/hulls/ulysses.blend`
2. Check supported formats: .blend .obj .fbx .gltf .glb
3. Check console logs: Look for "UlyssesLoader" messages
4. Verify file permissions and size
5. If all else fails, it will generate procedurally

### Equipment Not Fitting

- Check slot size vs item size
- Check equipment type compatibility
- Verify ship class has enough slots

### Interior Objects Not Placing

- Check for collisions with existing objects
- Verify position is within cell bounds
- Check object count limit (100 default)
- Ensure BuildModeActive is true if required

## Examples

See `AvorionLike/Examples/UlyssesShipExample.cs` for complete working examples of all features.

---

**Last Updated**: January 2026  
**Version**: X4 System v1.0  
**Status**: Production Ready
