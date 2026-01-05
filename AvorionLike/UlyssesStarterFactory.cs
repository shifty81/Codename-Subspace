using System.Numerics;
using AvorionLike.Core;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Modular;
using AvorionLike.Core.Building;
using AvorionLike.Core.Physics;
using AvorionLike.Core.Resources;
using AvorionLike.Core.RPG;
using AvorionLike.Core.Progression;
using AvorionLike.Core.Combat;
using AvorionLike.Core.Navigation;

namespace AvorionLike;

/// <summary>
/// Factory for creating the Ulysses starter ship with interiors
/// </summary>
public static class UlyssesStarterFactory
{
    /// <summary>
    /// Create Ulysses as the player's starting ship with full interior
    /// </summary>
    public static Guid CreateUlyssesWithInterior(GameEngine gameEngine, Vector3 position, string playerName = "Player")
    {
        // Generate the Ulysses ship using the template
        var ulyssesShip = StarterShipFactory.CreateUlyssesStarterShip(playerName);
        
        // Create entity from the generated ship
        var shipEntity = gameEngine.EntityManager.CreateEntity($"{playerName}'s Ulysses");
        
        // Add modular ship component
        gameEngine.EntityManager.AddComponent(shipEntity.Id, ulyssesShip.Ship);
        
        // Note: ShipEquipmentComponent is not an IComponent, it's managed separately
        // Equipment is already part of the X4GeneratedShip structure
        
        // Create ship interior
        var interior = GenerateUlyssesInterior(ulyssesShip.Ship);
        gameEngine.EntityManager.AddComponent(shipEntity.Id, interior);
        
        // Add physics - calculate stats from ship
        float totalThrust = ulyssesShip.Ship.AggregatedStats.ThrustPower;
        
        var physicsComponent = new PhysicsComponent
        {
            Position = position,
            Velocity = Vector3.Zero,
            Mass = ulyssesShip.Ship.TotalMass,
            MomentOfInertia = CalculateMomentOfInertia(ulyssesShip.Ship),
            MaxThrust = totalThrust,
            MaxTorque = totalThrust * 0.1f // 10% of thrust as torque
        };
        gameEngine.EntityManager.AddComponent(shipEntity.Id, physicsComponent);
        
        // Add progression
        var progressionComponent = new ProgressionComponent
        {
            EntityId = shipEntity.Id,
            Level = 1,
            Experience = 0,
            SkillPoints = 0
        };
        gameEngine.EntityManager.AddComponent(shipEntity.Id, progressionComponent);
        
        // Add inventory
        var inventoryComponent = new InventoryComponent(1000);
        inventoryComponent.Inventory.AddResource(ResourceType.Credits, 10000);
        inventoryComponent.Inventory.AddResource(ResourceType.Iron, 500);
        inventoryComponent.Inventory.AddResource(ResourceType.Titanium, 200);
        gameEngine.EntityManager.AddComponent(shipEntity.Id, inventoryComponent);
        
        // Add combat
        var combatComponent = new CombatComponent
        {
            EntityId = shipEntity.Id,
            CurrentShields = ulyssesShip.Ship.AggregatedStats.ShieldCapacity,
            MaxShields = ulyssesShip.Ship.AggregatedStats.ShieldCapacity,
            ShieldRegenRate = ulyssesShip.Ship.AggregatedStats.ShieldRechargeRate,
            CurrentEnergy = ulyssesShip.Ship.AggregatedStats.PowerStorage,
            MaxEnergy = ulyssesShip.Ship.AggregatedStats.PowerStorage
        };
        gameEngine.EntityManager.AddComponent(shipEntity.Id, combatComponent);
        
        // Add sector location (start at galaxy rim)
        var sectorLocation = new SectorLocationComponent
        {
            CurrentSector = new SectorCoordinate(400, 0, 0) // Galaxy rim - Iron zone
        };
        gameEngine.EntityManager.AddComponent(shipEntity.Id, sectorLocation);
        
        return shipEntity.Id;
    }
    
