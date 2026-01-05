using System.Numerics;
using AvorionLike.Core.ECS;

namespace AvorionLike.Core.Building;

/// <summary>
/// Interior object types that can be placed inside ships
/// Like No Man's Sky frigate builder
/// </summary>
public enum InteriorObjectType
{
    // Functional
    Terminal,           // Computer terminal
    Storage,            // Storage container
    Workbench,          // Crafting station
    MedicalStation,     // Healing/repair station
    WeaponRack,         // Weapon storage
    BedPod,             // Sleep/save point
    TeleportPad,        // Teleporter
    TurretControl,      // Manual turret control
    
    // Furniture
    Chair,
    Table,
    Bed,
    Sofa,
    Desk,
    
    // Decoration
    Plant,
    Poster,
    Light,
    Locker,
    Crate,
    Barrel,
    
    // Technical
    PowerNode,          // Power junction
    LifeSupportNode,    // Life support system
    DataNode,           // Computer network node
    CargoPallet         // Cargo storage pallet
}

/// <summary>
/// Placed interior object instance
/// </summary>
public class InteriorObject
{
    public Guid Id { get; set; } = Guid.NewGuid();
    public InteriorObjectType Type { get; set; }
    public string Name { get; set; } = "Object";
    
    // Position and rotation
    public Vector3 Position { get; set; }
    public Vector3 Rotation { get; set; } // Euler angles
    
    // Size (bounding box)
    public Vector3 Size { get; set; } = new Vector3(1, 1, 1);
    
    // Snap to grid
    public bool SnapToGrid { get; set; } = true;
    public float GridSize { get; set; } = 0.5f; // meters
    
    // Visual
    public string ModelPath { get; set; } = "";
    public (int R, int G, int B) Color { get; set; } = (255, 255, 255);
    
    // Functionality
    public bool IsInteractable { get; set; } = false;
    public string InteractionPrompt { get; set; } = "";
    
    // Requirements
    public bool RequiresPower { get; set; } = false;
    public float PowerConsumption { get; set; } = 0f;
    
    // Placement rules
    public bool RequiresFloor { get; set; } = true;
    public bool RequiresWall { get; set; } = false;
    public bool RequiresCeiling { get; set; } = false;
}

/// <summary>
/// Interior cell (room) within a ship
/// Generated from ship modules
/// </summary>
public class InteriorCell
{
    public Guid Id { get; set; } = Guid.NewGuid();
    public Guid ModuleId { get; set; } // Which ship module this interior belongs to
    
    // Room bounds
    public Vector3 MinBounds { get; set; }
    public Vector3 MaxBounds { get; set; }
    
    // Room type
    public InteriorCellType Type { get; set; } = InteriorCellType.Generic;
    
    // Placed objects
    public List<InteriorObject> PlacedObjects { get; set; } = new();
    
    // Connectivity
    public List<Guid> ConnectedCells { get; set; } = new(); // Adjacent rooms
    public List<Vector3> DoorPositions { get; set; } = new(); // Door locations
    
    // Environment
    public bool HasGravity { get; set; } = true;
    public bool HasAtmosphere { get; set; } = true;
    public float Temperature { get; set; } = 20f; // Celsius
    
    /// <summary>
    /// Get room volume
    /// </summary>
    public float GetVolume()
    {
        var size = MaxBounds - MinBounds;
        return size.X * size.Y * size.Z;
    }
    
    /// <summary>
    /// Check if position is within this cell
    /// </summary>
    public bool ContainsPosition(Vector3 position)
    {
        return position.X >= MinBounds.X && position.X <= MaxBounds.X &&
               position.Y >= MinBounds.Y && position.Y <= MaxBounds.Y &&
               position.Z >= MinBounds.Z && position.Z <= MaxBounds.Z;
    }
}

