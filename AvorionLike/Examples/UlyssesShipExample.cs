using AvorionLike.Core.Graphics;
using AvorionLike.Core.Logging;
using AvorionLike.Core.Modular;

namespace AvorionLike.Core.Examples;

/// <summary>
/// Example demonstrating how to use the Ulysses starter ship
/// </summary>
public class UlyssesShipExample
{
    private readonly Logger _logger = Logger.Instance;
    
    /// <summary>
    /// Create and configure a Ulysses starter ship for a new player
    /// </summary>
    public void CreatePlayerStarterShip(string playerName)
    {
        _logger.Info("Example", "=== Creating Ulysses Starter Ship ===");
        
        // Method 1: Use the factory (simplest)
        var ship = StarterShipFactory.CreateUlyssesStarterShip(playerName);
        
        LogShipDetails(ship);
        
        // The ship is now ready to use with:
        // - Hull: ship.Ship (ModularShipComponent)
        // - Equipment: ship.Equipment (ShipEquipmentComponent)
        // - Paint: ship.Paint (ShipPaintScheme)
        // - Stats: ship.Stats (X4ShipStats)
    }
    
    /// <summary>
    /// Create a customized Ulysses with different loadout
    /// </summary>
    public void CreateCustomUlysses()
    {
        _logger.Info("Example", "=== Creating Custom Ulysses ===");
        
        // Create basic Ulysses
        var ship = StarterShipFactory.CreateUlysses("Mining Rig", seed: 12345);
        
        // Customize equipment for mining
        var miningLaser2 = EquipmentFactory.CreateMiningLaser(2); // Tier 2 laser
        var salvageBeam = EquipmentFactory.CreateSalvageBeam(1);
        
        // Find utility slots and equip mining gear
        var utilitySlots = ship.Equipment.EquipmentSlots
            .Where(s => s.AllowedType == EquipmentType.MiningLaser || 
                       s.AllowedType == EquipmentType.SalvageBeam)
            .ToList();
        
        if (utilitySlots.Count >= 2)
        {
            ship.Equipment.EquipItem(utilitySlots[0].Id, miningLaser2);
            ship.Equipment.EquipItem(utilitySlots[1].Id, salvageBeam);
        }
        
        // Change paint to "Merchant Gold"
        var goldPaint = PaintLibrary.GetPaintsByQuality(PaintQuality.Exceptional)
            .FirstOrDefault(p => p.Name.Contains("Gold"));
        
        if (goldPaint != null)
        {
            ship.Paint = goldPaint;
        }
        
        LogShipDetails(ship);
    }
    
    /// <summary>
    /// Load the Ulysses 3D model directly
    /// </summary>
    public void LoadUlyssesModel()
    {
        _logger.Info("Example", "=== Loading Ulysses 3D Model ===");
        
        var assetManager = AssetManager.Instance;
        
        // Try to load the model (will check for .blend first, then fallbacks)
        var modelPaths = new[]
        {
            "ships/hulls/ulysses.blend",
            "ships/hulls/ulysses.obj",
            "ships/hulls/ulysses.fbx",
            "ships/hulls/ulysses.gltf"
        };
        
        List<MeshData>? meshes = null;
        string? loadedPath = null;
        
        foreach (var path in modelPaths)
        {
            try
            {
                // Check if file exists - need to prepend "Models/" for GetAssetPath
                var fullPath = assetManager.GetAssetPath("Models/" + path);
                if (File.Exists(fullPath))
                {
                    // LoadModel expects path relative to Assets/Models/, so use path without "Models/"
                    meshes = assetManager.LoadModel(path);
                    loadedPath = path;
                    break;
                }
            }
            catch
            {
                // Try next format
                continue;
            }
        }
        
        if (meshes != null && meshes.Count > 0)
        {
            _logger.Info("Example", $"Loaded Ulysses model from: {loadedPath}");
            _logger.Info("Example", $"Model contains {meshes.Count} mesh(es)");
            
            foreach (var mesh in meshes)
            {
                _logger.Info("Example", 
                    $"  - {mesh.Name}: {mesh.VertexCount} vertices, {mesh.TriangleCount} triangles");
            }
        }
        else
        {
            _logger.Warning("Example", "Ulysses model not found. Using procedural generation.");
            _logger.Info("Example", "Place ulysses.blend in Assets/Models/ships/hulls/ to use custom model");
        }
    }
    
