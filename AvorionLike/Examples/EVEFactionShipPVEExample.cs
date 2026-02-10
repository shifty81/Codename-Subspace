using AvorionLike.Core.Faction;
using AvorionLike.Core.Procedural;
using AvorionLike.Core.Quest;
using AvorionLike.Core.Logging;

namespace AvorionLike.Examples;

/// <summary>
/// Demonstrates the EVE-inspired game systems:
///   1. Four factions with lore and styles
///   2. Character creation with 3 slots, faction/bloodline/education
///   3. Faction-specific procedural ship generation with seed determinism and constraint checking
///   4. PVE mission generator with starter missions and progression
/// </summary>
public class EVEFactionShipPVEExample
{
    private readonly Logger _logger = Logger.Instance;

    public void RunExample()
    {
        Console.WriteLine("\n" + new string('=', 70));
        Console.WriteLine("   EVE-INSPIRED SYSTEMS: Factions, Characters, Ships & PVE Missions");
        Console.WriteLine(new string('=', 70));

        DemonstrateFactions();
        DemonstrateCharacterCreation();
        DemonstrateFactionShipGeneration();
        DemonstrateSeedDeterminism();
        DemonstrateConstraintChecking();
        DemonstratePVEMissions();
        DemonstrateFullPVELoop();
    }

    private void DemonstrateFactions()
    {
        Console.WriteLine("\n--- I. THE 4 FACTIONS ---\n");

        var profiles = EVEFactionDefinitions.GetAllProfiles();
        foreach (var (id, profile) in profiles)
        {
            Console.WriteLine(profile.GetSummary());
            Console.WriteLine();
        }
    }

    private void DemonstrateCharacterCreation()
    {
        Console.WriteLine("\n--- II. CHARACTER CREATION SYSTEM ---\n");

        var manager = new CharacterManager();

        // Display slot selection screen
        Console.WriteLine("=== Character Select Screen ===");
        for (int i = 0; i < manager.MaxSlots; i++)
        {
            Console.WriteLine($"  Slot {i + 1}: {manager.GetSlotDisplayText(i)}");
        }

        // Create characters in each slot
        Console.WriteLine("\n--- Creating Characters ---");

        var char1 = manager.CreateCharacter(0, "Imperator Vex",
            EVEFactionId.SanctumHegemony, Bloodline.TrueBloods, Education.Gunnery);
        Console.WriteLine($"\nSlot 1: {char1?.GetSummary()}");

        var char2 = manager.CreateCharacter(1, "Nexus-7 Alpha",
            EVEFactionId.CoreNexus, Bloodline.Deteis, Education.MissileOperations);
        Console.WriteLine($"\nSlot 2: {char2?.GetSummary()}");

        var char3 = manager.CreateCharacter(2, "RustBlade",
            EVEFactionId.RustScrapCoalition, Bloodline.Brutor, Education.Engineering);
        Console.WriteLine($"\nSlot 3: {char3?.GetSummary()}");

        // Show updated selection screen
        Console.WriteLine("\n=== Updated Character Select Screen ===");
        for (int i = 0; i < manager.MaxSlots; i++)
        {
            Console.WriteLine($"  Slot {i + 1}: {manager.GetSlotDisplayText(i)}");
        }

        // Test loading a character
        Console.WriteLine("\n--- Loading Character from Slot 2 ---");
        var loaded = manager.LoadCharacter(1);
        Console.WriteLine($"Loaded: {loaded?.Name} ({loaded?.GetFactionProfile().Name})");

        // Test invalid bloodline for faction
        Console.WriteLine("\n--- Testing Invalid Bloodline ---");
        var invalid = manager.CreateCharacter(0, "Invalid",
            EVEFactionId.SanctumHegemony, Bloodline.Brutor);
        Console.WriteLine($"Result: {(invalid == null ? "Correctly rejected — Brutor is not Sanctum Hegemony" : "ERROR")}");
    }