/// <summary>
/// Interior cell types
/// </summary>
public enum InteriorCellType
{
    Generic,
    Cockpit,
    Engine,
    Cargo,
    Corridor,
    PowerCore,
    MedBay,
    CrewQuarters,
    Bridge,
    Armory,
    Laboratory,
    Hangar
}

/// <summary>
/// Component for ship interiors
/// Attached to ModularShipComponent entities
/// </summary>
public class ShipInteriorComponent : IComponent
{
    public Guid EntityId { get; set; }
    
    /// <summary>
    /// All interior cells in this ship
    /// </summary>
    public List<InteriorCell> Cells { get; set; } = new();
    
    /// <summary>
    /// Build mode enabled (allows placing objects)
    /// </summary>
    public bool BuildModeActive { get; set; } = false;
    
    /// <summary>
    /// Maximum number of objects allowed
    /// </summary>
    public int MaxObjects { get; set; } = 100;
    
    /// <summary>
    /// Get total number of placed objects
    /// </summary>
    public int GetTotalObjectCount()
    {
        return Cells.Sum(c => c.PlacedObjects.Count);
    }
    
    /// <summary>
    /// Find cell containing position
    /// </summary>
    public InteriorCell? FindCellAtPosition(Vector3 position)
    {
        return Cells.FirstOrDefault(c => c.ContainsPosition(position));
    }
    
    /// <summary>
    /// Add a cell
    /// </summary>
    public void AddCell(InteriorCell cell)
    {
        Cells.Add(cell);
    }
    
    /// <summary>
    /// Place an object in the interior
    /// </summary>
    public bool PlaceObject(InteriorObject obj, Vector3 position)
    {
        var cell = FindCellAtPosition(position);
        if (cell == null) return false;
        
        if (GetTotalObjectCount() >= MaxObjects) return false;
        
        // Snap to grid if enabled
        if (obj.SnapToGrid)
        {
            position = SnapToGrid(position, obj.GridSize);
        }
        
        obj.Position = position;
        
        // Check for collisions with existing objects
        if (CheckCollision(cell, obj)) return false;
        
        cell.PlacedObjects.Add(obj);
        return true;
    }
    
    /// <summary>
    /// Remove an object
    /// </summary>
    public bool RemoveObject(Guid objectId)
    {
        foreach (var cell in Cells)
        {
            var obj = cell.PlacedObjects.FirstOrDefault(o => o.Id == objectId);
            if (obj != null)
            {
                cell.PlacedObjects.Remove(obj);
                return true;
            }
        }
        return false;
    }
    
    /// <summary>
    /// Snap position to grid
    /// </summary>
    private Vector3 SnapToGrid(Vector3 position, float gridSize)
    {
        return new Vector3(
            (float)Math.Round(position.X / gridSize) * gridSize,
            (float)Math.Round(position.Y / gridSize) * gridSize,
            (float)Math.Round(position.Z / gridSize) * gridSize
        );
    }
    
    /// <summary>
    /// Check if object collides with existing objects
    /// </summary>
    private bool CheckCollision(InteriorCell cell, InteriorObject obj)
    {
        foreach (var existing in cell.PlacedObjects)
        {
            // Simple AABB collision
            var objMin = obj.Position - obj.Size * 0.5f;
            var objMax = obj.Position + obj.Size * 0.5f;
            var existingMin = existing.Position - existing.Size * 0.5f;
            var existingMax = existing.Position + existing.Size * 0.5f;
            
            if (objMin.X < existingMax.X && objMax.X > existingMin.X &&
                objMin.Y < existingMax.Y && objMax.Y > existingMin.Y &&
                objMin.Z < existingMax.Z && objMax.Z > existingMin.Z)
            {
                return true; // Collision detected
            }
        }
        return false;
    }
}

