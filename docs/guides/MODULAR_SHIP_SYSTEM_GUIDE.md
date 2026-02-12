# Modular Ship Design System Guide

## Overview

The Codename:Subspace game engine has been upgraded with a **modular ship design system** that replaces the previous voxel-based ship construction. This new system is inspired by modular spaceship assets like the **Star Sparrow Modular Spaceship** and allows for more realistic, pre-designed ship components.

### Key Changes

**Voxels are NOW ONLY used for:**
- ✅ Visualizing damage on ships (showing broken/destroyed sections)
- ✅ Asteroid mining (material removal simulation)
- ✅ Asteroid deformation

**Ships are NOW built from:**
- ✅ Pre-defined modular parts (hull sections, engines, wings, weapons, etc.)
- ✅ 3D models (OBJ, FBX, GLTF) instead of cube blocks
- ✅ Attachment point system for connecting modules
- ✅ Material-based stat scaling

---

## Architecture

### Core Components

#### 1. **ShipModulePart** (`ShipModulePart.cs`)
Represents an individual module instance on a ship.

```csharp
public class ShipModulePart
{
    public Guid Id { get; set; }
    public string ModuleDefinitionId { get; set; }  // Reference to definition
    public Vector3 Position { get; set; }
    public Vector3 Rotation { get; set; }
    public string MaterialType { get; set; }        // Iron, Titanium, etc.
    public float Health { get; set; }
    public List<Guid> AttachedToModules { get; set; }
    public ModuleFunctionalStats FunctionalStats { get; set; }
}
```

**Properties:**
- Position, rotation, and scale in ship space
- Material type (affects stats through multipliers)
- Health and damage tracking
- Attachment relationships
- Functional statistics (thrust, power, shields, etc.)

#### 2. **ShipModuleDefinition** (`ShipModuleDefinition.cs`)
Defines a type of module (like a blueprint or prefab).

```csharp
public class ShipModuleDefinition
{
    public string Id { get; set; }
    public string Name { get; set; }
    public ModuleCategory Category { get; set; }
    public string ModelPath { get; set; }           // Path to 3D model
    public Dictionary<string, AttachmentPoint> AttachmentPoints { get; set; }
    public ModuleFunctionalStats BaseStats { get; set; }
}
```

**Key Features:**
- Reference to 3D model file
- Attachment points for connecting to other modules
- Base statistics (modified by material type)
- Tech level requirements
- Tags for filtering

#### 3. **ModularShipComponent** (`ModularShipComponent.cs`)
Component that holds all modules for a ship (replaces VoxelStructureComponent for ships).

```csharp
public class ModularShipComponent : IComponent
{
    public List<ShipModulePart> Modules { get; set; }
    public Vector3 CenterOfMass { get; private set; }
    public float TotalMass { get; private set; }
    public ModuleFunctionalStats AggregatedStats { get; private set; }
    public Guid? CoreModuleId { get; set; }  // Critical module (cockpit)
}
```

**Responsibilities:**
- Manages all modules on the ship
- Calculates center of mass from module positions
- Aggregates functional stats from all modules
- Handles module attachment validation
- Tracks core module (ship dies if core is destroyed)

#### 4. **ModuleLibrary** (`ModuleLibrary.cs`)
Registry of all available module definitions.

```csharp
public class ModuleLibrary
{
    public void InitializeBuiltInModules();
    public void AddDefinition(ShipModuleDefinition definition);
    public ShipModuleDefinition? GetDefinition(string id);
    public List<ShipModuleDefinition> GetDefinitionsByCategory(ModuleCategory category);
}
```

**Built-in Module Types:**
- **Hull**: Cockpit, hull sections, corners
- **Engines**: Main engines, engine nacelles, thrusters
- **Wings**: Wing sections, stabilizers
- **Weapons**: Weapon mounts, turrets
- **Utility**: Power cores, shields, cargo, crew quarters
- **Special**: Hyperdrive, sensors, mining lasers
- **Decorative**: Antennas, details

