# X4 Ship System - Quick Reference

## 🚀 Create Ships

```csharp
// Ulysses starter ship
var ship = StarterShipFactory.CreateUlyssesStarterShip("PlayerName");

// Custom X4 ship
var library = new ModuleLibrary();
library.InitializeBuiltInModules();
var generator = new X4ShipGenerator(library);
var ship = generator.GenerateX4Ship(new X4ShipConfig
{
    ShipClass = X4ShipClass.Frigate,
    DesignStyle = X4DesignStyle.Balanced,
    ShipName = "My Ship"
});
```

## ⚔️ Equipment

```csharp
// Create equipment
var weapon = EquipmentFactory.CreatePulseLaser(tier: 2);
var miner = EquipmentFactory.CreateMiningLaser(tier: 1);
var turret = EquipmentFactory.CreateBeamTurret(tier: 1);

// Equip item
var slot = ship.Equipment.EquipmentSlots.First(s => !s.IsOccupied);
ship.Equipment.EquipItem(slot.Id, weapon);

// Get stats
var damage = ship.Equipment.GetTotalWeaponDamage();
var mining = ship.Equipment.GetTotalMiningPower();
```

## 🎨 Paint

```csharp
// Use built-in paint
var paint = PaintLibrary.GetAllPaints()[0];

// Custom paint
var custom = PaintLibrary.CreateCustomPaint(
    "My Colors",
    primaryColor: (255, 0, 0),
    secondaryColor: (0, 0, 0),
    accentColor: (255, 255, 0)
);

// Apply
var paintComp = new ShipPaintComponent { EntityId = ship.Ship.EntityId };
paintComp.ApplyPaint(custom);
```

## 🚶 Player Character

```csharp
// Create character
var character = new PlayerCharacterComponent
{
    Position = Vector3.Zero,
    CurrentShipId = ship.Ship.EntityId
};

var camera = new PlayerCameraComponent { EntityId = character.EntityId };
var system = new PlayerCharacterSystem();

// Update (in game loop)
system.UpdateMovement(character, moveDirection, deltaTime);
system.UpdateCamera(character, camera, deltaTime);

// Interact
var interactable = system.FindInteractableInRange(character, camera.Forward);
if (interactable != null)
    system.Interact(character, interactable);
```

## 🏠 Interior

```csharp
// Generate interior
var interiorSystem = new InteriorGenerationSystem();
var interior = interiorSystem.GenerateInterior(
    ship.Ship.EntityId,
    ship.Ship.Modules.Select(m => m.Id).ToList()
);

// Add objects
var terminal = InteriorObjectLibrary.CreateTerminal();
var storage = InteriorObjectLibrary.CreateStorage();
interior.PlaceObject(terminal, new Vector3(1, 0, 1));
interior.PlaceObject(storage, new Vector3(-1, 0, 1));
```

## 📊 Ship Classes

| Class | Type | Primary | Turrets | Utility |
|-------|------|---------|---------|---------|
| Fighter_Light | Combat | 2 | 0 | 1 |
| Corvette | Multi | 2 | 0 | 2 |
| Frigate | Combat | 2 | 2 | 3 |
| Destroyer | Combat | 2 | 4 | 3 |
| Battleship | Capital | 4 | 8 | 4 |
| Miner_Medium | Mining | 1 | 0 | 3 |

## 🎨 Design Styles

- **Balanced**: 1.0x all stats (Argon-like)
- **Aggressive**: 1.3x speed, 0.8x hull (Split-like)
- **Durable**: 1.4x hull, 0.8x speed (Teladi-like)
- **Sleek**: 1.2x speed, 0.9x hull (Paranid-like)
- **Advanced**: 1.1x all stats (Terran-like)
- **Alien**: 1.0x unique look (Xenon-like)

## 📁 File Locations

**Ulysses Model**: `Assets/Models/ships/Ulysses/source/ulysses.blend`

**Formats Supported**: .blend, .obj, .fbx, .gltf, .glb, .dae, 40+ more

**Check Model**:
```csharp
var info = UlyssesModelLoader.GetModelInfo();
Console.WriteLine(info.IsAvailable ? "Model found!" : "Using procedural");
```

## 🔧 Equipment Types

**Weapons**: PrimaryWeapon, Turret, Missile  
**Tools**: MiningLaser, SalvageBeam, TractorBeam  
**Defense**: Shield, CounterMeasure  
**Utility**: Scanner, Drone, RepairBot

## 📚 Documentation

- **X4_SHIP_SYSTEM_GUIDE.md** - Complete guide
- **IMPLEMENTATION_COMPLETE_X4_SHIPS.md** - Summary
- **AvorionLike/Examples/UlyssesShipExample.cs** - Code examples

---

**Status**: ✅ Complete & Building  
**Version**: X4 System v1.0
