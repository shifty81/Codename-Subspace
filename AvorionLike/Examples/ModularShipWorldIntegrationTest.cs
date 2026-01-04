using System.Numerics;
using AvorionLike.Core;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Modular;
using AvorionLike.Core.AI;
using AvorionLike.Core.Combat;
using AvorionLike.Core.Physics;

namespace AvorionLike.Examples;

/// <summary>
/// Test to verify modular ship integration with GameWorldPopulator
/// Ensures AI ships spawn correctly using the new modular system
/// </summary>
public class ModularShipWorldIntegrationTest
{
    private readonly GameEngine _gameEngine;
    
    public ModularShipWorldIntegrationTest(GameEngine gameEngine)
    {
        _gameEngine = gameEngine;
    }
    
    /// <summary>
    /// Run the integration test
    /// </summary>
    public bool RunTest()
    {
        Console.WriteLine("\n=== Modular Ship World Integration Test ===");
        Console.WriteLine("Testing modular ships in game world...\n");
        
        bool allPassed = true;
        
        allPassed &= TestGameWorldPopulation();
        allPassed &= TestShipComponents();
        allPassed &= TestAIShipVariety();
        
        Console.WriteLine("\n=== Test Results ===");
        if (allPassed)
        {
            Console.WriteLine("✅ ALL TESTS PASSED");
            Console.WriteLine("Modular ships successfully integrated into game world!");
        }
        else
        {
            Console.WriteLine("❌ SOME TESTS FAILED");
            Console.WriteLine("Review errors above for details.");
        }
        
        return allPassed;
    }
    
    /// <summary>
    /// Test that GameWorldPopulator creates ships correctly
    /// </summary>
    private bool TestGameWorldPopulation()
    {
        Console.WriteLine("Test 1: GameWorldPopulator Integration");
        Console.WriteLine("----------------------------------------");
        
        try
        {
            var populator = new GameWorldPopulator(_gameEngine, seed: 12345);
            var playerPos = new Vector3(0, 0, 0);
            
            // Get initial entity count
            int initialCount = _gameEngine.EntityManager.GetAllEntities().Count();
            
            // Populate starter area
            populator.PopulateStarterArea(playerPos, radius: 500f);
            
            // Get final entity count
            int finalCount = _gameEngine.EntityManager.GetAllEntities().Count();
            int entitiesCreated = finalCount - initialCount;
            
            Console.WriteLine($"  Entities created: {entitiesCreated}");
            
            if (entitiesCreated > 0)
            {
                Console.WriteLine("  ✓ GameWorldPopulator successfully creates entities");
                return true;
            }
            else
            {
                Console.WriteLine("  ✗ No entities created");
                return false;
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"  ✗ Exception: {ex.Message}");
            return false;
        }
    }
    
    /// <summary>
    /// Test that ships have correct components
    /// </summary>
    private bool TestShipComponents()
    {
        Console.WriteLine("\nTest 2: Ship Component Verification");
        Console.WriteLine("------------------------------------");
        
        try
        {
            var entities = _gameEngine.EntityManager.GetAllEntities();
            int shipsWithModularComponent = 0;
            int shipsWithPhysics = 0;
            int shipsWithCombat = 0;
            int shipsWithAI = 0;
            
            foreach (var entity in entities)
            {
                var modularShip = _gameEngine.EntityManager.GetComponent<ModularShipComponent>(entity.Id);
                if (modularShip != null)
                {
                    shipsWithModularComponent++;
                    
                    // Check for other required components
                    if (_gameEngine.EntityManager.GetComponent<PhysicsComponent>(entity.Id) != null)
                        shipsWithPhysics++;
                    
                    if (_gameEngine.EntityManager.GetComponent<CombatComponent>(entity.Id) != null)
                        shipsWithCombat++;
                    
                    if (_gameEngine.EntityManager.GetComponent<AIComponent>(entity.Id) != null)
                        shipsWithAI++;
                    
                    // Verify ship has modules
                    if (modularShip.Modules.Count == 0)
                    {
                        Console.WriteLine($"  ⚠ Warning: Ship '{modularShip.Name}' has no modules");
                    }
                }
            }
            
            Console.WriteLine($"  Ships with ModularShipComponent: {shipsWithModularComponent}");
            Console.WriteLine($"  Ships with PhysicsComponent: {shipsWithPhysics}");
            Console.WriteLine($"  Ships with CombatComponent: {shipsWithCombat}");
            Console.WriteLine($"  Ships with AIComponent: {shipsWithAI}");
            
            bool passed = shipsWithModularComponent > 0 &&
                         shipsWithModularComponent == shipsWithPhysics &&
                         shipsWithModularComponent == shipsWithCombat &&
                         shipsWithAI > 0;
            
            if (passed)
            {
                Console.WriteLine("  ✓ All ships have correct components");
                return true;
            }
            else
            {
                Console.WriteLine("  ✗ Component mismatch detected");
                return false;
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"  ✗ Exception: {ex.Message}");
            return false;
        }
    }
    
    /// <summary>
    /// Test that different AI personalities create different ship types
    /// </summary>
    private bool TestAIShipVariety()
    {
        Console.WriteLine("\nTest 3: AI Ship Variety");
        Console.WriteLine("------------------------");
        
        try
        {
            var entities = _gameEngine.EntityManager.GetAllEntities();
            var shipTypes = new Dictionary<string, int>();
            var personalities = new HashSet<AIPersonality>();
            
            foreach (var entity in entities)
            {
                var modularShip = _gameEngine.EntityManager.GetComponent<ModularShipComponent>(entity.Id);
                var aiComponent = _gameEngine.EntityManager.GetComponent<AIComponent>(entity.Id);
                
                if (modularShip != null && aiComponent != null)
                {
                    personalities.Add(aiComponent.Personality);
                    
                    // Count module types
                    string shipKey = $"{aiComponent.Personality} ({modularShip.Modules.Count} modules)";
                    if (!shipTypes.ContainsKey(shipKey))
                        shipTypes[shipKey] = 0;
                    shipTypes[shipKey]++;
                }
            }
            
            Console.WriteLine($"  Unique personalities: {personalities.Count}");
            foreach (var kvp in shipTypes)
            {
                Console.WriteLine($"    {kvp.Key}: {kvp.Value} ship(s)");
            }
            
            if (personalities.Count > 1)
            {
                Console.WriteLine("  ✓ Multiple ship types created");
                return true;
            }
            else
            {
                Console.WriteLine("  ✗ Insufficient variety");
                return false;
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"  ✗ Exception: {ex.Message}");
            return false;
        }
    }
}
