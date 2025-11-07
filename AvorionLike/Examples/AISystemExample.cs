using System.Numerics;
using AvorionLike.Core;
using AvorionLike.Core.AI;
using AvorionLike.Core.Combat;
using AvorionLike.Core.Mining;
using AvorionLike.Core.Physics;
using AvorionLike.Core.Resources;
using AvorionLike.Core.Voxel;

namespace AvorionLike.Examples;

/// <summary>
/// Example demonstrating how to use the AI system
/// </summary>
public static class AISystemExample
{
    /// <summary>
    /// Create a mining AI ship
    /// </summary>
    public static Guid CreateMiningAIShip(GameEngine engine, Vector3 position)
    {
        var entity = engine.EntityManager.CreateEntity("Mining AI Ship");
        
        // Add voxel structure
        var structure = new VoxelStructureComponent();
        structure.AddBlock(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(2, 2, 2), "Iron"));
        structure.AddBlock(new VoxelBlock(new Vector3(2, 0, 0), new Vector3(1, 1, 1), "Iron", BlockType.Engine));
        engine.EntityManager.AddComponent(entity.Id, structure);
        
        // Add physics
        var physics = new PhysicsComponent
        {
            Position = position,
            Mass = structure.TotalMass,
            MaxThrust = 500f
        };
        engine.EntityManager.AddComponent(entity.Id, physics);
        
        // Add mining capability
        var mining = new MiningComponent
        {
            EntityId = entity.Id,
            MiningPower = 15f,
            MiningRange = 100f
        };
        engine.EntityManager.AddComponent(entity.Id, mining);
        
        // Add inventory
        var inventory = new InventoryComponent
        {
            EntityId = entity.Id,
            Inventory = new Inventory { MaxCapacity = 500 }
        };
        engine.EntityManager.AddComponent(entity.Id, inventory);
        
        // Add AI with miner personality
        var ai = new AIComponent
        {
            EntityId = entity.Id,
            Personality = AIPersonality.Miner,
            CurrentState = AIState.Idle,
            CanMine = true,
            HomeBase = new Vector3(0, 0, 0),
            CargoReturnThreshold = 0.9f
        };
        engine.EntityManager.AddComponent(entity.Id, ai);
        
