using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Physics;
using AvorionLike.Core.Combat;
using AvorionLike.Core.Resources;
using AvorionLike.Core.RPG;
using AvorionLike.Core.Navigation;
using AvorionLike.Core.AI;
using AvorionLike.Core.Procedural;
using AvorionLike.Core.Mining;

namespace AvorionLike.Core.DevTools;

/// <summary>
/// Enhanced in-game testing console for rapid iteration and testing
/// Provides commands to spawn entities, modify systems, and test gameplay features
/// </summary>
public class InGameTestingConsole : DebugConsole
{
    private readonly GameEngine _gameEngine;
    private Guid _playerShipId = Guid.Empty;

    public InGameTestingConsole(GameEngine gameEngine) : base(gameEngine.ScriptingEngine)
    {
        _gameEngine = gameEngine;
        RegisterTestingCommands();
    }

    /// <summary>
    /// Set the player ship ID for player-relative commands
    /// </summary>
    public void SetPlayerShip(Guid shipId)
    {
        _playerShipId = shipId;
    }

    private void RegisterTestingCommands()
    {
        // === ENTITY SPAWNING COMMANDS ===
        
        RegisterCommand("spawn_ship", "Spawn a test ship (args: [material] [x] [y] [z])", args =>
        {
            string material = args.Length > 0 ? args[0] : "Titanium";
            Vector3 position = ParsePosition(args, 1) ?? GetPlayerPosition() + new Vector3(20, 0, 0);
            
            var ship = CreateTestShip($"Spawned Ship", material, position);
            WriteLine($"Spawned ship '{ship.Name}' at {position}");
        });

        RegisterCommand("spawn_fighter", "Spawn a fighter ship near player", args =>
        {
            Vector3 position = GetPlayerPosition() + new Vector3(30, 10, 0);
            var ship = CreateFighterShip(position);
            WriteLine($"Spawned fighter ship at {position}");
        });

        RegisterCommand("spawn_cargo", "Spawn a cargo ship near player", args =>
        {
            Vector3 position = GetPlayerPosition() + new Vector3(-30, 0, 10);
            var ship = CreateCargoShip(position);
            WriteLine($"Spawned cargo ship at {position}");
        });

        RegisterCommand("spawn_enemy", "Spawn an AI enemy ship (args: [aggressive/defensive/miner])", args =>
        {
            string personality = args.Length > 0 ? args[0] : "aggressive";
            Vector3 position = GetPlayerPosition() + new Vector3(50, 0, 0);
            
            var enemy = CreateEnemyShip(position, personality);
            WriteLine($"Spawned {personality} enemy at {position}");
        });

        RegisterCommand("spawn_asteroid", "Spawn an asteroid (args: [resourceType] [x] [y] [z])", args =>
        {
            ResourceType resource = args.Length > 0 ? ParseResourceType(args[0]) : ResourceType.Iron;
            Vector3 position = ParsePosition(args, 1) ?? GetPlayerPosition() + new Vector3(40, 20, 0);
            
            var asteroid = CreateAsteroid(position, resource);
            WriteLine($"Spawned {resource} asteroid at {position}");
        });

        RegisterCommand("spawn_station", "Spawn a station (args: [trading/mining/military])", args =>
        {
            string stationType = args.Length > 0 ? args[0] : "trading";
            Vector3 position = GetPlayerPosition() + new Vector3(0, 0, 100);
            
            var station = CreateStation(position, stationType);
            WriteLine($"Spawned {stationType} station at {position}");
        });

        RegisterCommand("clear_entities", "Remove all non-player entities", args =>
        {
            int count = 0;
            foreach (var entity in _gameEngine.EntityManager.GetAllEntities().ToList())
            {
                if (entity.Id != _playerShipId)
                {
                    _gameEngine.EntityManager.DestroyEntity(entity.Id);
                    count++;
                }
            }
            WriteLine($"Cleared {count} entities");
        });

        RegisterCommand("populate_sector", "Populate current sector with entities (args: [count])", args =>
        {
            int count = args.Length > 0 ? int.Parse(args[0]) : 10;
            Vector3 playerPos = GetPlayerPosition();
            
            for (int i = 0; i < count; i++)
            {
                float angle = (i / (float)count) * MathF.PI * 2;
                float radius = 100 + Random.Shared.Next(100);
                Vector3 offset = new Vector3(
                    MathF.Cos(angle) * radius,
                    (Random.Shared.NextSingle() - 0.5f) * 50,
                    MathF.Sin(angle) * radius
                );
                
                if (i % 3 == 0)
                    CreateAsteroid(playerPos + offset, RandomResourceType());
                else if (i % 3 == 1)
                    CreateTestShip($"NPC Ship {i}", "Iron", playerPos + offset);
                else
                    CreateEnemyShip(playerPos + offset, "defensive");
            }
            
            WriteLine($"Populated sector with {count} entities");
        });

        // === COMBAT TESTING COMMANDS ===
        
        RegisterCommand("damage", "Damage player ship (args: amount)", args =>
        {
            if (!TryGetPlayerShip(out var playerShip)) return;
            
            float amount = args.Length > 0 ? float.Parse(args[0]) : 100f;
            
            if (_gameEngine.EntityManager.HasComponent<CombatComponent>(playerShip.Id))
            {
                var combat = _gameEngine.EntityManager.GetComponent<CombatComponent>(playerShip.Id);
                if (combat != null)
                {
                    combat.CurrentShields = Math.Max(0, combat.CurrentShields - amount);
                    WriteLine($"Dealt {amount} damage. Shields: {combat.CurrentShields:F0}/{combat.MaxShields:F0}");
                }
            }
        });

        RegisterCommand("heal", "Restore player ship shields", args =>
        {
            if (!TryGetPlayerShip(out var playerShip)) return;
            
            if (_gameEngine.EntityManager.HasComponent<CombatComponent>(playerShip.Id))
            {
                var combat = _gameEngine.EntityManager.GetComponent<CombatComponent>(playerShip.Id);
                if (combat != null)
                {
                    combat.CurrentShields = combat.MaxShields;
                    combat.CurrentEnergy = combat.MaxEnergy;
                    WriteLine($"Fully healed. Shields: {combat.CurrentShields:F0}, Energy: {combat.CurrentEnergy:F0}");
                }
            }
        });

        RegisterCommand("godmode", "Toggle invincibility", args =>
        {
            // This would need a flag in CombatComponent - for now, just heal continuously
            WriteLine("Godmode not fully implemented. Use 'heal' command for testing.");
        });

        // === RESOURCE/ECONOMY COMMANDS ===
        
        RegisterCommand("add_resource", "Add resources (args: resourceType amount)", args =>
        {
            if (!TryGetPlayerShip(out var playerShip)) return;
            
            if (args.Length < 2)
            {
                WriteLine("Usage: add_resource <type> <amount>");
                return;
            }
            
            ResourceType type = ParseResourceType(args[0]);
            int amount = int.Parse(args[1]);
            
            if (_gameEngine.EntityManager.HasComponent<InventoryComponent>(playerShip.Id))
            {
                var inv = _gameEngine.EntityManager.GetComponent<InventoryComponent>(playerShip.Id);
                if (inv != null)
                {
                    inv.Inventory.AddResource(type, amount);
                    WriteLine($"Added {amount} {type}. New total: {inv.Inventory.GetResourceAmount(type)}");
                }
            }
        });

        RegisterCommand("credits", "Add credits (args: amount)", args =>
        {
            if (!TryGetPlayerShip(out var playerShip)) return;
            
            int amount = args.Length > 0 ? int.Parse(args[0]) : 10000;
            
            if (_gameEngine.EntityManager.HasComponent<InventoryComponent>(playerShip.Id))
            {
                var inv = _gameEngine.EntityManager.GetComponent<InventoryComponent>(playerShip.Id);
                if (inv != null)
                {
                    inv.Inventory.AddResource(ResourceType.Credits, amount);
                    WriteLine($"Added {amount} credits. New total: {inv.Inventory.GetResourceAmount(ResourceType.Credits)}");
                }
            }
        });

        RegisterCommand("clear_inventory", "Clear all resources", args =>
        {
            if (!TryGetPlayerShip(out var playerShip)) return;
            
            if (_gameEngine.EntityManager.HasComponent<InventoryComponent>(playerShip.Id))
            {
                var inv = _gameEngine.EntityManager.GetComponent<InventoryComponent>(playerShip.Id);
                if (inv != null)
                {
                    foreach (var type in Enum.GetValues<ResourceType>())
                    {
                        inv.Inventory.RemoveResource(type, inv.Inventory.GetResourceAmount(type));
                    }
                    WriteLine("Inventory cleared");
                }
            }
        });

        // === PHYSICS/MOVEMENT COMMANDS ===
        
        RegisterCommand("tp", "Teleport player (args: x y z)", args =>
        {
            if (!TryGetPlayerShip(out var playerShip)) return;
            
            if (args.Length < 3)
            {
                WriteLine("Usage: tp <x> <y> <z>");
                return;
            }
            
            Vector3 position = new Vector3(float.Parse(args[0]), float.Parse(args[1]), float.Parse(args[2]));
            
            if (_gameEngine.EntityManager.HasComponent<PhysicsComponent>(playerShip.Id))
            {
                var physics = _gameEngine.EntityManager.GetComponent<PhysicsComponent>(playerShip.Id);
                if (physics != null)
                {
                    physics.Position = position;
                    physics.Velocity = Vector3.Zero;
                    WriteLine($"Teleported to {position}");
                }
            }
        });

        RegisterCommand("velocity", "Set player velocity (args: x y z)", args =>
        {
            if (!TryGetPlayerShip(out var playerShip)) return;
            
            if (args.Length < 3)
            {
                WriteLine("Usage: velocity <x> <y> <z>");
                return;
            }
            
            Vector3 velocity = new Vector3(float.Parse(args[0]), float.Parse(args[1]), float.Parse(args[2]));
            
            if (_gameEngine.EntityManager.HasComponent<PhysicsComponent>(playerShip.Id))
            {
                var physics = _gameEngine.EntityManager.GetComponent<PhysicsComponent>(playerShip.Id);
                if (physics != null)
                {
                    physics.Velocity = velocity;
                    WriteLine($"Velocity set to {velocity}");
                }
            }
        });

        RegisterCommand("stop", "Stop all movement", args =>
        {
            if (!TryGetPlayerShip(out var playerShip)) return;
            
            if (_gameEngine.EntityManager.HasComponent<PhysicsComponent>(playerShip.Id))
            {
                var physics = _gameEngine.EntityManager.GetComponent<PhysicsComponent>(playerShip.Id);
                if (physics != null)
                {
                    physics.Velocity = Vector3.Zero;
                    physics.AngularVelocity = Vector3.Zero;
                    WriteLine("All movement stopped");
                }
            }
        });

        // === AI TESTING COMMANDS ===
        
        RegisterCommand("ai_state", "Set AI state for nearest entity (args: idle/combat/mining/fleeing)", args =>
        {
            if (args.Length < 1)
            {
                WriteLine("Usage: ai_state <state>");
                return;
            }
            
            var nearestAI = GetNearestAIEntity();
            if (nearestAI == null)
            {
                WriteLine("No AI entities nearby");
                return;
            }
            
            var aiComp = _gameEngine.EntityManager.GetComponent<AIComponent>(nearestAI.Id);
            if (aiComp != null)
            {
                aiComp.CurrentState = ParseAIState(args[0]);
                WriteLine($"Set AI state to {aiComp.CurrentState}");
            }
        });

        RegisterCommand("ai_attack_player", "Make nearest AI attack player", args =>
        {
            var nearestAI = GetNearestAIEntity();
            if (nearestAI == null)
            {
                WriteLine("No AI entities nearby");
                return;
            }
            
            var aiComp = _gameEngine.EntityManager.GetComponent<AIComponent>(nearestAI.Id);
            if (aiComp != null)
            {
                aiComp.CurrentState = AIState.Combat;
                aiComp.CurrentTarget = _playerShipId;
                WriteLine($"AI entity {nearestAI.Name} now targeting player");
            }
        });

        // === WORLD/SAVE COMMANDS ===
        
        RegisterCommand("quicksave", "Quick save game", args =>
        {
            if (_gameEngine.QuickSave())
                WriteLine("Game saved");
            else
                WriteLine("Save failed");
        });

        RegisterCommand("quickload", "Quick load game", args =>
        {
            if (_gameEngine.QuickLoad())
                WriteLine("Game loaded");
            else
                WriteLine("Load failed");
        });

        // === INFO COMMANDS ===
        
        RegisterCommand("pos", "Show player position", args =>
        {
            Vector3 pos = GetPlayerPosition();
            WriteLine($"Position: ({pos.X:F1}, {pos.Y:F1}, {pos.Z:F1})");
        });

        RegisterCommand("stats", "Show player ship stats", args =>
        {
            if (!TryGetPlayerShip(out var playerShip)) return;
            
            WriteLine($"=== {playerShip.Name} Stats ===");
            
            if (_gameEngine.EntityManager.HasComponent<PhysicsComponent>(playerShip.Id))
            {
                var physics = _gameEngine.EntityManager.GetComponent<PhysicsComponent>(playerShip.Id);
                if (physics != null)
                {
                    WriteLine($"Position: {physics.Position}");
                    WriteLine($"Velocity: {physics.Velocity} ({physics.Velocity.Length():F1} m/s)");
                    WriteLine($"Mass: {physics.Mass:F0} kg");
                }
            }
            
            if (_gameEngine.EntityManager.HasComponent<CombatComponent>(playerShip.Id))
            {
                var combat = _gameEngine.EntityManager.GetComponent<CombatComponent>(playerShip.Id);
                if (combat != null)
                {
                    WriteLine($"Shields: {combat.CurrentShields:F0}/{combat.MaxShields:F0}");
                    WriteLine($"Energy: {combat.CurrentEnergy:F0}/{combat.MaxEnergy:F0}");
                }
            }
            
            if (_gameEngine.EntityManager.HasComponent<VoxelStructureComponent>(playerShip.Id))
            {
                var voxel = _gameEngine.EntityManager.GetComponent<VoxelStructureComponent>(playerShip.Id);
                if (voxel != null)
                {
                    WriteLine($"Blocks: {voxel.Blocks.Count}");
                    WriteLine($"Integrity: {voxel.StructuralIntegrity:F1}%");
                }
            }
        });

        RegisterCommand("list_entities", "List all entities", args =>
        {
            var entities = _gameEngine.EntityManager.GetAllEntities().ToList();
            WriteLine($"=== Entities ({entities.Count}) ===");
            
            foreach (var entity in entities.Take(20)) // Limit to first 20
            {
                var isPlayer = entity.Id == _playerShipId ? " [PLAYER]" : "";
                var hasAI = _gameEngine.EntityManager.HasComponent<AIComponent>(entity.Id) ? " [AI]" : "";
                WriteLine($"  {entity.Name}{isPlayer}{hasAI}");
            }
            
            if (entities.Count > 20)
                WriteLine($"  ... and {entities.Count - 20} more");
        });
    }

