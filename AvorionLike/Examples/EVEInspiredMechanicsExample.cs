using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Navigation;
using AvorionLike.Core.Economy;
using AvorionLike.Core.Combat;
using AvorionLike.Core.AI;
using AvorionLike.Core.Resources;
using AvorionLike.Core.Physics;
using AvorionLike.Core.Procedural;

namespace AvorionLike.Examples;

/// <summary>
/// Example demonstrating EVE Online-inspired deep space mechanics
/// Shows wormholes, scanning, CONCORD, NPC economy, manufacturing, and fitting systems
/// </summary>
public class EVEInspiredMechanicsExample
{
    public static void Run()
    {
        Console.WriteLine("=== EVE Online-Inspired Deep Space Mechanics Demo ===\n");
        
        var entityManager = new EntityManager();
        int seed = 12345;
        
        // Initialize systems
        var wormholeSystem = new WormholeSystem(entityManager, seed);
        var scanningSystem = new ScanningSystem(entityManager);
        var concordSystem = new CONCORDSystem(entityManager);
        var economySystem = new EconomySystem(entityManager);
        var npcEconomySystem = new NPCEconomicAgentSystem(entityManager, economySystem, seed);
        var manufacturingSystem = new ManufacturingSystem(entityManager);
        var fittingSystem = new FittingSystem(entityManager);
        var galaxyGenerator = new GalaxyGenerator(seed);
        
        Console.WriteLine("✓ All systems initialized\n");
        
        // === 1. Wormhole System Demo ===
        Console.WriteLine("--- 1. Wormhole System ---");
        
        // Create a wandering wormhole
        Vector3 sector1 = new Vector3(100, 50, 25);
        Vector3 sector2 = new Vector3(-50, 100, -75);
        var wormholeId = wormholeSystem.CreateWormhole(sector1, sector2, WormholeClass.Class3, WormholeType.Wandering);
        var wormhole = entityManager.GetComponent<WormholeComponent>(wormholeId);
        
        Console.WriteLine($"Created {wormhole!.Type} wormhole: {wormhole.Designation}");
        Console.WriteLine($"  Class: {wormhole.Class}");
        Console.WriteLine($"  Source: ({sector1.X}, {sector1.Y}, {sector1.Z})");
        Console.WriteLine($"  Destination: ({sector2.X}, {sector2.Y}, {sector2.Z})");
        Console.WriteLine($"  Lifetime: {wormhole.RemainingLifetime / 3600:F1} hours");
        Console.WriteLine($"  Max Ship Mass: {wormhole.MaxShipMass / 1000000:F0} million kg");
        Console.WriteLine($"  Mass Restriction: {wormhole.GetMassRestriction()}");
        
        // Create a static wormhole to high-sec
        var staticWormholeId = wormholeSystem.CreateStaticWormhole(sector1, SecurityLevel.HighSec, WormholeClass.Class2);
        var staticWh = entityManager.GetComponent<WormholeComponent>(staticWormholeId);
        Console.WriteLine($"\nCreated Static wormhole: {staticWh!.Designation} → {staticWh.StaticDestinationType}");
        
        // Simulate a ship jump
        float shipMass = 150000000f; // 150 million kg
        Console.WriteLine($"\nShip attempting jump (mass: {shipMass / 1000000:F0}M kg)...");
        bool canJump = wormhole.CanShipJump(shipMass);
        Console.WriteLine($"  Can jump: {canJump}");
        
        if (canJump)
        {
            wormhole.ProcessJump(shipMass);
            Console.WriteLine($"  Jump successful! Wormhole stability: {wormhole.Stability}");
            Console.WriteLine($"  Remaining mass capacity: {wormhole.RemainingMass / 1000000:F0}M kg");
        }
        
        // === 2. Scanning System Demo ===
        Console.WriteLine("\n--- 2. Scanning System ---");
        
        // Create a scout ship with scanning equipment
        var scoutShip = entityManager.CreateEntity("Scout Ship");
        var scannerComponent = new ScanningComponent
        {
            EntityId = scoutShip.Id,
            DirectionalScannerRange = 14000f,
            ScanResolution = 1.5f,
            ProbeStrength = 1.2f,
            AvailableProbes = 8
        };
        entityManager.AddComponent(scoutShip.Id, scannerComponent);
        
        var scoutPhysics = new PhysicsComponent
        {
            EntityId = scoutShip.Id,
            Position = sector1 * 10000f, // Scale to sector units
            Mass = 50000000f
        };
        entityManager.AddComponent(scoutShip.Id, scoutPhysics);
        
        Console.WriteLine("Scout ship deployed with scanning equipment");
        Console.WriteLine($"  Scanner Range: {scannerComponent.DirectionalScannerRange:F0} km");
        Console.WriteLine($"  Available Probes: {scannerComponent.AvailableProbes}");
        
        // Perform directional scan
        var signatures = scanningSystem.PerformDirectionalScan(scoutShip.Id, Vector3.UnitX, 360f);
        Console.WriteLine($"\nDirectional scan detected {signatures.Count} signatures:");
        foreach (var sig in signatures.Take(3))
        {
            Console.WriteLine($"  - {sig.Name} ({sig.Type}) - Strength: {sig.SignatureStrength:P0}");
        }
        
        // Deploy probes
        var probePositions = new List<Vector3>
        {
            scoutPhysics.Position + new Vector3(1000, 0, 0),
            scoutPhysics.Position + new Vector3(-500, 866, 0),
            scoutPhysics.Position + new Vector3(-500, -866, 0),
            scoutPhysics.Position + new Vector3(0, 0, 1000)
        };
        scanningSystem.DeployProbes(scoutShip.Id, probePositions);
        Console.WriteLine($"\nDeployed {probePositions.Count} scanning probes");
        Console.WriteLine($"  Remaining probes: {scannerComponent.AvailableProbes}");
        
        // Perform probe scan
        var probeSignatures = scanningSystem.PerformProbeScan(scoutShip.Id);
        Console.WriteLine($"Probe scan progress: {probeSignatures.Count} signatures being analyzed");
        
        // === 3. CONCORD System Demo ===
        Console.WriteLine("\n--- 3. CONCORD Law Enforcement ---");
        
        // Create two ships - one attacker, one victim
        var attackerShip = entityManager.CreateEntity("Pirate Ship");
        var victimShip = entityManager.CreateEntity("Freighter");
        
        // Check sector security
        var sectorCoords = new Vector3(50, 50, 50); // Near center = high-sec
        var securityData = concordSystem.GetSectorSecurity(sectorCoords);
        Console.WriteLine($"Sector ({sectorCoords.X}, {sectorCoords.Y}, {sectorCoords.Z}):");
        Console.WriteLine($"  Security Level: {securityData.SecurityLevel}");
        Console.WriteLine($"  Security Rating: {securityData.SecurityRating:F1}");
        Console.WriteLine($"  CONCORD Response Time: {securityData.GetCONCORDResponseTime():F1} seconds");
        
        // Register illegal attack
        Console.WriteLine("\nPirate attacks freighter in high-sec...");
        concordSystem.RegisterIllegalAttack(attackerShip.Id, victimShip.Id, sectorCoords);
        
        var attackerStatus = entityManager.GetComponent<SecurityStatusComponent>(attackerShip.Id);
        if (attackerStatus != null)
        {
            Console.WriteLine($"  Attacker Status:");
            Console.WriteLine($"    Criminal Flag: {attackerStatus.IsCriminal}");
            Console.WriteLine($"    Security Status: {attackerStatus.SecurityStatus:F1}");
            Console.WriteLine($"    CONCORD Target: {attackerStatus.IsCONCORDTarget}");
            Console.WriteLine($"    Response ETA: {attackerStatus.CONCORDResponseTimer:F1}s");
        }
        
        // === 4. NPC Economy Demo ===
        Console.WriteLine("\n--- 4. NPC Economic Simulation ---");
        
        // Create a station
        var stationData = new StationData
        {
            Name = "Trade Hub Alpha",
            StationType = "Trading",
            Position = Vector3.Zero
        };
        var station = economySystem.CreateStation(stationData);
        
        Console.WriteLine($"Station '{station.Name}' created ({station.Type})");
        
        // Spawn NPC economic agents
        Console.WriteLine("\nSpawning NPC economic agents...");
        for (int i = 0; i < 5; i++)
        {
            npcEconomySystem.Update(121f); // Trigger spawning
        }
        
        var npcAgents = entityManager.GetAllComponents<NPCEconomicAgentComponent>();
        Console.WriteLine($"  Active NPC agents: {npcAgents.Count()}");
        
        foreach (var agent in npcAgents.Take(3))
        {
            Console.WriteLine($"    - {agent.AgentType}: {agent.Credits} credits, {agent.CurrentCargo}/{agent.MaxCargo} cargo");
        }
        
        // === 5. Manufacturing System Demo ===
        Console.WriteLine("\n--- 5. Manufacturing & Blueprints ---");
        
        // Create a blueprint
        var blueprintEntity = entityManager.CreateEntity("Frigate Blueprint");
        var blueprint = new BlueprintComponent
        {
            EntityId = blueprintEntity.Id,
            Name = "Combat Frigate",
            Type = BlueprintType.Ship,
            IsOriginal = true,
            MaterialEfficiency = 0,
            TimeEfficiency = 0,
            BaseProductionTime = 3600f,
            MaterialRequirements = new Dictionary<ResourceType, int>
            {
                { ResourceType.Iron, 1000 },
                { ResourceType.Titanium, 500 }
            }
        };
        entityManager.AddComponent(blueprintEntity.Id, blueprint);
        
        Console.WriteLine($"Blueprint: {blueprint.Name}");
        Console.WriteLine($"  Type: {blueprint.Type}");
        Console.WriteLine($"  Production Time: {blueprint.GetActualProductionTime() / 60:F1} minutes");
        Console.WriteLine("  Materials Required:");
        foreach (var mat in blueprint.GetActualMaterialRequirements())
        {
            Console.WriteLine($"    - {mat.Key}: {mat.Value}");
        }
        
        // Research the blueprint
        manufacturingSystem.ResearchMaterialEfficiency(blueprintEntity.Id, 250);
        Console.WriteLine($"\n  Researched! Material Efficiency: Level {blueprint.MaterialEfficiency}");
        Console.WriteLine($"  New material requirements: {blueprint.GetActualMaterialRequirements()[ResourceType.Iron]} Iron");
        
        // === 6. Fitting System Demo ===
        Console.WriteLine("\n--- 6. Ship Fitting System ---");
        
        // Create a ship with fitting component
        var combatShip = entityManager.CreateEntity("Combat Cruiser");
        var fitting = new FittingComponent
        {
            EntityId = combatShip.Id,
            MaxPowerGrid = 1500f,
            MaxCPU = 750f,
            MaxCapacitor = 2000f,
            CurrentCapacitor = 2000f,
            CapacitorRechargeRate = 15f,
            MaxModuleSlots = 12
        };
        entityManager.AddComponent(combatShip.Id, fitting);
        
        Console.WriteLine("Combat Cruiser fitting bay:");
        Console.WriteLine($"  Power Grid: {fitting.MaxPowerGrid} MW");
        Console.WriteLine($"  CPU: {fitting.MaxCPU} tf");
        Console.WriteLine($"  Capacitor: {fitting.MaxCapacitor} GJ");
        Console.WriteLine($"  Module Slots: {fitting.MaxModuleSlots}");
        
        // Create and fit modules
        var shieldBooster = fittingSystem.CreateModule("Large Shield Booster", FittingModuleType.ShieldBooster, 
            ModuleSlot.Medium, 100f, 50f, 150f);
        shieldBooster.Attributes["shieldBoostAmount"] = 500f;
        
        var afterburner = fittingSystem.CreateModule("10MN Afterburner", FittingModuleType.Afterburner,
            ModuleSlot.Medium, 80f, 40f, 50f);
            
        var weaponTurret = fittingSystem.CreateModule("Heavy Laser Turret", FittingModuleType.Turret,
            ModuleSlot.High, 150f, 75f, 0f);
        
        Console.WriteLine("\nFitting modules...");
        fittingSystem.FitModule(combatShip.Id, shieldBooster);
        fittingSystem.FitModule(combatShip.Id, afterburner);
        fittingSystem.FitModule(combatShip.Id, weaponTurret);
        
        Console.WriteLine($"  Modules fitted: {fitting.FittedModules.Count}");
        Console.WriteLine($"  Power Grid used: {fitting.UsedPowerGrid}/{fitting.MaxPowerGrid} MW");
        Console.WriteLine($"  CPU used: {fitting.UsedCPU}/{fitting.MaxCPU} tf");
        
        // Validate fitting
        var (isValid, errors) = fittingSystem.ValidateFitting(combatShip.Id);
        Console.WriteLine($"\nFitting validation: {(isValid ? "✓ Valid" : "✗ Invalid")}");
        
        // === 7. Galaxy Generation with Wormholes ===
        Console.WriteLine("\n--- 7. Procedural Galaxy with Wormholes ---");
        
        var sector = galaxyGenerator.GenerateSector(100, 50, 25, entityManager);
        Console.WriteLine($"Generated sector ({sector.X}, {sector.Y}, {sector.Z}):");
        Console.WriteLine($"  Asteroids: {sector.Asteroids.Count}");
        Console.WriteLine($"  Station: {(sector.Station != null ? sector.Station.Name : "None")}");
        Console.WriteLine($"  Wormholes: {sector.Wormholes.Count}");
        
        foreach (var wh in sector.Wormholes)
        {
            Console.WriteLine($"    - {wh.Designation} ({wh.WormholeClass}) → ({wh.DestinationSector.X:F0}, {wh.DestinationSector.Y:F0}, {wh.DestinationSector.Z:F0})");
        }
        
        // === Summary ===
        Console.WriteLine("\n=== Summary ===");
        Console.WriteLine("✓ Wormhole system: Dynamic topology with 6 classes");
        Console.WriteLine("✓ Scanning system: Directional scanner and probe mechanics");
        Console.WriteLine("✓ CONCORD: Law enforcement with security zones");
        Console.WriteLine("✓ NPC Economy: Background simulation with autonomous agents");
        Console.WriteLine("✓ Manufacturing: Blueprint research and production");
        Console.WriteLine("✓ Fitting: Power grid, CPU, capacitor constraints");
        Console.WriteLine("✓ Integrated: All systems work together for deep simulation");
        Console.WriteLine("\nEVE Online-inspired mechanics successfully implemented!");
    }
}