    /// <summary>
    /// Create Ulysses with player character for interior walking
    /// </summary>
    public void CreateUlyssesWithInterior()
    {
        _logger.Info("Example", "=== Creating Ulysses with Interior ===");
        
        // Create ship
        var ship = StarterShipFactory.CreateUlyssesStarterShip("Captain");
        
        // Generate interior
        var interiorSystem = new AvorionLike.Core.Building.InteriorGenerationSystem();
        var moduleIds = ship.Ship.Modules.Select(m => m.Id).ToList();
        var interior = interiorSystem.GenerateInterior(ship.Ship.EntityId, moduleIds);
        
        _logger.Info("Example", $"Generated {interior.Cells.Count} interior cells");
        
        // Add some interior objects
        var terminal = AvorionLike.Core.Building.InteriorObjectLibrary.CreateTerminal();
        var storage = AvorionLike.Core.Building.InteriorObjectLibrary.CreateStorage();
        var chair = AvorionLike.Core.Building.InteriorObjectLibrary.CreateChair();
        
        if (interior.Cells.Count > 0)
        {
            var cockpit = interior.Cells[0];
            interior.PlaceObject(terminal, cockpit.MinBounds + new System.Numerics.Vector3(1, 0, 1));
            interior.PlaceObject(storage, cockpit.MinBounds + new System.Numerics.Vector3(-1, 0, 1));
            interior.PlaceObject(chair, cockpit.MinBounds + new System.Numerics.Vector3(0, 0, 0.5f));
            
            _logger.Info("Example", $"Placed {interior.GetTotalObjectCount()} interior objects");
        }
        
        // Create player character
        var character = new AvorionLike.Core.RPG.PlayerCharacterComponent
        {
            EntityId = Guid.NewGuid(),
            Position = new System.Numerics.Vector3(0, 0.9f, 0), // Standing in cockpit
            CurrentShipId = ship.Ship.EntityId
        };
        
        _logger.Info("Example", "Player character ready - can walk around interior!");
    }
    
    /// <summary>
    /// Demonstrate all X4-style ship classes
    /// </summary>
    public void ShowAllShipClasses()
    {
        _logger.Info("Example", "=== X4 Ship Classes ===");
        
        var library = new ModuleLibrary();
        library.InitializeBuiltInModules();
        var generator = new X4ShipGenerator(library);
        
        // Generate one of each class
        var shipClasses = new[]
        {
            X4ShipClass.Fighter_Light,
            X4ShipClass.Corvette,
            X4ShipClass.Frigate,
            X4ShipClass.Destroyer,
            X4ShipClass.Battleship,
            X4ShipClass.Miner_Medium,
            X4ShipClass.Freighter_Large
        };
        
        foreach (var shipClass in shipClasses)
        {
            var config = new X4ShipConfig
            {
                ShipClass = shipClass,
                DesignStyle = X4DesignStyle.Balanced,
                ShipName = $"Test {shipClass}",
                Seed = 42
            };
            
            var ship = generator.GenerateX4Ship(config);
            
            _logger.Info("Example", $"\n{shipClass}:");
            _logger.Info("Example", $"  Slots: {ship.Equipment.EquipmentSlots.Count}");
            _logger.Info("Example", $"  Mass: {ship.Stats.Mass:F0}t");
            _logger.Info("Example", $"  Hull: {ship.Stats.Hull:F0}");
            _logger.Info("Example", $"  Speed: {ship.Stats.Speed:F1} m/s");
        }
    }
    
    private void LogShipDetails(X4GeneratedShip ship)
    {
        _logger.Info("Example", $"\nShip: {ship.Ship.Name}");
        _logger.Info("Example", $"Class: {ship.Config.ShipClass}");
        _logger.Info("Example", $"Style: {ship.Config.DesignStyle}");
        _logger.Info("Example", $"\nStats:");
        _logger.Info("Example", ship.Stats.ToString());
        _logger.Info("Example", $"\nPaint: {ship.Paint.Name}");
        _logger.Info("Example", $"  Primary: RGB({ship.Paint.PrimaryColor.R}, {ship.Paint.PrimaryColor.G}, {ship.Paint.PrimaryColor.B})");
        _logger.Info("Example", $"\nEquipment:");
        
        foreach (var slot in ship.Equipment.EquipmentSlots)
        {
            if (slot.IsOccupied)
            {
                _logger.Info("Example", $"  {slot.MountName}: {slot.EquippedItem!.Name}");
            }
        }
    }
}