        return entity.Id;
    }
    
    /// <summary>
    /// Create a combat AI ship
    /// </summary>
    public static Guid CreateCombatAIShip(GameEngine engine, Vector3 position, AIPersonality personality = AIPersonality.Aggressive)
    {
        var entity = engine.EntityManager.CreateEntity("Combat AI Ship");
        
        // Add voxel structure
        var structure = new VoxelStructureComponent();
        structure.AddBlock(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(3, 2, 3), "Titanium", BlockType.Armor));
        structure.AddBlock(new VoxelBlock(new Vector3(3, 0, 0), new Vector3(1, 1, 1), "Titanium", BlockType.Engine));
        engine.EntityManager.AddComponent(entity.Id, structure);
        
        // Add physics
        var physics = new PhysicsComponent
        {
            Position = position,
            Mass = structure.TotalMass,
            MaxThrust = 800f,
            MaxTorque = 100f
        };
        engine.EntityManager.AddComponent(entity.Id, physics);
        
        // Add combat capabilities
        var combat = new CombatComponent
        {
            EntityId = entity.Id,
            MaxShields = 500f,
            CurrentShields = 500f,
            MaxEnergy = 200f,
            CurrentEnergy = 200f
        };
        
        // Add turrets
        combat.AddTurret(new Turret
        {
            Name = "Laser Turret",
            Type = WeaponType.Laser,
            Damage = 25f,
            FireRate = 2f,
            Range = 1200f,
            ProjectileSpeed = 800f,
            IsAutoTargeting = true
        });
        
        combat.AddTurret(new Turret
        {
            Name = "Chaingun",
            Type = WeaponType.Chaingun,
            Damage = 15f,
            FireRate = 5f,
            Range = 800f,
            ProjectileSpeed = 600f,
            IsAutoTargeting = true
        });
        
        engine.EntityManager.AddComponent(entity.Id, combat);
        
        // Add AI with combat personality
        var ai = new AIComponent
        {
            EntityId = entity.Id,
            Personality = personality,
            CurrentState = AIState.Idle,
            CombatTactic = personality == AIPersonality.Aggressive 
                ? CombatTactic.Aggressive 
                : CombatTactic.Defensive,
            MinCombatDistance = 400f,
            MaxCombatDistance = 1000f,
            FleeThreshold = 0.2f,
            ReturnToCombatThreshold = 0.6f
        };
        engine.EntityManager.AddComponent(entity.Id, ai);
        
        return entity.Id;
    }
    
    /// <summary>
    /// Create a patrol AI ship
    /// </summary>
    public static Guid CreatePatrolAIShip(GameEngine engine, Vector3 position, List<Vector3> waypoints)
    {
        var entity = engine.EntityManager.CreateEntity("Patrol AI Ship");
        
        // Add voxel structure
        var structure = new VoxelStructureComponent();
        structure.AddBlock(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(2, 1, 2), "Iron"));
        structure.AddBlock(new VoxelBlock(new Vector3(2, 0, 0), new Vector3(1, 1, 1), "Iron", BlockType.Engine));
        engine.EntityManager.AddComponent(entity.Id, structure);
        
        // Add physics
        var physics = new PhysicsComponent
        {
            Position = position,
            Mass = structure.TotalMass,
            MaxThrust = 400f
        };
        engine.EntityManager.AddComponent(entity.Id, physics);
        
        // Add basic combat for defense
        var combat = new CombatComponent
        {
            EntityId = entity.Id,
            MaxShields = 200f,
            CurrentShields = 200f,
            MaxEnergy = 100f,
            CurrentEnergy = 100f
        };
        
        combat.AddTurret(new Turret
        {
            Name = "Defense Turret",
            Type = WeaponType.Chaingun,
            Damage = 10f,
            FireRate = 3f,
            Range = 600f,
            IsAutoTargeting = true
        });
        
        engine.EntityManager.AddComponent(entity.Id, combat);
        
        // Add AI with balanced personality and patrol waypoints
        var ai = new AIComponent
        {
            EntityId = entity.Id,
            Personality = AIPersonality.Balanced,
            CurrentState = AIState.Patrol,
            PatrolWaypoints = waypoints,
            CurrentPatrolIndex = 0,
            CombatTactic = CombatTactic.Defensive
        };
        engine.EntityManager.AddComponent(entity.Id, ai);
        
        return entity.Id;
    }
    
    /// <summary>
    /// Run a complete AI demonstration
    /// </summary>
    public static void RunAIDemo()
    {
        Console.WriteLine("=== AI System Demonstration ===\n");
        
        // Create game engine
        var engine = new GameEngine(12345);
        engine.Start();
        
        Console.WriteLine("1. Creating Mining AI Ship...");
        var minerShip = CreateMiningAIShip(engine, new Vector3(100, 0, 0));
        Console.WriteLine($"   Created mining ship: {minerShip}");
        
        // Add some asteroids for the miner
        engine.MiningSystem.AddAsteroid(new Asteroid(new AvorionLike.Core.Procedural.AsteroidData
        {
            Position = new Vector3(150, 0, 0),
            Size = 10f,
            ResourceType = "Iron"
        }));
        Console.WriteLine("   Added asteroid at (150, 0, 0)");
        
        Console.WriteLine("\n2. Creating Combat AI Ships...");
        var aggressiveShip = CreateCombatAIShip(engine, new Vector3(0, 100, 0), AIPersonality.Aggressive);
        var defensiveShip = CreateCombatAIShip(engine, new Vector3(0, -100, 0), AIPersonality.Defensive);
        Console.WriteLine($"   Created aggressive ship: {aggressiveShip}");
        Console.WriteLine($"   Created defensive ship: {defensiveShip}");
        
        Console.WriteLine("\n3. Creating Patrol AI Ship...");
        var patrolWaypoints = new List<Vector3>
        {
            new Vector3(200, 0, 0),
            new Vector3(200, 200, 0),
            new Vector3(0, 200, 0),
            new Vector3(0, 0, 0)
        };
        var patrolShip = CreatePatrolAIShip(engine, new Vector3(0, 0, 0), patrolWaypoints);
        Console.WriteLine($"   Created patrol ship: {patrolShip}");
        Console.WriteLine($"   Patrol waypoints: {patrolWaypoints.Count}");
        
        Console.WriteLine("\n4. Simulating AI behavior...");
        for (int i = 0; i < 10; i++)
        {
            engine.Update();
            System.Threading.Thread.Sleep(100);
            
            if (i % 3 == 0)
            {
                Console.WriteLine($"   Update {i}: AI systems running...");
                
                // Check miner state
                var minerAI = engine.EntityManager.GetComponent<AIComponent>(minerShip);
                if (minerAI != null)
                {
                    Console.WriteLine($"     Miner state: {minerAI.CurrentState}");
                }
                
                // Check patrol state
                var patrolAI = engine.EntityManager.GetComponent<AIComponent>(patrolShip);
                if (patrolAI != null)
                {
                    Console.WriteLine($"     Patrol state: {patrolAI.CurrentState}, Waypoint: {patrolAI.CurrentPatrolIndex}");
                }
            }
        }
        
        Console.WriteLine("\n=== AI Demonstration Complete ===");
        Console.WriteLine("\nAI Features Demonstrated:");
        Console.WriteLine("- State-based AI behavior (Idle, Mining, Patrol, Combat)");
        Console.WriteLine("- Personality-driven decision making (Miner, Aggressive, Defensive, Balanced)");
        Console.WriteLine("- Perception system for environmental awareness");
        Console.WriteLine("- Movement and navigation behaviors");
        Console.WriteLine("- Integration with existing game systems (Combat, Mining, Physics)");
        
        engine.Stop();
    }
}
