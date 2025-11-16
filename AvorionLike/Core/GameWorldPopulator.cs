using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Physics;
using AvorionLike.Core.Resources;
using AvorionLike.Core.Combat;
using AvorionLike.Core.Navigation;
using AvorionLike.Core.AI;
using AvorionLike.Core.RPG;
using AvorionLike.Core.Progression;
using AvorionLike.Core.Procedural;

namespace AvorionLike.Core;

/// <summary>
/// Populates the game world with AI ships, asteroids, stations, and other entities
/// Creates a living, breathing universe for the player to interact with
/// </summary>
public class GameWorldPopulator
{
    private readonly GameEngine _gameEngine;
    private readonly Random _random;
    
    public GameWorldPopulator(GameEngine gameEngine, int? seed = null)
    {
        _gameEngine = gameEngine;
        _random = seed.HasValue ? new Random(seed.Value) : new Random();
    }
    
    /// <summary>
    /// Creates a populated starter area with various entities for the player
    /// </summary>
    public void PopulateStarterArea(Vector3 playerPosition, float radius = 1000f)
    {
        Console.WriteLine($"Populating starter area around {playerPosition} (radius: {radius}m)...");
        
        // Create asteroid field for mining
        CreateAsteroidField(playerPosition, radius, 15);
        
        // Create friendly traders
        CreateTraderShips(playerPosition, radius, 3);
        
        // Create neutral miners
        CreateMinerShips(playerPosition, radius, 4);
        
        // Create some pirates (but not too close to player)
        CreatePirateShips(playerPosition, radius * 0.8f, 2);
        
        // Create a friendly station
        CreateStation(playerPosition + new Vector3(radius * 0.5f, 0, 0), StationType.TradingPost, "Haven Station");
        
        Console.WriteLine("Starter area population complete!");
    }
    
    /// <summary>
    /// Creates a populated area with zone-appropriate content based on galaxy progression
    /// </summary>
    public void PopulateZoneArea(Vector3 playerPosition, SectorCoordinate sector, float radius = 1000f)
    {
        int distanceFromCenter = GalaxyProgressionSystem.GetDistanceFromCenter(sector);
        var zoneName = GalaxyProgressionSystem.GetZoneName(distanceFromCenter);
        var availableTier = GalaxyProgressionSystem.GetAvailableMaterialTier(distanceFromCenter);
        var difficulty = GalaxyProgressionSystem.GetDifficultyMultiplier(distanceFromCenter);
        var spawnRate = GalaxyProgressionSystem.GetEnemySpawnRate(distanceFromCenter);
        
        Console.WriteLine($"Populating {zoneName} at sector [{sector.X}, {sector.Y}, {sector.Z}]");
        Console.WriteLine($"  Distance from center: {distanceFromCenter} sectors");
        Console.WriteLine($"  Available material: {availableTier}");
        Console.WriteLine($"  Difficulty: {difficulty:F1}x");
        Console.WriteLine($"  Enemy spawn rate: {spawnRate:F1}x");
        
        // Create zone-appropriate asteroids with correct material types
        CreateZoneAsteroidField(playerPosition, radius, 15, availableTier);
        
        // Scale ship counts by zone
        int traderCount = Math.Max(1, (int)(3 * (1.0f / difficulty))); // Fewer traders in dangerous zones
        int minerCount = Math.Max(2, (int)(4 * (1.0f / difficulty)));  // Fewer miners in dangerous zones
        int pirateCount = (int)(2 * spawnRate * difficulty);            // More pirates in dangerous zones
        
        CreateTraderShips(playerPosition, radius, traderCount);
        CreateMinerShips(playerPosition, radius, minerCount);
        CreatePirateShips(playerPosition, radius * 0.8f, pirateCount);
        
        // Create zone-appropriate station
        var stationType = DetermineStationType(availableTier, distanceFromCenter);
        var stationName = GenerateStationName(stationType, zoneName);
        CreateStation(playerPosition + new Vector3(radius * 0.5f, 0, 0), stationType, stationName);
        
        Console.WriteLine("Zone population complete!");
    }
    
    /// <summary>
    /// Determine appropriate station type for the zone
    /// </summary>
    private StationType DetermineStationType(MaterialTier tier, int distance)
    {
        // Core zones get better stations
        if (distance < 50) return StationType.Research;
        if (distance < 150) return StationType.Military;
        if (distance < 250) return StationType.Mining;
        return StationType.TradingPost;
    }
    