    // === Helper Methods ===

    private bool TryGetPlayerShip(out Entity playerShip)
    {
        if (_playerShipId == Guid.Empty)
        {
            WriteLine("Error: No player ship set");
            playerShip = null!;
            return false;
        }
        
        var entity = _gameEngine.EntityManager.GetEntity(_playerShipId);
        if (entity == null)
        {
            WriteLine("Error: Player ship not found");
            playerShip = null!;
            return false;
        }
        
        playerShip = entity;
        return true;
    }

    private Vector3 GetPlayerPosition()
    {
        if (_playerShipId != Guid.Empty && _gameEngine.EntityManager.HasComponent<PhysicsComponent>(_playerShipId))
        {
            var physics = _gameEngine.EntityManager.GetComponent<PhysicsComponent>(_playerShipId);
            if (physics != null)
                return physics.Position;
        }
        return Vector3.Zero;
    }

    private Vector3? ParsePosition(string[] args, int startIndex)
    {
        if (args.Length < startIndex + 3)
            return null;
        
        try
        {
            return new Vector3(
                float.Parse(args[startIndex]),
                float.Parse(args[startIndex + 1]),
                float.Parse(args[startIndex + 2])
            );
        }
        catch
        {
            return null;
        }
    }

    private ResourceType ParseResourceType(string str)
    {
        return str.ToLower() switch
        {
            "iron" => ResourceType.Iron,
            "titanium" => ResourceType.Titanium,
            "naonite" => ResourceType.Naonite,
            "trinium" => ResourceType.Trinium,
            "xanion" => ResourceType.Xanion,
            "ogonite" => ResourceType.Ogonite,
            "avorion" => ResourceType.Avorion,
            "credits" => ResourceType.Credits,
            _ => ResourceType.Iron
        };
    }