/// <summary>
/// System for generating interior spaces from ship modules
/// </summary>
public class InteriorGenerationSystem
{
    /// <summary>
    /// Generate interior cells for a modular ship
    /// </summary>
    public ShipInteriorComponent GenerateInterior(Guid shipEntityId, List<Guid> moduleIds)
    {
        var interior = new ShipInteriorComponent
        {
            EntityId = shipEntityId
        };
        
        // Generate a cell for each module
        foreach (var moduleId in moduleIds)
        {
            var cell = GenerateCellForModule(moduleId);
            interior.AddCell(cell);
        }
        
        // Connect adjacent cells with corridors
        ConnectCells(interior);
        
        return interior;
    }
    
    /// <summary>
    /// Generate an interior cell for a module
    /// </summary>
    private InteriorCell GenerateCellForModule(Guid moduleId)
    {
        // Simplified - in reality would check module type and size
        var cell = new InteriorCell
        {
            ModuleId = moduleId,
            MinBounds = new Vector3(-2, 0, -2),
            MaxBounds = new Vector3(2, 2.5f, 2),
            Type = InteriorCellType.Generic,
            HasGravity = true,
            HasAtmosphere = true
        };
        
        return cell;
    }
    
    /// <summary>
    /// Connect adjacent cells with doors
    /// </summary>
    private void ConnectCells(ShipInteriorComponent interior)
    {
        // Simplified - would check module connections and generate corridors
        for (int i = 0; i < interior.Cells.Count - 1; i++)
        {
            var cell1 = interior.Cells[i];
            var cell2 = interior.Cells[i + 1];
            
            cell1.ConnectedCells.Add(cell2.Id);
            cell2.ConnectedCells.Add(cell1.Id);
            
            // Add door position
            var doorPos = (cell1.MaxBounds + cell2.MinBounds) * 0.5f;
            cell1.DoorPositions.Add(doorPos);
            cell2.DoorPositions.Add(doorPos);
        }
    }
}

/// <summary>
/// Library of interior object definitions
/// </summary>
public static class InteriorObjectLibrary
{
    /// <summary>
    /// Create a terminal object
    /// </summary>
    public static InteriorObject CreateTerminal()
    {
        return new InteriorObject
        {
            Type = InteriorObjectType.Terminal,
            Name = "Computer Terminal",
            Size = new Vector3(0.8f, 1.5f, 0.4f),
            IsInteractable = true,
            InteractionPrompt = "Press E to access terminal",
            RequiresPower = true,
            PowerConsumption = 10f,
            RequiresWall = true,
            ModelPath = "interior/terminal.obj",
            Color = (100, 150, 200)
        };
    }
    
    /// <summary>
    /// Create a storage container
    /// </summary>
    public static InteriorObject CreateStorage()
    {
        return new InteriorObject
        {
            Type = InteriorObjectType.Storage,
            Name = "Storage Container",
            Size = new Vector3(1.0f, 1.2f, 0.6f),
            IsInteractable = true,
            InteractionPrompt = "Press E to open storage",
            RequiresFloor = true,
            ModelPath = "interior/storage.obj",
            Color = (150, 150, 150)
        };
    }
    
    /// <summary>
    /// Create a chair
    /// </summary>
    public static InteriorObject CreateChair()
    {
        return new InteriorObject
        {
            Type = InteriorObjectType.Chair,
            Name = "Chair",
            Size = new Vector3(0.6f, 1.0f, 0.6f),
            IsInteractable = true,
            InteractionPrompt = "Press E to sit",
            RequiresFloor = true,
            ModelPath = "interior/chair.obj",
            Color = (80, 80, 100)
        };
    }
    
    /// <summary>
    /// Create a workbench
    /// </summary>
    public static InteriorObject CreateWorkbench()
    {
        return new InteriorObject
        {
            Type = InteriorObjectType.Workbench,
            Name = "Workbench",
            Size = new Vector3(2.0f, 1.0f, 1.0f),
            IsInteractable = true,
            InteractionPrompt = "Press E to use workbench",
            RequiresPower = true,
            PowerConsumption = 20f,
            RequiresFloor = true,
            ModelPath = "interior/workbench.obj",
            Color = (120, 120, 120)
        };
    }
}