    /// <summary>
    /// Generate a contextual station name
    /// </summary>
    private string GenerateStationName(StationType type, string zoneName)
    {
        var prefix = type switch
        {
            StationType.TradingPost => "Trading Hub",
            StationType.Military => "Defense Station",
            StationType.Research => "Research Outpost",
            StationType.Mining => "Mining Station",
            _ => "Station"
        };
        
        var zonePrefix = zoneName.Contains("Core") ? "Core" :
                        zoneName.Contains("Frontier") ? "Frontier" :
                        zoneName.Contains("Rim") ? "Rim" : "";
        
        return $"{zonePrefix} {prefix} {_random.Next(100, 999)}";
    }
    
    /// <summary>
    /// Creates asteroids with zone-appropriate materials
    /// </summary>
    private void CreateZoneAsteroidField(Vector3 center, float radius, int count, MaterialTier maxTier)
    {
        Console.WriteLine($"  Creating {count} asteroids (up to {maxTier} materials)...");
        
        // Determine which resource types are available in this zone
        var availableResources = new List<ResourceType>();
        
        // Always have Iron
        availableResources.Add(ResourceType.Iron);
        
        if (maxTier >= MaterialTier.Titanium) availableResources.Add(ResourceType.Titanium);
        if (maxTier >= MaterialTier.Naonite) availableResources.Add(ResourceType.Naonite);
        if (maxTier >= MaterialTier.Trinium) availableResources.Add(ResourceType.Trinium);
        if (maxTier >= MaterialTier.Xanion) availableResources.Add(ResourceType.Xanion);
        if (maxTier >= MaterialTier.Ogonite) availableResources.Add(ResourceType.Ogonite);
        if (maxTier >= MaterialTier.Avorion) availableResources.Add(ResourceType.Avorion);
        
        for (int i = 0; i < count; i++)
        {
            // Random position within radius
            var offset = new Vector3(
                (float)(_random.NextDouble() * 2 - 1) * radius,
                (float)(_random.NextDouble() * 2 - 1) * radius * 0.3f,
                (float)(_random.NextDouble() * 2 - 1) * radius
            );
            
            var position = center + offset;
            var size = (float)(_random.NextDouble() * 10 + 5);
            
            // Weight resources towards the highest tier in the zone
            ResourceType resourceType;
            var tierRoll = _random.NextDouble();
            if (tierRoll > 0.7 && availableResources.Count > 0)
            {
                // 30% chance of highest tier
                resourceType = availableResources[availableResources.Count - 1];
            }
            else if (tierRoll > 0.4 && availableResources.Count > 1)
            {
                // 30% chance of second highest tier
                resourceType = availableResources[Math.Max(0, availableResources.Count - 2)];
            }
            else
            {
                // 40% chance of any lower tier
                resourceType = availableResources[_random.Next(availableResources.Count)];
            }
            
            CreateAsteroid(position, size, resourceType, $"Asteroid-{i + 1}");
        }
    }
    
    /// <summary>
    /// Creates an asteroid field for mining
    /// </summary>
    private void CreateAsteroidField(Vector3 center, float radius, int count)
    {
        Console.WriteLine($"  Creating {count} asteroids...");
        
        for (int i = 0; i < count; i++)
        {
            // Random position within radius
            var offset = new Vector3(
                (float)(_random.NextDouble() * 2 - 1) * radius,
                (float)(_random.NextDouble() * 2 - 1) * radius * 0.3f, // Less vertical spread
                (float)(_random.NextDouble() * 2 - 1) * radius
            );
            
            var position = center + offset;
            
            // Random asteroid properties
            var size = (float)(_random.NextDouble() * 10 + 5); // 5-15 meters
            var resourceTypes = Enum.GetValues<ResourceType>().Where(r => r != ResourceType.Credits).ToArray();
            var resourceType = resourceTypes[_random.Next(resourceTypes.Length)];
            
            CreateAsteroid(position, size, resourceType, $"Asteroid-{i + 1}");
        }
    }
    