    private ResourceType RandomResourceType()
    {
        var types = new[] { ResourceType.Iron, ResourceType.Titanium, ResourceType.Naonite, ResourceType.Trinium };
        return types[Random.Shared.Next(types.Length)];
    }

    private AIState ParseAIState(string str)
    {
        return str.ToLower() switch
        {
            "idle" => AIState.Idle,
            "combat" => AIState.Combat,
            "mining" => AIState.Mining,
            "fleeing" => AIState.Fleeing,
            "patrol" => AIState.Patrol,
            "trading" => AIState.Trading,
            _ => AIState.Idle
        };
    }

    private Entity? GetNearestAIEntity()
    {
        Vector3 playerPos = GetPlayerPosition();
        Entity? nearest = null;
        float nearestDist = float.MaxValue;
        
        foreach (var entity in _gameEngine.EntityManager.GetAllEntities())
        {
            if (!_gameEngine.EntityManager.HasComponent<AIComponent>(entity.Id))
                continue;
            
            if (_gameEngine.EntityManager.HasComponent<PhysicsComponent>(entity.Id))
            {
                var physics = _gameEngine.EntityManager.GetComponent<PhysicsComponent>(entity.Id);
                if (physics != null)
                {
                    float dist = Vector3.Distance(playerPos, physics.Position);
                    if (dist < nearestDist)
                    {
                        nearestDist = dist;
                        nearest = entity;
                    }
                }
            }
        }
        
        return nearest;
    }