#### 5. **VoxelDamageSystem** (`VoxelDamageSystem.cs`)
System that uses voxels to visualize damage on modular ships.

```csharp
public class VoxelDamageSystem : SystemBase
{
    public void ApplyDamageToModule(Guid shipId, Guid moduleId, float damage);
    public void RepairModule(Guid shipId, Guid moduleId, float repairAmount);
}
```

**How it works:**
- Generates voxel "holes" over damaged modules
- Number of voxels destroyed = damage percentage
- Voxels are overlays on the 3D models
- Automatically updates as modules take/repair damage

---

## Module Categories

### ModuleCategory Enum

```csharp
public enum ModuleCategory
{
    Hull,           // Main hull sections, cockpits
    Wing,           // Wing sections
    Tail,           // Tail sections
    Engine,         // Main engines
    Thruster,       // Maneuvering thrusters
    WeaponMount,    // Turret mounts, weapon hardpoints
    Weapon,         // Actual weapon modules
    PowerCore,      // Power generators
    Shield,         // Shield generators
    Cargo,          // Cargo bays
    CrewQuarters,   // Crew compartments
    Hyperdrive,     // FTL drives
    Sensor,         // Radar and sensors
    Mining,         // Mining equipment
    Decorative,     // Non-functional decorative elements
    Antenna,        // Antennas and communication arrays
}
```

---

## Attachment System

### AttachmentPoint

Modules connect to each other via **attachment points**.

```csharp
public class AttachmentPoint
{
    public string Name { get; set; }              // e.g., "front", "rear", "left"
    public Vector3 Position { get; set; }         // Relative to module center
    public Vector3 Direction { get; set; }        // Direction this point faces
    public List<ModuleCategory> AllowedCategories { get; set; }
    public AttachmentSize Size { get; set; }      // Small, Medium, Large, ExtraLarge
}
```

**Example:**
```csharp
// Hull section with front and rear attachment points
module.AttachmentPoints["front"] = new AttachmentPoint
{
    Name = "front",
    Position = new Vector3(0, 0, 2),      // 2 units forward
    Direction = new Vector3(0, 0, 1),     // Faces forward
    Size = AttachmentSize.Medium
};

module.AttachmentPoints["rear"] = new AttachmentPoint
{
    Name = "rear",
    Position = new Vector3(0, 0, -2),     // 2 units backward
    Direction = new Vector3(0, 0, -1),    // Faces backward
    Size = AttachmentSize.Medium
};
```

---

## Material System

Materials affect module statistics through multipliers.

### MaterialProperties

```csharp
public static class MaterialProperties
{
    // Available materials
    Iron      - 1.0x durability, 1.0x mass, 0.8x energy
    Titanium  - 1.5x durability, 0.9x mass, 1.0x energy
    Naonite   - 2.0x durability, 0.8x mass, 1.2x energy
    Trinium   - 2.5x durability, 0.6x mass, 1.5x energy
    Xanion    - 3.0x durability, 0.5x mass, 1.8x energy
    Ogonite   - 4.0x durability, 0.4x mass, 2.2x energy
    Avorion   - 5.0x durability, 0.3x mass, 3.0x energy
}
```

**Example:**
```csharp
// A power core made of Avorion generates 3x more power than Iron
var definition = library.GetDefinition("power_core_basic");
// Base: 500 power generation
// Avorion: 500 * 3.0 = 1500 power generation

var stats = definition.GetStatsForMaterial("Avorion");
// stats.PowerGeneration = 1500
```

---

## Usage Examples

### 1. Building a Ship from Modules