    /// <summary>
    /// Creates a single minable asteroid
    /// </summary>
    private Guid CreateAsteroid(Vector3 position, float size, ResourceType resourceType, string name)
    {
        var asteroid = _gameEngine.EntityManager.CreateEntity(name);
        
        // Create voxel structure
        var voxelComponent = new VoxelStructureComponent();
        
        // Choose material based on resource type for visual distinction
        string asteroidMaterial = resourceType switch
        {
            ResourceType.Iron => "Iron",        // Gray - basic asteroids
            ResourceType.Titanium => "Titanium", // Blue-gray - rare asteroids
            ResourceType.Naonite => "Naonite",   // Green - exotic asteroids
            ResourceType.Trinium => "Trinium",   // Blue - valuable asteroids
            ResourceType.Xanion => "Xanion",     // Gold - precious asteroids
            ResourceType.Ogonite => "Ogonite",   // Red/Orange - rare asteroids
            ResourceType.Avorion => "Avorion",   // Purple - ultra-rare asteroids
            _ => "Iron"
        };
        
        // Create MORE blocks for substantial asteroid appearance (10-20 blocks)
        int blockCount = Math.Max(10, (int)(size / 2) + _random.Next(5, 10));
        
        // Create a core cluster
        for (int i = 0; i < blockCount; i++)
        {
            // Create irregular, organic asteroid shape
            float angle = (float)(i * 2 * Math.PI / blockCount);
            float distance = (float)(_random.NextDouble() * size * 0.4f);
            
            var blockOffset = new Vector3(
                (float)(Math.Cos(angle) * distance + (_random.NextDouble() * 2 - 1) * size * 0.2f),
                (float)(_random.NextDouble() * 2 - 1) * size * 0.4f,
                (float)(Math.Sin(angle) * distance + (_random.NextDouble() * 2 - 1) * size * 0.2f)
            );
            
            // Vary block sizes for irregular appearance
            float blockSizeVariation = (float)(_random.NextDouble() * 0.5f + 0.5f); // 0.5 to 1.0
            var blockSize = size * 0.5f * blockSizeVariation;
            
            voxelComponent.AddBlock(new VoxelBlock(
                blockOffset,
                new Vector3(blockSize, blockSize * (0.8f + (float)_random.NextDouble() * 0.4f), blockSize),
                asteroidMaterial,
                BlockType.Armor // Use Armor block type for rough, rocky appearance
            ));
        }
        
        // Add some outlying chunks for more interesting shape (3-6 extra blocks)
        int outlierCount = _random.Next(3, 7);
        for (int i = 0; i < outlierCount; i++)
        {
            var outlierOffset = new Vector3(
                (float)(_random.NextDouble() * 2 - 1) * size * 0.8f,
                (float)(_random.NextDouble() * 2 - 1) * size * 0.6f,
                (float)(_random.NextDouble() * 2 - 1) * size * 0.8f
            );
            
            float outlierSize = size * 0.3f * (0.5f + (float)_random.NextDouble() * 0.5f);
            
            voxelComponent.AddBlock(new VoxelBlock(
                outlierOffset,
                new Vector3(outlierSize, outlierSize, outlierSize),
                asteroidMaterial,
                BlockType.Armor
            ));
        }
        
        _gameEngine.EntityManager.AddComponent(asteroid.Id, voxelComponent);
        
        // Add physics
        var physicsComponent = new PhysicsComponent
        {
            Position = position,
            Mass = voxelComponent.TotalMass,
            Velocity = new Vector3(
                (float)(_random.NextDouble() * 2 - 1) * 2f,
                (float)(_random.NextDouble() * 2 - 1) * 2f,
                (float)(_random.NextDouble() * 2 - 1) * 2f
            ) // Slow drift
        };
        _gameEngine.EntityManager.AddComponent(asteroid.Id, physicsComponent);
        
        // Add inventory with resources to mine
        var inventoryComponent = new InventoryComponent(1000);
        int resourceAmount = (int)(size * size * 10); // More resources in larger asteroids
        inventoryComponent.Inventory.AddResource(resourceType, resourceAmount);
        _gameEngine.EntityManager.AddComponent(asteroid.Id, inventoryComponent);
        
        return asteroid.Id;
    }
    