    // === Entity Creation Methods ===

    private Entity CreateTestShip(string name, string material, Vector3 position)
    {
        var ship = _gameEngine.EntityManager.CreateEntity(name);
        
        var voxelComp = new VoxelStructureComponent();
        voxelComp.AddBlock(new VoxelBlock(Vector3.Zero, new Vector3(3, 3, 3), material, BlockType.Hull));
        voxelComp.AddBlock(new VoxelBlock(new Vector3(-4, 0, 0), new Vector3(2, 2, 2), material, BlockType.Engine));
        _gameEngine.EntityManager.AddComponent(ship.Id, voxelComp);
        
        var physicsComp = new PhysicsComponent
        {
            Position = position,
            Mass = voxelComp.TotalMass,
            MaxThrust = voxelComp.TotalThrust
        };
        _gameEngine.EntityManager.AddComponent(ship.Id, physicsComp);
        
        var inventoryComp = new InventoryComponent(500);
        _gameEngine.EntityManager.AddComponent(ship.Id, inventoryComp);
        
        var combatComp = new CombatComponent
        {
            EntityId = ship.Id,
            MaxShields = voxelComp.ShieldCapacity,
            CurrentShields = voxelComp.ShieldCapacity
        };
        _gameEngine.EntityManager.AddComponent(ship.Id, combatComp);
        
        return ship;
    }