    private void DemonstrateFactionShipGeneration()
    {
        Console.WriteLine("\n--- III. FACTION-SPECIFIC SHIP GENERATION ---\n");

        var generator = new FactionShipGenerator(42);

        foreach (EVEFactionId factionId in Enum.GetValues<EVEFactionId>())
        {
            var profile = EVEFactionDefinitions.GetProfile(factionId);
            Console.WriteLine($"Generating ship for: {profile.Name}");
            Console.WriteLine($"  Style: {profile.VisualStyle}");

            var ship = generator.GenerateFactionShip(new FactionShipGenerationConfig
            {
                FactionId = factionId,
                Size = ShipSize.Frigate,
                Role = ShipRole.Combat,
                Seed = 42 + (int)factionId
            });

            Console.WriteLine($"  Blocks: {ship.Structure.Blocks.Count}");
            Console.WriteLine($"  Thrust: {ship.TotalThrust:F0}N");
            Console.WriteLine($"  Power: {ship.TotalPowerGeneration:F0}W");
            Console.WriteLine($"  Shield: {ship.TotalShieldCapacity:F0}");
            Console.WriteLine($"  Weapons: {ship.WeaponMountCount}");
            Console.WriteLine($"  Cargo: {ship.CargoBlockCount}");
            Console.WriteLine();
        }
    }

    private void DemonstrateSeedDeterminism()
    {
        Console.WriteLine("--- SEED-BASED DETERMINISTIC GENERATION ---\n");

        var generator = new FactionShipGenerator();
        int testSeed = 12345;

        var ship1 = generator.GenerateFactionShip(new FactionShipGenerationConfig
        {
            FactionId = EVEFactionId.CoreNexus,
            Size = ShipSize.Corvette,
            Role = ShipRole.Combat,
            Seed = testSeed
        });

        var ship2 = generator.GenerateFactionShip(new FactionShipGenerationConfig
        {
            FactionId = EVEFactionId.CoreNexus,
            Size = ShipSize.Corvette,
            Role = ShipRole.Combat,
            Seed = testSeed
        });

        bool blocksMatch = ship1.Structure.Blocks.Count == ship2.Structure.Blocks.Count;
        bool thrustMatch = Math.Abs(ship1.TotalThrust - ship2.TotalThrust) < 0.01f;
        Console.WriteLine($"Same seed ({testSeed}) produces identical ships:");
        Console.WriteLine($"  Block count: {ship1.Structure.Blocks.Count} == {ship2.Structure.Blocks.Count} → {blocksMatch}");
        Console.WriteLine($"  Thrust: {ship1.TotalThrust:F0} == {ship2.TotalThrust:F0} → {thrustMatch}");
        Console.WriteLine($"  Deterministic: {blocksMatch && thrustMatch}");
        Console.WriteLine();
    }

    private void DemonstrateConstraintChecking()
    {
        Console.WriteLine("--- BOUNDING-BOX CONSTRAINT CHECKING ---\n");

        var existingModules = new List<PlacedModule>
        {
            new PlacedModule
            {
                Position = new System.Numerics.Vector3(0, 0, 0),
                Bounds = new ModuleBounds(new System.Numerics.Vector3(0, 0, 0),
                    new System.Numerics.Vector3(10, 5, 10))
            }
        };

        // Overlapping position
        bool overlap = FactionShipGenerator.CheckCollision(
            new System.Numerics.Vector3(3, 0, 0),
            new System.Numerics.Vector3(5, 5, 5),
            existingModules);
        Console.WriteLine($"Module at (3,0,0) overlaps existing at origin: {overlap} (expected: true)");

        // Non-overlapping position
        bool noOverlap = FactionShipGenerator.CheckCollision(
            new System.Numerics.Vector3(20, 0, 0),
            new System.Numerics.Vector3(5, 5, 5),
            existingModules);
        Console.WriteLine($"Module at (20,0,0) overlaps existing at origin: {noOverlap} (expected: false)");
        Console.WriteLine();
    }