    /// <summary>
    /// Creates AI trader ships
    /// </summary>
    private void CreateTraderShips(Vector3 center, float radius, int count)
    {
        Console.WriteLine($"  Creating {count} trader ships...");
        
        for (int i = 0; i < count; i++)
        {
            var position = center + new Vector3(
                (float)(_random.NextDouble() * 2 - 1) * radius * 0.6f,
                (float)(_random.NextDouble() * 2 - 1) * radius * 0.2f,
                (float)(_random.NextDouble() * 2 - 1) * radius * 0.6f
            );
            
            CreateAIShip(position, AIPersonality.Trader, "Trader", i + 1);
        }
    }
    
    /// <summary>
    /// Creates AI miner ships
    /// </summary>
    private void CreateMinerShips(Vector3 center, float radius, int count)
    {
        Console.WriteLine($"  Creating {count} miner ships...");
        
        for (int i = 0; i < count; i++)
        {
            var position = center + new Vector3(
                (float)(_random.NextDouble() * 2 - 1) * radius,
                (float)(_random.NextDouble() * 2 - 1) * radius * 0.2f,
                (float)(_random.NextDouble() * 2 - 1) * radius
            );
            
            CreateAIShip(position, AIPersonality.Miner, "Miner", i + 1);
        }
    }
    
    /// <summary>
    /// Creates AI pirate ships
    /// </summary>
    private void CreatePirateShips(Vector3 center, float radius, int count)
    {
        Console.WriteLine($"  Creating {count} pirate ships...");
        
        for (int i = 0; i < count; i++)
        {
            var position = center + new Vector3(
                (float)(_random.NextDouble() * 2 - 1) * radius,
                (float)(_random.NextDouble() * 2 - 1) * radius * 0.3f,
                (float)(_random.NextDouble() * 2 - 1) * radius
            );
            
            CreateAIShip(position, AIPersonality.Aggressive, "Pirate", i + 1);
        }
    }
    
