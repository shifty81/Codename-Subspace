# Modular Ship System Integration

## Overview

The modular ship system is now fully integrated with the game engine, providing automatic synchronization between ship modules and physics, as well as visual damage representation.

## Systems

### 1. ModularShipSyncSystem

**Purpose:** Synchronizes `ModularShipComponent` stats with `PhysicsComponent`

**Features:**
- Automatically creates `PhysicsComponent` for ships that don't have one
- Updates physics mass when ship modules change
- Calculates collision radius from ship bounds
- Sets ships to static physics when destroyed (core module destroyed)
- Updates moment of inertia based on mass and size

**When it runs:** Every frame as part of the engine update loop

**Example:**
```csharp
var engine = new GameEngine();
var ship = GenerateModularShip(); // Your ship generation code
engine.EntityManager.AddComponent(entityId, ship);

// ModularShipSyncSystem automatically creates and updates PhysicsComponent
engine.Update(); // Physics now reflects ship configuration
```

### 2. VoxelDamageSystem

**Purpose:** Generates voxel-based damage visualization for modular ships

**Features:**
- Creates `VoxelDamageComponent` for ships with damaged modules
- Generates damage voxels showing "holes" and "broken sections"
- Updates damage visualization as modules take more damage
- Clears damage voxels when modules are repaired
- Supports different damage levels (0-100%)

**When it runs:** Every frame as part of the engine update loop

**Visual Effect:**
- Damage voxels appear as darker/missing sections over ship modules
- More damage = more missing voxels
- Completely destroyed modules show maximum damage voxels

**Example:**
```csharp
// Damage a ship module
var module = ship.Modules[0];
module.TakeDamage(50); // 50 damage points
ship.RecalculateStats();

// VoxelDamageSystem automatically creates damage voxels
engine.Update(); // Damage now visible on ship
```

## Integration with GameEngine

Both systems are automatically registered and active when the GameEngine is created:

```csharp
var engine = new GameEngine(seed);
// ModularShipSyncSystem and VoxelDamageSystem are already running
```

## Ship Lifecycle

### 1. Ship Creation
```csharp
// Generate a modular ship
var generator = new ModularProceduralShipGenerator(library, seed);
var config = new ModularShipConfig { ... };
var result = generator.GenerateShip(config);
var ship = result.Ship;

// Add to game world
var entity = engine.EntityManager.CreateEntity("My Ship");
engine.EntityManager.AddComponent(entity.Id, ship);

// On next update:
// - ModularShipSyncSystem creates PhysicsComponent
// - VoxelDamageSystem creates VoxelDamageComponent (if damage exists)
```

### 2. Ship Operation
```csharp
// Physics is automatically synchronized
engine.Update(); 

// Ship mass and collision radius always match ship configuration
var physics = engine.EntityManager.GetComponent<PhysicsComponent>(entityId);
Console.WriteLine($"Mass: {physics.Mass} == {ship.TotalMass}"); // Always equal
```

### 3. Ship Damage
```csharp
// Damage modules
ship.Modules[0].TakeDamage(100);
ship.Modules[1].TakeDamage(50);
ship.RecalculateStats();

// Damage visualization appears automatically
engine.Update();

// Check damage visuals
var damageComp = engine.EntityManager.GetComponent<VoxelDamageComponent>(entityId);
Console.WriteLine($"Damage voxels: {damageComp.DamageVoxels.Count}");
```

### 4. Ship Destruction
```csharp
// Destroy core module
var core = ship.GetModule(ship.CoreModuleId.Value);
core.TakeDamage(core.Health); // Reduce to 0
ship.RecalculateStats();

// Ship destruction is handled automatically
engine.Update();

// Physics is set to static
var physics = engine.EntityManager.GetComponent<PhysicsComponent>(entityId);
Console.WriteLine($"Ship destroyed: {ship.IsDestroyed}");
Console.WriteLine($"Physics static: {physics.IsStatic}"); // true
```