    /// <summary>
    /// Generate interior for Ulysses corvette
    /// </summary>
    private static ShipInteriorComponent GenerateUlyssesInterior(ModularShipComponent ship)
    {
        var interior = new ShipInteriorComponent
        {
            EntityId = ship.EntityId,
            MaxObjects = 100
        };
        
        // Cockpit/Bridge cell
        var cockpit = new InteriorCell
        {
            ModuleId = ship.CoreModuleId ?? Guid.Empty,
            Type = InteriorCellType.Cockpit,
            MinBounds = new Vector3(-2, -2, 5),
            MaxBounds = new Vector3(2, 2, 10),
            HasGravity = true,
            HasAtmosphere = true
        };
        
        // Add cockpit furniture
        AddInteriorObject(cockpit, InteriorObjectLibrary.CreateTerminal(), new Vector3(0, 0, 8));
        AddInteriorObject(cockpit, InteriorObjectLibrary.CreateChair(), new Vector3(0, -1, 7));
        AddInteriorObject(cockpit, InteriorObjectLibrary.CreateChair(), new Vector3(-1, -1, 6));
        AddInteriorObject(cockpit, InteriorObjectLibrary.CreateChair(), new Vector3(1, -1, 6));
        
        interior.Cells.Add(cockpit);
        
        // Crew quarters
        var quarters = new InteriorCell
        {
            ModuleId = ship.CoreModuleId ?? Guid.Empty,
            Type = InteriorCellType.CrewQuarters,
            MinBounds = new Vector3(-3, -2, -2),
            MaxBounds = new Vector3(3, 2, 2),
            HasGravity = true,
            HasAtmosphere = true
        };
        
        // Add crew quarter furniture
        AddInteriorObject(quarters, InteriorObjectLibrary.CreateBed(), new Vector3(-2, -1, 0));
        AddInteriorObject(quarters, InteriorObjectLibrary.CreateBed(), new Vector3(2, -1, 0));
        AddInteriorObject(quarters, InteriorObjectLibrary.CreateLocker(), new Vector3(-2, 1, 1));
        AddInteriorObject(quarters, InteriorObjectLibrary.CreateLocker(), new Vector3(2, 1, 1));
        
        interior.Cells.Add(quarters);
        
        // Cargo bay
        var cargoBay = new InteriorCell
        {
            ModuleId = ship.CoreModuleId ?? Guid.Empty,
            Type = InteriorCellType.Cargo,
            MinBounds = new Vector3(-3, -2, -8),
            MaxBounds = new Vector3(3, 2, -3),
            HasGravity = true,
            HasAtmosphere = true
        };
        
        // Add cargo furniture
        AddInteriorObject(cargoBay, InteriorObjectLibrary.CreateStorage(), new Vector3(-2, -1, -6));
        AddInteriorObject(cargoBay, InteriorObjectLibrary.CreateStorage(), new Vector3(2, -1, -6));
        AddInteriorObject(cargoBay, InteriorObjectLibrary.CreateCrate(), new Vector3(0, -1, -5));
        
        interior.Cells.Add(cargoBay);
        
        // Engine room
        var engineRoom = new InteriorCell
        {
            ModuleId = ship.CoreModuleId ?? Guid.Empty,
            Type = InteriorCellType.Engine,
            MinBounds = new Vector3(-2, -2, -12),
            MaxBounds = new Vector3(2, 2, -9),
            HasGravity = true,
            HasAtmosphere = true
        };
        
        // Add engine room objects
        AddInteriorObject(engineRoom, InteriorObjectLibrary.CreatePowerNode(), new Vector3(0, 0, -10));
        AddInteriorObject(engineRoom, InteriorObjectLibrary.CreateWorkbench(), new Vector3(-1, -1, -10));
        
        interior.Cells.Add(engineRoom);
        
        // Connect cells with corridors
        cockpit.ConnectedCells.Add(quarters.Id);
        quarters.ConnectedCells.Add(cockpit.Id);
        quarters.ConnectedCells.Add(cargoBay.Id);
        cargoBay.ConnectedCells.Add(quarters.Id);
        cargoBay.ConnectedCells.Add(engineRoom.Id);
        engineRoom.ConnectedCells.Add(cargoBay.Id);
        
        return interior;
    }
    