    private void DemonstratePVEMissions()
    {
        Console.WriteLine("--- IV. PVE MISSION GENERATOR ---\n");

        var missionGen = new PVEMissionGenerator(99);

        // Generate starter mission for each faction
        foreach (EVEFactionId factionId in Enum.GetValues<EVEFactionId>())
        {
            var profile = EVEFactionDefinitions.GetProfile(factionId);
            var starterMission = missionGen.GenerateStarterMission(factionId);

            Console.WriteLine($"Starter Mission for {profile.Name}:");
            Console.WriteLine($"  Title: {starterMission.Title}");
            Console.WriteLine($"  Difficulty: {starterMission.Difficulty}");
            Console.WriteLine($"  Objectives: {starterMission.Objectives.Count}");
            foreach (var obj in starterMission.Objectives)
            {
                Console.WriteLine($"    - {obj.Type}: {obj.Description} (x{obj.RequiredQuantity})");
            }
            Console.WriteLine($"  Rewards: {starterMission.Rewards.Count}");
            foreach (var reward in starterMission.Rewards)
            {
                Console.WriteLine($"    - {reward.Type}: {reward.Description}");
            }
            Console.WriteLine();
        }

        // Generate mission board
        Console.WriteLine("--- MISSION BOARD (Core Nexus) ---\n");
        var missions = missionGen.GenerateMissionBoard(EVEFactionId.CoreNexus, 5);
        foreach (var mission in missions)
        {
            Console.WriteLine($"  [{mission.Difficulty}] {mission.Title}");
        }
        Console.WriteLine();
    }

    private void DemonstrateFullPVELoop()
    {
        Console.WriteLine("--- FULL PVE LOOP DEMONSTRATION ---\n");

        // Step 1: Create character
        var charManager = new CharacterManager();
        var character = charManager.CreateCharacter(0, "Captain Nova",
            EVEFactionId.VanguardRepublic, Bloodline.Intaki, Education.Drones);

        Console.WriteLine("Step 1: Character Created");
        Console.WriteLine($"  {character!.GetSummary()}\n");

        // Step 2: Generate starter ship
        var shipGen = new FactionShipGenerator(character.Name.GetHashCode());
        var starterShip = shipGen.GenerateStarterShip(character.FactionId);

        Console.WriteLine("Step 2: Starter Ship Generated");
        Console.WriteLine($"  Faction: {character.GetFactionProfile().Name}");
        Console.WriteLine($"  Blocks: {starterShip.Structure.Blocks.Count}");
        Console.WriteLine($"  Thrust: {starterShip.TotalThrust:F0}N\n");

        // Step 3: Get starter mission
        var missionGen = new PVEMissionGenerator();
        var starterMission = missionGen.GenerateStarterMission(character.FactionId);

        Console.WriteLine("Step 3: Starter Mission Received");
        Console.WriteLine($"  '{starterMission.Title}'");
        Console.WriteLine($"  {starterMission.Description}\n");

        // Step 4: Simulate completing the mission
        starterMission.Accept();
        foreach (var obj in starterMission.Objectives)
        {
            obj.Activate();
            for (int i = 0; i < obj.RequiredQuantity; i++)
            {
                obj.Progress(1);
            }
        }
        starterMission.Update();

        Console.WriteLine("Step 4: Mission Completed");
        Console.WriteLine($"  Status: {starterMission.Status}");
        Console.WriteLine($"  Completion: {starterMission.CompletionPercentage:F0}%\n");

        // Step 5: Progression — harder missions become available
        var missionBoard = missionGen.GenerateMissionBoard(character.FactionId, 3);
        Console.WriteLine("Step 5: New Missions Available (Progression)");
        foreach (var mission in missionBoard)
        {
            Console.WriteLine($"  [{mission.Difficulty}] {mission.Title}");
        }

        Console.WriteLine("\n" + new string('=', 70));
        Console.WriteLine("   PVE LOOP COMPLETE: Create → Ship → Mission → Upgrade → Repeat");
        Console.WriteLine(new string('=', 70));
    }
}