    /// <summary>
    /// Creates an AI-controlled ship
    /// </summary>
    private Guid CreateAIShip(Vector3 position, AIPersonality personality, string typePrefix, int number)
    {
        var ship = _gameEngine.EntityManager.CreateEntity($"{typePrefix} Ship {number}");
        
        // Create ship structure based on personality with MORE blocks for better appearance
        var voxelComponent = new VoxelStructureComponent();
        
        switch (personality)
        {
            case AIPersonality.Trader:
                // Cargo-heavy ship with substantial hull
                // Central hull spine (5 blocks)
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(4, 3, 3), "Titanium", BlockType.Hull));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(4, 0, 0), new Vector3(3, 3, 3), "Titanium", BlockType.Hull));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(-4, 0, 0), new Vector3(3, 3, 3), "Titanium", BlockType.Hull));
                
                // Large cargo containers (4 blocks)
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(8, 0, 0), new Vector3(4, 4, 4), "Iron", BlockType.Cargo));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(12, 0, 0), new Vector3(4, 4, 4), "Iron", BlockType.Cargo));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 4, 0), new Vector3(3, 3, 3), "Iron", BlockType.Cargo));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, -4, 0), new Vector3(3, 3, 3), "Iron", BlockType.Cargo));
                
                // Engines (2 blocks)
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(-8, 0, 0), new Vector3(2.5f, 2.5f, 2.5f), "Naonite", BlockType.Engine));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(-8, 0, 3), new Vector3(2.5f, 2.5f, 2.5f), "Naonite", BlockType.Engine));
                
                // Shield generator (1 block)
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 0, 4), new Vector3(2, 2, 2), "Titanium", BlockType.ShieldGenerator));
                
                // Generator (1 block)
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 0, -4), new Vector3(2, 2, 2), "Xanion", BlockType.Generator));
                
                // Armor plating on sides (4 blocks)
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(4, 4, 0), new Vector3(2.5f, 2, 2.5f), "Iron", BlockType.Armor));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(4, -4, 0), new Vector3(2.5f, 2, 2.5f), "Iron", BlockType.Armor));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(-4, 4, 0), new Vector3(2.5f, 2, 2.5f), "Iron", BlockType.Armor));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(-4, -4, 0), new Vector3(2.5f, 2, 2.5f), "Iron", BlockType.Armor));
                break;
                
            case AIPersonality.Miner:
                // Functional mining ship with industrial look
                // Core hull (3 blocks)
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(4, 3, 3), "Titanium", BlockType.Hull));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(4, 0, 0), new Vector3(3, 3, 3), "Titanium", BlockType.Hull));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(-4, 0, 0), new Vector3(3, 3, 3), "Titanium", BlockType.Hull));
                
                // Cargo holds for ore (3 blocks)
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(8, 0, 0), new Vector3(3, 3, 3), "Iron", BlockType.Cargo));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 4, 0), new Vector3(3, 2.5f, 3), "Iron", BlockType.Cargo));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, -4, 0), new Vector3(3, 2.5f, 3), "Iron", BlockType.Cargo));
                
                // Engines (2 blocks)
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(-8, 1, 0), new Vector3(2.5f, 2, 2.5f), "Naonite", BlockType.Engine));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(-8, -1, 0), new Vector3(2.5f, 2, 2.5f), "Naonite", BlockType.Engine));
                
                // Generators for mining lasers (2 blocks)
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(11, 0, 0), new Vector3(2, 2, 2), "Xanion", BlockType.Generator));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 0, 4), new Vector3(2, 2, 2), "Xanion", BlockType.Generator));
                
                // Shield (1 block)
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 0, -4), new Vector3(2, 2, 2), "Naonite", BlockType.ShieldGenerator));
                
                // Thrusters for maneuverability (2 blocks)
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(4, 0, 4), new Vector3(1.5f, 1.5f, 1.5f), "Trinium", BlockType.Thruster));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(4, 0, -4), new Vector3(1.5f, 1.5f, 1.5f), "Trinium", BlockType.Thruster));
                break;
                
            case AIPersonality.Aggressive:
                // Combat-focused pirate ship - sleek and dangerous
                // Central combat hull (3 blocks)
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(4, 2.5f, 2.5f), "Ogonite", BlockType.Hull));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(4, 0, 0), new Vector3(3, 2.5f, 2.5f), "Ogonite", BlockType.Hull));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(-4, 0, 0), new Vector3(3, 2.5f, 2.5f), "Ogonite", BlockType.Hull));
                
                // Heavy armor (6 blocks)
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(7, 0, 0), new Vector3(2.5f, 2, 2), "Ogonite", BlockType.Armor));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 3, 0), new Vector3(3, 2, 2.5f), "Ogonite", BlockType.Armor));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, -3, 0), new Vector3(3, 2, 2.5f), "Ogonite", BlockType.Armor));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(4, 3, 0), new Vector3(2, 1.5f, 2), "Ogonite", BlockType.Armor));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(4, -3, 0), new Vector3(2, 1.5f, 2), "Ogonite", BlockType.Armor));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(-4, 0, 0), new Vector3(2, 2, 3), "Ogonite", BlockType.Armor));
                
                // Twin engines for speed (2 blocks)
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(-7, 1.5f, 0), new Vector3(2.5f, 2, 2.5f), "Trinium", BlockType.Engine));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(-7, -1.5f, 0), new Vector3(2.5f, 2, 2.5f), "Trinium", BlockType.Engine));
                
                // Maneuvering thrusters (4 blocks)
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 5, 1), new Vector3(1.5f, 1.5f, 1.5f), "Trinium", BlockType.Thruster));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, -5, 1), new Vector3(1.5f, 1.5f, 1.5f), "Trinium", BlockType.Thruster));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 5, -1), new Vector3(1.5f, 1.5f, 1.5f), "Trinium", BlockType.Thruster));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, -5, -1), new Vector3(1.5f, 1.5f, 1.5f), "Trinium", BlockType.Thruster));
                
                // Shield and generator for combat endurance (2 blocks)
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 0, 3), new Vector3(2, 2, 2), "Naonite", BlockType.ShieldGenerator));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 0, -3), new Vector3(2, 2, 2), "Xanion", BlockType.Generator));
                
                // Gyro for combat maneuvers (2 blocks)
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(4, 0, 3), new Vector3(1.5f, 1.5f, 1.5f), "Avorion", BlockType.GyroArray));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(4, 0, -3), new Vector3(1.5f, 1.5f, 1.5f), "Avorion", BlockType.GyroArray));
                break;
                
            default:
                // Generic ship with reasonable size
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(3, 3, 3), "Titanium", BlockType.Hull));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(3, 0, 0), new Vector3(3, 3, 3), "Titanium", BlockType.Hull));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(-3, 0, 0), new Vector3(2, 2, 2), "Iron", BlockType.Engine));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(-3, 0, 2), new Vector3(2, 2, 2), "Iron", BlockType.Engine));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 3, 0), new Vector3(2, 2, 2), "Naonite", BlockType.ShieldGenerator));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, -3, 0), new Vector3(2, 2, 2), "Xanion", BlockType.Generator));
                break;
        }
        
        _gameEngine.EntityManager.AddComponent(ship.Id, voxelComponent);
        
        // Add physics
        var physicsComponent = new PhysicsComponent
        {
            Position = position,
            Mass = voxelComponent.TotalMass,
            MaxThrust = voxelComponent.TotalThrust,
            MaxTorque = voxelComponent.TotalTorque,
            Velocity = new Vector3(
                (float)(_random.NextDouble() * 2 - 1) * 10f,
                (float)(_random.NextDouble() * 2 - 1) * 5f,
                (float)(_random.NextDouble() * 2 - 1) * 10f
            )
        };
        _gameEngine.EntityManager.AddComponent(ship.Id, physicsComponent);
        
        // Add inventory
        var inventoryComponent = new InventoryComponent(500);
        inventoryComponent.Inventory.AddResource(ResourceType.Credits, _random.Next(1000, 5000));
        if (personality == AIPersonality.Trader)
        {
            // Traders carry goods
            inventoryComponent.Inventory.AddResource(ResourceType.Iron, _random.Next(50, 200));
            inventoryComponent.Inventory.AddResource(ResourceType.Titanium, _random.Next(20, 100));
        }
        _gameEngine.EntityManager.AddComponent(ship.Id, inventoryComponent);
        
        // Add combat component
        var combatComponent = new CombatComponent
        {
            EntityId = ship.Id,
            MaxShields = voxelComponent.ShieldCapacity,
            CurrentShields = voxelComponent.ShieldCapacity,
            MaxEnergy = voxelComponent.PowerGeneration,
            CurrentEnergy = voxelComponent.PowerGeneration
        };
        _gameEngine.EntityManager.AddComponent(ship.Id, combatComponent);
        
        // Add AI component
        var aiComponent = new AIComponent
        {
            EntityId = ship.Id,
            Personality = personality,
            CurrentState = AIState.Patrol
        };
        _gameEngine.EntityManager.AddComponent(ship.Id, aiComponent);
        
        // Add faction component
        var factionComponent = new FactionComponent
        {
            EntityId = ship.Id,
            FactionName = personality == AIPersonality.Aggressive ? "Pirates" : "Neutral"
        };
        _gameEngine.EntityManager.AddComponent(ship.Id, factionComponent);
        
        // Add hyperdrive
        var hyperdriveComponent = new HyperdriveComponent
        {
            EntityId = ship.Id,
            JumpRange = 5f
        };
        _gameEngine.EntityManager.AddComponent(ship.Id, hyperdriveComponent);
        
        // Add sector location
        var locationComponent = new SectorLocationComponent
        {
            EntityId = ship.Id,
            CurrentSector = new SectorCoordinate(0, 0, 0)
        };
        _gameEngine.EntityManager.AddComponent(ship.Id, locationComponent);
        
        return ship.Id;
    }
    
    /// <summary>
    /// Creates a space station
    /// </summary>
    private Guid CreateStation(Vector3 position, StationType stationType, string name)
    {
        Console.WriteLine($"  Creating {stationType} station: {name}...");
        
        var station = _gameEngine.EntityManager.CreateEntity(name);
        
        // Create MUCH larger, more impressive voxel structure for station
        var voxelComponent = new VoxelStructureComponent();
        
        // MASSIVE central core (stations should dwarf ships)
        voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(15, 15, 15), "Titanium", BlockType.Hull));
        
        // Inner support structure (4 blocks)
        voxelComponent.AddBlock(new VoxelBlock(new Vector3(8, 8, 8), new Vector3(6, 6, 6), "Titanium", BlockType.Hull));
        voxelComponent.AddBlock(new VoxelBlock(new Vector3(-8, 8, 8), new Vector3(6, 6, 6), "Titanium", BlockType.Hull));
        voxelComponent.AddBlock(new VoxelBlock(new Vector3(8, -8, -8), new Vector3(6, 6, 6), "Titanium", BlockType.Hull));
        voxelComponent.AddBlock(new VoxelBlock(new Vector3(-8, -8, -8), new Vector3(6, 6, 6), "Titanium", BlockType.Hull));
        
        // Four large docking bays (one on each cardinal direction)
        voxelComponent.AddBlock(new VoxelBlock(new Vector3(20, 0, 0), new Vector3(8, 12, 12), "Naonite", BlockType.PodDocking));
        voxelComponent.AddBlock(new VoxelBlock(new Vector3(-20, 0, 0), new Vector3(8, 12, 12), "Naonite", BlockType.PodDocking));
        voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 20, 0), new Vector3(12, 8, 12), "Naonite", BlockType.PodDocking));
        voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, -20, 0), new Vector3(12, 8, 12), "Naonite", BlockType.PodDocking));
        
        // Type-specific modules with distinct colors
        switch (stationType)
        {
            case StationType.TradingPost:
                // Large green cargo storage modules (6 blocks)
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 0, 20), new Vector3(10, 10, 8), "Naonite", BlockType.Cargo));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 0, -20), new Vector3(10, 10, 8), "Naonite", BlockType.Cargo));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(12, 12, 0), new Vector3(8, 8, 8), "Naonite", BlockType.Cargo));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(-12, 12, 0), new Vector3(8, 8, 8), "Naonite", BlockType.Cargo));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(12, -12, 0), new Vector3(8, 8, 8), "Naonite", BlockType.Cargo));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(-12, -12, 0), new Vector3(8, 8, 8), "Naonite", BlockType.Cargo));
                
                // Market displays (4 blocks) - bright colors to attract traders
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(15, 0, 15), new Vector3(5, 5, 5), "Xanion", BlockType.Generator));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(-15, 0, 15), new Vector3(5, 5, 5), "Xanion", BlockType.Generator));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(15, 0, -15), new Vector3(5, 5, 5), "Xanion", BlockType.Generator));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(-15, 0, -15), new Vector3(5, 5, 5), "Xanion", BlockType.Generator));
                break;
                
            case StationType.Military:
                // Heavy red armor plating (8 blocks)
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 0, 18), new Vector3(12, 12, 6), "Ogonite", BlockType.Armor));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 0, -18), new Vector3(12, 12, 6), "Ogonite", BlockType.Armor));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(18, 0, 0), new Vector3(6, 12, 12), "Ogonite", BlockType.Armor));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(-18, 0, 0), new Vector3(6, 12, 12), "Ogonite", BlockType.Armor));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(12, 12, 12), new Vector3(6, 6, 6), "Ogonite", BlockType.Armor));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(-12, 12, 12), new Vector3(6, 6, 6), "Ogonite", BlockType.Armor));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(12, -12, -12), new Vector3(6, 6, 6), "Ogonite", BlockType.Armor));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(-12, -12, -12), new Vector3(6, 6, 6), "Ogonite", BlockType.Armor));
                
                // Weapon platforms (4 blocks)
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 15, 15), new Vector3(5, 5, 5), "Trinium", BlockType.TurretMount));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 15, -15), new Vector3(5, 5, 5), "Trinium", BlockType.TurretMount));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, -15, 15), new Vector3(5, 5, 5), "Trinium", BlockType.TurretMount));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, -15, -15), new Vector3(5, 5, 5), "Trinium", BlockType.TurretMount));
                break;
                
            case StationType.Research:
                // Purple research modules (6 blocks)
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 0, 20), new Vector3(8, 8, 10), "Avorion", BlockType.Generator));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 0, -20), new Vector3(8, 8, 10), "Avorion", BlockType.Generator));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 18, 0), new Vector3(8, 10, 8), "Avorion", BlockType.Generator));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, -18, 0), new Vector3(8, 10, 8), "Avorion", BlockType.Generator));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(15, 0, 0), new Vector3(8, 8, 8), "Avorion", BlockType.Generator));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(-15, 0, 0), new Vector3(8, 8, 8), "Avorion", BlockType.Generator));
                
                // Sensor arrays (4 blocks)
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(10, 10, 10), new Vector3(4, 4, 4), "Trinium", BlockType.GyroArray));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(-10, 10, 10), new Vector3(4, 4, 4), "Trinium", BlockType.GyroArray));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(10, -10, -10), new Vector3(4, 4, 4), "Trinium", BlockType.GyroArray));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(-10, -10, -10), new Vector3(4, 4, 4), "Trinium", BlockType.GyroArray));
                break;
                
            default:
                // Generic station gets mixed modules
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 0, 18), new Vector3(10, 10, 8), "Naonite", BlockType.Cargo));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 0, -18), new Vector3(10, 10, 8), "Naonite", BlockType.Cargo));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 15, 0), new Vector3(8, 8, 8), "Xanion", BlockType.Generator));
                voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, -15, 0), new Vector3(8, 8, 8), "Xanion", BlockType.Generator));
                break;
        }
        
        // Massive shield generators for all stations (4 blocks) - Gold/Yellow glow
        voxelComponent.AddBlock(new VoxelBlock(new Vector3(25, 0, 0), new Vector3(6, 6, 6), "Xanion", BlockType.ShieldGenerator));
        voxelComponent.AddBlock(new VoxelBlock(new Vector3(-25, 0, 0), new Vector3(6, 6, 6), "Xanion", BlockType.ShieldGenerator));
        voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 0, 25), new Vector3(6, 6, 6), "Xanion", BlockType.ShieldGenerator));
        voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 0, -25), new Vector3(6, 6, 6), "Xanion", BlockType.ShieldGenerator));
        
        // Power generators (4 blocks) - More gold for visibility
        voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 15, 0), new Vector3(5, 5, 5), "Xanion", BlockType.Generator));
        voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, -15, 0), new Vector3(5, 5, 5), "Xanion", BlockType.Generator));
        voxelComponent.AddBlock(new VoxelBlock(new Vector3(15, 0, 0), new Vector3(5, 5, 5), "Xanion", BlockType.Generator));
        voxelComponent.AddBlock(new VoxelBlock(new Vector3(-15, 0, 0), new Vector3(5, 5, 5), "Xanion", BlockType.Generator));
        
        // Communication antennas (4 blocks) - Blue thrusters for visual interest
        voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 22, 0), new Vector3(3, 6, 3), "Trinium", BlockType.Thruster));
        voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, -22, 0), new Vector3(3, 6, 3), "Trinium", BlockType.Thruster));
        voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 0, 22), new Vector3(3, 6, 3), "Trinium", BlockType.Thruster));
        voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 0, -22), new Vector3(3, 6, 3), "Trinium", BlockType.Thruster));
        
        _gameEngine.EntityManager.AddComponent(station.Id, voxelComponent);
        
        // Add physics (stationary)
        var physicsComponent = new PhysicsComponent
        {
            Position = position,
            Mass = voxelComponent.TotalMass * 10, // Stations are HEAVY
            Velocity = Vector3.Zero // Stationary
        };
        _gameEngine.EntityManager.AddComponent(station.Id, physicsComponent);
        
        // Add large inventory for trading
        var inventoryComponent = new InventoryComponent(10000);
        inventoryComponent.Inventory.AddResource(ResourceType.Credits, 1000000);
        inventoryComponent.Inventory.AddResource(ResourceType.Iron, 5000);
        inventoryComponent.Inventory.AddResource(ResourceType.Titanium, 2000);
        inventoryComponent.Inventory.AddResource(ResourceType.Naonite, 1000);
        _gameEngine.EntityManager.AddComponent(station.Id, inventoryComponent);
        
        // Add combat component (stations can defend themselves)
        var combatComponent = new CombatComponent
        {
            EntityId = station.Id,
            MaxShields = voxelComponent.ShieldCapacity * 5, // Strong shields
            CurrentShields = voxelComponent.ShieldCapacity * 5,
            MaxEnergy = voxelComponent.PowerGeneration * 3,
            CurrentEnergy = voxelComponent.PowerGeneration * 3
        };
        _gameEngine.EntityManager.AddComponent(station.Id, combatComponent);
        
        // Add docking component
        var dockingComponent = new DockingComponent
        {
            EntityId = station.Id,
            HasDockingPort = true
        };
        _gameEngine.EntityManager.AddComponent(station.Id, dockingComponent);
        
        return station.Id;
    }
}

/// <summary>
/// Station types
/// </summary>
public enum StationType
{
    TradingPost,
    Military,
    Research,
    Mining,
    Shipyard
}