    private Entity CreateFighterShip(Vector3 position)
    {
        var ship = _gameEngine.EntityManager.CreateEntity("Fighter");
        
        var voxelComp = new VoxelStructureComponent();
        voxelComp.AddBlock(new VoxelBlock(Vector3.Zero, new Vector3(3, 2, 2), "Titanium", BlockType.Hull));
        voxelComp.AddBlock(new VoxelBlock(new Vector3(-4, 0, 0), new Vector3(2, 2, 2), "Ogonite", BlockType.Engine));
        voxelComp.AddBlock(new VoxelBlock(new Vector3(4, 0, 0), new Vector3(1.5f, 1.5f, 1.5f), "Trinium", BlockType.Thruster));
        _gameEngine.EntityManager.AddComponent(ship.Id, voxelComp);
        
        var physicsComp = new PhysicsComponent
        {
            Position = position,
            Mass = voxelComp.TotalMass,
            MaxThrust = voxelComp.TotalThrust,
            MaxTorque = voxelComp.TotalTorque
        };
        _gameEngine.EntityManager.AddComponent(ship.Id, physicsComp);
        
        return ship;
    }

    private Entity CreateCargoShip(Vector3 position)
    {
        var ship = _gameEngine.EntityManager.CreateEntity("Cargo Ship");
        
        var voxelComp = new VoxelStructureComponent();
        for (int i = 0; i < 3; i++)
        {
            voxelComp.AddBlock(new VoxelBlock(
                new Vector3(i * 4, 0, 0),
                new Vector3(3, 3, 3),
                "Iron",
                i == 1 ? BlockType.Cargo : BlockType.Hull
            ));
        }
        _gameEngine.EntityManager.AddComponent(ship.Id, voxelComp);
        
        var physicsComp = new PhysicsComponent
        {
            Position = position,
            Mass = voxelComp.TotalMass,
            MaxThrust = voxelComp.TotalThrust
        };
        _gameEngine.EntityManager.AddComponent(ship.Id, physicsComp);
        
        var inventoryComp = new InventoryComponent(2000);
        _gameEngine.EntityManager.AddComponent(ship.Id, inventoryComp);
        
        return ship;
    }