    /// <summary>
    /// Helper to add interior object to a cell
    /// </summary>
    private static void AddInteriorObject(InteriorCell cell, InteriorObject obj, Vector3 position)
    {
        obj.Position = position;
        cell.PlacedObjects.Add(obj);
    }
    
    /// <summary>
    /// Calculate moment of inertia for a modular ship
    /// </summary>
    private static float CalculateMomentOfInertia(ModularShipComponent ship)
    {
        if (ship.Modules.Count == 0)
            return 1000f;
        
        float totalMass = ship.TotalMass;
        float avgRadius = 5f; // Approximate radius
        
        // Use parallel axis theorem for distributed mass
        float I = totalMass * avgRadius * avgRadius;
        return I;
    }
}

/// <summary>
/// Library of interior objects that can be placed
/// </summary>
public static class InteriorObjectLibrary
{
    public static InteriorObject CreateTerminal()
    {
        return new InteriorObject
        {
            Type = InteriorObjectType.Terminal,
            Name = "Computer Terminal",
            Size = new Vector3(0.8f, 1.2f, 0.4f),
            IsInteractable = true,
            InteractionPrompt = "Press E to access terminal",
            RequiresPower = true,
            PowerConsumption = 50f,
            Color = (50, 100, 150)
        };
    }
    
    public static InteriorObject CreateChair()
    {
        return new InteriorObject
        {
            Type = InteriorObjectType.Chair,
            Name = "Chair",
            Size = new Vector3(0.6f, 1.0f, 0.6f),
            RequiresFloor = true,
            Color = (80, 80, 80)
        };
    }
    
    public static InteriorObject CreateStorage()
    {
        return new InteriorObject
        {
            Type = InteriorObjectType.Storage,
            Name = "Storage Container",
            Size = new Vector3(1.2f, 1.5f, 0.8f),
            IsInteractable = true,
            InteractionPrompt = "Press E to open storage",
            RequiresFloor = true,
            Color = (120, 120, 120)
        };
    }
    
    public static InteriorObject CreateBed()
    {
        return new InteriorObject
        {
            Type = InteriorObjectType.Bed,
            Name = "Crew Bed",
            Size = new Vector3(2.0f, 0.8f, 1.0f),
            IsInteractable = true,
            InteractionPrompt = "Press E to rest",
            RequiresFloor = true,
            Color = (100, 100, 120)
        };
    }
    
    public static InteriorObject CreateLocker()
    {
        return new InteriorObject
        {
            Type = InteriorObjectType.Locker,
            Name = "Locker",
            Size = new Vector3(0.6f, 2.0f, 0.5f),
            IsInteractable = true,
            InteractionPrompt = "Press E to open locker",
            RequiresFloor = true,
            RequiresWall = true,
            Color = (90, 90, 90)
        };
    }
    
    public static InteriorObject CreateCrate()
    {
        return new InteriorObject
        {
            Type = InteriorObjectType.Crate,
            Name = "Cargo Crate",
            Size = new Vector3(1.0f, 1.0f, 1.0f),
            IsInteractable = true,
            InteractionPrompt = "Press E to examine",
            RequiresFloor = true,
            Color = (140, 120, 80)
        };
    }
    
    public static InteriorObject CreateWorkbench()
    {
        return new InteriorObject
        {
            Type = InteriorObjectType.Workbench,
            Name = "Workbench",
            Size = new Vector3(1.5f, 1.0f, 0.8f),
            IsInteractable = true,
            InteractionPrompt = "Press E to craft",
            RequiresPower = true,
            PowerConsumption = 100f,
            RequiresFloor = true,
            Color = (130, 130, 130)
        };
    }
    
    public static InteriorObject CreatePowerNode()
    {
        return new InteriorObject
        {
            Type = InteriorObjectType.PowerNode,
            Name = "Power Node",
            Size = new Vector3(0.8f, 0.8f, 0.4f),
            RequiresPower = false,
            RequiresWall = true,
            Color = (200, 150, 50)
        };
    }
}