## Key Properties

### ModularShipComponent

- `TotalMass` - Sum of all module masses (synced to physics)
- `Bounds` - Bounding box of all modules (used for collision radius)
- `IsDestroyed` - True if core module destroyed or no modules left
- `AggregatedStats` - Combined functional stats from all modules

### PhysicsComponent (Auto-Synced)

- `Mass` - Matches `ship.TotalMass`
- `CollisionRadius` - Calculated from `ship.Bounds`
- `IsStatic` - Set to true when ship destroyed

### VoxelDamageComponent (Auto-Created)

- `DamageVoxels` - List of voxels showing damage
- `ModuleDamageMap` - Maps module IDs to their damage voxels
- `ShowDamage` - Toggle damage visualization on/off

## Performance Considerations

### ModularShipSyncSystem
- Only updates physics when mass changes by > 0.1 units
- Only updates collision radius when it changes by > 0.1 units
- Minimal overhead for ships with stable configurations

### VoxelDamageSystem
- Only creates voxels for modules with > 10% damage
- Reuses voxel data when damage level doesn't change
- Clears voxels for fully repaired modules

## Testing

Run the integration test to verify all systems work correctly:

```csharp
var test = new ModularShipSystemIntegrationTest();
test.RunTest();
```

The test verifies:
1. ✓ Systems are registered in GameEngine
2. ✓ Physics components are created automatically
3. ✓ Mass and collision radius sync correctly
4. ✓ Damage voxels are generated
5. ✓ Ship destruction is handled properly

## Common Patterns

### Spawning an AI Ship
```csharp
var factory = new ModularShipFactory(library);
var ship = factory.CreateShipForAI(AIPersonality.Aggressive, "Pirate", "Titanium");

var entity = engine.EntityManager.CreateEntity("Pirate Ship");
engine.EntityManager.AddComponent(entity.Id, ship.Ship);
// Physics and damage systems automatically handle the rest
```

### Manual Physics Update
```csharp
// Usually not needed - system does this automatically
// But if you need immediate sync without waiting for Update():
engine.ModularShipSyncSystem.Update(0);
```

### Disabling Damage Visualization
```csharp
var damageComp = engine.EntityManager.GetComponent<VoxelDamageComponent>(entityId);
if (damageComp != null)
{
    damageComp.ShowDamage = false; // Hides damage voxels
}
```

## Architecture Notes

- **Separation of Concerns:** Ship configuration (modules) is separate from physics simulation
- **Automatic Sync:** Changes to ship configuration automatically propagate to physics
- **Visual Feedback:** Damage is purely visual and doesn't affect module functionality directly
- **Destruction Handling:** Destroyed ships automatically stop moving (static physics)

## Future Enhancements

Potential improvements for the modular ship system:

1. **Module Separation:** Detached modules could become separate entities
2. **Repair System:** Gradual repair that updates damage voxels
3. **Advanced Damage:** Different damage types (burn, pierce, explosive)
4. **Module Effects:** Damaged modules reduce their functional effectiveness
5. **Explosion Effects:** Particle effects at damage voxel positions

## Related Documentation

- [MODULAR_SHIP_SYSTEM_GUIDE.md](MODULAR_SHIP_SYSTEM_GUIDE.md) - Complete modular ship documentation
- [MODULAR_SHIP_3D_MODELS_IMPLEMENTATION.md](MODULAR_SHIP_3D_MODELS_IMPLEMENTATION.md) - 3D model integration
- [SHIP_WORK_CONTINUATION_COMPLETE.md](SHIP_WORK_CONTINUATION_COMPLETE.md) - Previous ship work summary

## Changelog

**2026-01-04:** Initial integration of ModularShipSyncSystem and VoxelDamageSystem
- Created ModularShipSyncSystem for physics synchronization
- Registered VoxelDamageSystem in GameEngine
- Added comprehensive integration test
- Documentation created