```csharp
// Initialize module library
var library = new ModuleLibrary();
library.InitializeBuiltInModules();

// Create a new ship
var ship = new ModularShipComponent
{
    EntityId = Guid.NewGuid(),
    Name = "Star Fighter"
};

// Add cockpit (core module)
var cockpit = new ShipModulePart("cockpit_basic", Vector3.Zero, "Titanium");
var cockpitDef = library.GetDefinition("cockpit_basic");
cockpit.MaxHealth = cockpitDef.GetHealthForMaterial("Titanium");
cockpit.Health = cockpit.MaxHealth;
cockpit.Mass = cockpitDef.GetMassForMaterial("Titanium");
cockpit.FunctionalStats = cockpitDef.GetStatsForMaterial("Titanium");
ship.AddModule(cockpit);
ship.CoreModuleId = cockpit.Id; // Mark as core

// Add hull section behind cockpit
var hull = new ShipModulePart("hull_section_basic", new Vector3(0, 0, -4), "Titanium");
var hullDef = library.GetDefinition("hull_section_basic");
hull.MaxHealth = hullDef.GetHealthForMaterial("Titanium");
hull.Health = hull.MaxHealth;
hull.Mass = hullDef.GetMassForMaterial("Titanium");
hull.FunctionalStats = hullDef.GetStatsForMaterial("Titanium");
ship.AddModule(hull);

// Attach hull to cockpit
ship.AttachModules(cockpit.Id, hull.Id, "rear", "front", library);

// Add engines
var engine = new ShipModulePart("engine_main", new Vector3(0, 0, -8), "Titanium");
var engineDef = library.GetDefinition("engine_main");
engine.MaxHealth = engineDef.GetHealthForMaterial("Titanium");
engine.Health = engine.MaxHealth;
engine.Mass = engineDef.GetMassForMaterial("Titanium");
engine.FunctionalStats = engineDef.GetStatsForMaterial("Titanium");
ship.AddModule(engine);
ship.AttachModules(hull.Id, engine.Id, "rear", "mount", library);

// Add wings
var leftWing = new ShipModulePart("wing_basic", new Vector3(-3, 0, -4), "Titanium");
var wingDef = library.GetDefinition("wing_basic");
leftWing.MaxHealth = wingDef.GetHealthForMaterial("Titanium");
leftWing.Health = leftWing.MaxHealth;
leftWing.Mass = wingDef.GetMassForMaterial("Titanium");
leftWing.FunctionalStats = wingDef.GetStatsForMaterial("Titanium");
ship.AddModule(leftWing);
ship.AttachModules(hull.Id, leftWing.Id, "left", "mount", library);

// Ship stats are automatically calculated
Console.WriteLine($"Ship Mass: {ship.TotalMass}");
Console.WriteLine($"Thrust Power: {ship.AggregatedStats.ThrustPower}");
Console.WriteLine($"Sensor Range: {ship.AggregatedStats.SensorRange}");
```

### 2. Applying Damage with Voxel Visualization

```csharp
// Create damage system
var entityManager = new EntityManager();
var damageSystem = new VoxelDamageSystem(entityManager);

// Register ship
entityManager.AddEntity(ship.EntityId);
entityManager.AddComponent(ship.EntityId, ship);

// Apply damage to a wing
damageSystem.ApplyDamageToModule(ship.EntityId, leftWing.Id, 50f);

// Damage voxels are automatically created to show "holes" in the wing
var damageComponent = entityManager.GetComponent<VoxelDamageComponent>(ship.EntityId);
Console.WriteLine($"Damage voxels: {damageComponent.DamageVoxels.Count}");

// Repair the wing
damageSystem.RepairModule(ship.EntityId, leftWing.Id, 25f);

// Damage voxels are automatically updated/removed
```

### 3. Creating Custom Module Definitions