    private Entity CreateEnemyShip(Vector3 position, string personality)
    {
        var ship = CreateTestShip("Enemy Ship", "Ogonite", position);
        
        var aiComp = new AIComponent
        {
            EntityId = ship.Id,
            CurrentState = AIState.Patrol,
            Personality = personality.ToLower() switch
            {
                "aggressive" => AIPersonality.Aggressive,
                "defensive" => AIPersonality.Defensive,
                "miner" => AIPersonality.Miner,
                _ => AIPersonality.Defensive
            }
        };
        _gameEngine.EntityManager.AddComponent(ship.Id, aiComp);
        
        return ship;
    }

    private Entity CreateAsteroid(Vector3 position, ResourceType resourceType)
    {
        var asteroid = _gameEngine.EntityManager.CreateEntity($"{resourceType} Asteroid");
        
        var voxelComp = new VoxelStructureComponent();
        float size = 5f + Random.Shared.NextSingle() * 5f;
        voxelComp.AddBlock(new VoxelBlock(Vector3.Zero, new Vector3(size, size, size), "Iron", BlockType.Hull));
        _gameEngine.EntityManager.AddComponent(asteroid.Id, voxelComp);
        
        var physicsComp = new PhysicsComponent
        {
            Position = position,
            Mass = voxelComp.TotalMass,
            Velocity = new Vector3(
                (Random.Shared.NextSingle() - 0.5f) * 2f,
                (Random.Shared.NextSingle() - 0.5f) * 2f,
                (Random.Shared.NextSingle() - 0.5f) * 2f
            )
        };
        _gameEngine.EntityManager.AddComponent(asteroid.Id, physicsComp);
        
        // Note: Asteroid resources are managed by MiningSystem separately
        // This creates a visual representation only
        
        return asteroid;
    }

    private Entity CreateStation(Vector3 position, string stationType)
    {
        var station = _gameEngine.EntityManager.CreateEntity($"{stationType} Station");
        
        var voxelComp = new VoxelStructureComponent();
        // Create a larger, more complex structure for stations
        for (int x = -1; x <= 1; x++)
        {
            for (int y = -1; y <= 1; y++)
            {
                voxelComp.AddBlock(new VoxelBlock(
                    new Vector3(x * 8, y * 8, 0),
                    new Vector3(7, 7, 7),
                    "Titanium",
                    BlockType.Hull
                ));
            }
        }
        _gameEngine.EntityManager.AddComponent(station.Id, voxelComp);
        
        var physicsComp = new PhysicsComponent
        {
            Position = position,
            Mass = voxelComp.TotalMass,
            Velocity = Vector3.Zero
        };
        _gameEngine.EntityManager.AddComponent(station.Id, physicsComp);
        
        return station;
    }
}