```csharp
// Define a new custom module
var customModule = new ShipModuleDefinition
{
    Id = "heavy_armor_section",
    Name = "Heavy Armor Section",
    Description = "Reinforced armor hull section",
    Category = ModuleCategory.Hull,
    SubCategory = "Armored",
    Size = new Vector3(4, 4, 5),
    BaseMass = 50f,        // Heavier than normal hull
    BaseHealth = 500f,     // Much more durable
    BaseCost = 1000,
    TechLevel = 3,
    Tags = new List<string> { "hull", "armor", "heavy" },
    ModelPath = "Assets/Models/HeavyArmor.obj"
};

// Add attachment points
customModule.AttachmentPoints["front"] = new AttachmentPoint
{
    Name = "front",
    Position = new Vector3(0, 0, 2.5f),
    Direction = new Vector3(0, 0, 1),
    Size = AttachmentSize.Large
};

customModule.AttachmentPoints["rear"] = new AttachmentPoint
{
    Name = "rear",
    Position = new Vector3(0, 0, -2.5f),
    Direction = new Vector3(0, 0, -1),
    Size = AttachmentSize.Large
};

// Add to library
library.AddDefinition(customModule);

// Now it can be used in ship construction
var armorModule = new ShipModulePart("heavy_armor_section", position, "Ogonite");
```

### 4. Querying Ship Modules

```csharp
// Get all engine modules
var engines = ship.GetModulesByCategory(ModuleCategory.Engine, library);
Console.WriteLine($"Ship has {engines.Count} engines");

// Get total thrust
float totalThrust = engines.Sum(e => e.FunctionalStats.ThrustPower);

// Get all weapon mounts
var weapons = ship.GetModulesByCategory(ModuleCategory.WeaponMount, library);
Console.WriteLine($"Ship has {weapons.Count} weapon mounts");

// Check if ship has hyperdrive
if (ship.AggregatedStats.HasHyperdrive)
{
    Console.WriteLine($"Hyperdrive range: {ship.AggregatedStats.HyperdriveRange}");
}
```

---

## Integration with Existing Systems

### Physics System Integration

The modular ship component works with the existing physics system:

```csharp
// ModularShipComponent provides:
- CenterOfMass (calculated from module positions)
- TotalMass (sum of all module masses)
- ThrustPower (from engine modules)

// Physics calculations use these values
var physics = entityManager.GetComponent<PhysicsComponent>(shipId);
physics.Mass = ship.TotalMass;
physics.CenterOfMass = ship.CenterOfMass;
```

### Combat System Integration

```csharp
// Damage is applied to specific modules
damageSystem.ApplyDamageToModule(shipId, targetModuleId, damage);

// If core module is destroyed, ship is destroyed
if (ship.CoreModuleId.HasValue && ship.GetModule(ship.CoreModuleId.Value)?.IsDestroyed == true)
{
    // Ship is destroyed
}

// Modules can detach if their connection is destroyed
```

### Power System Integration

```csharp
// Power generation and consumption from modules
var powerGen = ship.AggregatedStats.PowerGeneration;
var powerCon = ship.AggregatedStats.PowerConsumption;
var powerStorage = ship.AggregatedStats.PowerStorage;

// Check if ship has sufficient power
if (powerGen >= powerCon)
{
    // All systems operational
}
```

---

## Voxel Usage (Limited to Damage & Asteroids)

### Ship Damage Visualization

Voxels are ONLY used to show damage on modular ships:

```csharp
public class VoxelDamageComponent : IComponent
{
    // Voxel blocks that show damage (overlays on 3D models)
    public List<VoxelBlock> DamageVoxels { get; set; }
    
    // Maps module ID to its damage voxels
    public Dictionary<Guid, List<VoxelBlock>> ModuleDamageMap { get; set; }
}
```

**How it works:**
1. When a module takes damage, voxels are generated over it
2. Number of "destroyed" voxels = damage percentage
3. Voxels appear as "holes" or "broken sections" on the module
4. Voxels are removed when module is repaired
5. Rendering system overlays damage voxels on 3D models

### Asteroid Voxels (Unchanged)

Asteroids still use voxels for mining:

```csharp
// Asteroids use VoxelStructureComponent (not ModularShipComponent)
var asteroid = new VoxelStructureComponent();
asteroid.AddBlock(voxelBlock);

// Mining removes voxel blocks
var miningSystem = new MiningSystem();
miningSystem.MineVoxel(asteroidId, voxelPosition);
```

---

## Migration from Voxel Ships

### For Existing Ships

Ships using `VoxelStructureComponent` need to be converted:

```csharp
public class ShipMigrationHelper
{
    public static ModularShipComponent ConvertVoxelShipToModular(
        VoxelStructureComponent voxelShip, 
        ModuleLibrary library)
    {
        var modularShip = new ModularShipComponent
        {
            Name = "Converted Ship"
        };
        
        // Analyze voxel ship structure
        var bounds = CalculateBounds(voxelShip);
        var centerOfMass = voxelShip.CenterOfMass;
        
        // Create equivalent modules
        // This is a simplified conversion - actual implementation would be more complex
        
        // Add cockpit at front
        var cockpit = CreateModuleFromVoxels("cockpit_basic", centerOfMass, library);
        modularShip.AddModule(cockpit);
        modularShip.CoreModuleId = cockpit.Id;
        
        // Add hull sections
        // Add engines based on engine blocks
        // Add weapons based on turret mounts
        // etc.
        
        return modularShip;
    }
}
```

### For Procedural Generation

Update procedural ship generators to create modular ships:

```csharp
public class ModularProceduralShipGenerator
{
    public ModularShipComponent GenerateShip(
        ShipSize size, 
        ShipRole role, 
        ModuleLibrary library)
    {
        var ship = new ModularShipComponent();
        
        // Start with cockpit
        var cockpit = CreateModule("cockpit_basic", Vector3.Zero, library);
        ship.AddModule(cockpit);
        ship.CoreModuleId = cockpit.Id;
        
        // Add hull sections based on size
        // Add engines based on role
        // Add weapons, cargo, etc.
        
        return ship;
    }
}
```

---

## Asset Integration

### 3D Model Loading (Future)

When 3D model loading is implemented:

```csharp
public class AssetManager
{
    public Mesh LoadModel(string path);
    public uint LoadTexture(string path);
}

// Module definitions reference 3D models
var module = new ShipModuleDefinition
{
    ModelPath = "Assets/Models/StarSparrow/Cockpit_01.fbx",
    TexturePath = "Assets/Textures/StarSparrow/Cockpit_Albedo.png"
};

// Renderer loads and renders the model
var mesh = assetManager.LoadModel(module.ModelPath);
meshRenderer.Render(mesh, moduleTransform);
```

### Recommended Asset Structure

```
AvorionLike/Assets/
├── Models/
│   ├── ShipModules/
│   │   ├── Cockpits/
│   │   │   ├── cockpit_01.fbx
│   │   │   ├── cockpit_02.fbx
│   │   ├── HullSections/
│   │   │   ├── hull_section_01.fbx
│   │   │   ├── hull_corner_01.fbx
│   │   ├── Engines/
│   │   │   ├── engine_main_01.fbx
│   │   │   ├── engine_nacelle_01.fbx
│   │   ├── Wings/
│   │   ├── Weapons/
│   │   └── Utility/
├── Textures/
│   ├── ShipModules/
│   │   ├── hull_albedo.png
│   │   ├── hull_normal.png
│   │   ├── hull_metallic.png
└── ModuleDefinitions/
    └── modules.json  // Module library definitions
```

---

## Performance Considerations

### Memory Usage

- **Modular ships**: Lower memory than voxel ships (fewer objects)
- **Damage voxels**: Only created when modules are damaged
- **LOD**: Can implement LOD for distant ships (show fewer modules)

### Rendering

- **3D models**: More efficient than rendering thousands of voxel cubes
- **Damage overlays**: Voxel damage rendered on top of models
- **Instancing**: Same module definition can be instanced multiple times

### Physics

- **Simpler collision**: Modules have bounding boxes instead of per-voxel collision
- **Center of mass**: Calculated from module positions (lighter than per-voxel)

---

## Future Enhancements

### Planned Features

1. **3D Model Loading**: Integrate Assimp.NET for FBX/OBJ/GLTF
2. **Texture Mapping**: PBR materials for modules
3. **Module Animations**: Rotating turrets, engine glow effects
4. **Module Sockets**: Upgrade slots on modules
5. **Visual Customization**: Paint jobs, decals, module variants
6. **Module Rarity**: Common/Rare/Legendary modules with stat bonuses
7. **Modular Stations**: Use same system for space stations
8. **Ship Templates**: Save/load modular ship designs

### Modding Support

```csharp
// Mods can add custom modules via JSON
{
  "id": "mod_custom_engine",
  "name": "Quantum Drive Engine",
  "category": "Engine",
  "modelPath": "Mods/MyMod/Models/quantum_engine.fbx",
  "baseMass": 35,
  "baseHealth": 150,
  "baseStats": {
    "thrustPower": 2000,
    "powerConsumption": 100
  }
}
```

---

## Testing

### Unit Tests

```csharp
[Test]
public void TestModuleAttachment()
{
    var library = new ModuleLibrary();
    library.InitializeBuiltInModules();
    
    var ship = new ModularShipComponent();
    var module1 = new ShipModulePart("hull_section_basic", Vector3.Zero);
    var module2 = new ShipModulePart("engine_main", new Vector3(0, 0, -4));
    
    ship.AddModule(module1);
    ship.AddModule(module2);
    
    bool attached = ship.AttachModules(
        module1.Id, module2.Id, 
        "rear", "mount", 
        library
    );
    
    Assert.IsTrue(attached);
    Assert.Contains(module2.Id, module1.AttachedModules);
}

[Test]
public void TestDamageVisualization()
{
    var entityManager = new EntityManager();
    var damageSystem = new VoxelDamageSystem(entityManager);
    
    var ship = CreateTestShip();
    var moduleId = ship.Modules[0].Id;
    
    entityManager.AddEntity(ship.EntityId);
    entityManager.AddComponent(ship.EntityId, ship);
    
    damageSystem.ApplyDamageToModule(ship.EntityId, moduleId, 50f);
    
    var damageComp = entityManager.GetComponent<VoxelDamageComponent>(ship.EntityId);
    Assert.IsNotNull(damageComp);
    Assert.Greater(damageComp.DamageVoxels.Count, 0);
}
```

---

## Summary

### Key Benefits of Modular System

✅ **More realistic** - Ships use actual 3D models instead of cubes
✅ **Better performance** - Fewer objects to render
✅ **Easier design** - Pre-defined parts are easier to work with
✅ **Asset integration** - Can use Unity Asset Store models (converted)
✅ **Voxels preserved** - Still used for damage and asteroid mining
✅ **Material system** - Stats scale with material tier
✅ **Attachment validation** - Ensures ships are structurally sound
✅ **Modular damage** - Individual modules can be destroyed

### Migration Path

1. ✅ Core modular system implemented
2. ⏳ Update ship builder UI for module placement
3. ⏳ Integrate 3D model loading (Assimp.NET)
4. ⏳ Convert procedural generation to use modules
5. ⏳ Update examples and tests
6. ⏳ Add asset integration (FBX/OBJ models)

---

## Questions?

For more information, see:
- `ShipModulePart.cs` - Module instance implementation
- `ShipModuleDefinition.cs` - Module type definitions
- `ModularShipComponent.cs` - Ship component implementation
- `ModuleLibrary.cs` - Module registry and built-in modules
- `VoxelDamageSystem.cs` - Damage visualization with voxels
- `ASSET_INTEGRATION_GUIDE.md` - Guide for loading 3D models

This system provides a solid foundation for modular ship design while preserving voxels for their intended purposes (damage visualization and asteroid mining).
