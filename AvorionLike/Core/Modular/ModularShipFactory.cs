using System.Numerics;
using AvorionLike.Core.AI;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Modular;

/// <summary>
/// Factory for creating modular ships with pre-configured designs for AI ships
/// Maps AI personalities and requirements to appropriate ship classes and configurations
/// </summary>
public class ModularShipFactory
{
    private readonly ModuleLibrary _library;
    private readonly ModularProceduralShipGenerator _generator;
    private readonly Logger _logger = Logger.Instance;
    private readonly Random _random;

    public ModularShipFactory(ModuleLibrary library, int? seed = null)
    {
        _library = library;
        _random = seed.HasValue ? new Random(seed.Value) : new Random();
        _generator = new ModularProceduralShipGenerator(_library, _random.Next());
    }

    /// <summary>
    /// Create a ship appropriate for the given AI personality
    /// </summary>
    public ModularGeneratedShip CreateShipForAI(AIPersonality personality, string name, string material = "Iron")
    {
        var config = CreateConfigForPersonality(personality, name, material);
        return _generator.GenerateShip(config);
    }

    /// <summary>
    /// Create configuration based on AI personality
    /// </summary>
    private ModularShipConfig CreateConfigForPersonality(AIPersonality personality, string name, string material)
    {
        return personality switch
        {
            AIPersonality.Trader => new ModularShipConfig
            {
                ShipName = name,
                Size = ShipSize.Corvette,
                Role = ShipRole.Trading,
                Material = material,
                Seed = _random.Next(),
                AddWings = false,
                AddWeapons = true,
                AddCargo = true,
                AddHyperdrive = true,
                DesiredWeaponMounts = 1, // Light defense
                MinimumEngines = 1
            },

            AIPersonality.Miner => new ModularShipConfig
            {
                ShipName = name,
                Size = ShipSize.Corvette,
                Role = ShipRole.Mining,
                Material = material,
                Seed = _random.Next(),
                AddWings = false,
                AddWeapons = true,
                AddCargo = true,
                AddHyperdrive = false,
                DesiredWeaponMounts = 1, // Light defense
                MinimumEngines = 1
            },

            AIPersonality.Aggressive => new ModularShipConfig
            {
                ShipName = name,
                Size = ShipSize.Frigate,
                Role = ShipRole.Combat,
                Material = material,
                Seed = _random.Next(),
                AddWings = true,
                AddWeapons = true,
                AddCargo = false,
                AddHyperdrive = true,
                DesiredWeaponMounts = 3, // Heavy weapons
                MinimumEngines = 2
            },

            AIPersonality.Defensive => new ModularShipConfig
            {
                ShipName = name,
                Size = ShipSize.Frigate,
                Role = ShipRole.Combat,
                Material = material,
                Seed = _random.Next(),
                AddWings = true,
                AddWeapons = true,
                AddCargo = false,
                AddHyperdrive = false,
                DesiredWeaponMounts = 2,
                MinimumEngines = 1
            },

            AIPersonality.Explorer => new ModularShipConfig
            {
                ShipName = name,
                Size = ShipSize.Fighter,
                Role = ShipRole.Exploration,
                Material = material,
                Seed = _random.Next(),
                AddWings = true,
                AddWeapons = true,
                AddCargo = false,
                AddHyperdrive = true,
                DesiredWeaponMounts = 1,
                MinimumEngines = 1
            },

            _ => new ModularShipConfig
            {
                ShipName = name,
                Size = ShipSize.Corvette,
                Role = ShipRole.Multipurpose,
                Material = material,
                Seed = _random.Next(),
                AddWings = true,
                AddWeapons = true,
                AddCargo = true,
                AddHyperdrive = true,
                DesiredWeaponMounts = 2,
                MinimumEngines = 1
            }
        };
    }

    /// <summary>
    /// Create a fighter ship for escort or patrol duties
    /// </summary>
    public ModularGeneratedShip CreateFighter(string name, string material = "Iron")
    {
        var config = new ModularShipConfig
        {
            ShipName = name,
            Size = ShipSize.Fighter,
            Role = ShipRole.Combat,
            Material = material,
            Seed = _random.Next(),
            AddWings = true,
            AddWeapons = true,
            AddCargo = false,
            AddHyperdrive = false,
            DesiredWeaponMounts = 2,
            MinimumEngines = 1
        };
        return _generator.GenerateShip(config);
    }

    /// <summary>
    /// Create a mining ship
    /// </summary>
    public ModularGeneratedShip CreateMiner(string name, string material = "Iron")
    {
        var config = new ModularShipConfig
        {
            ShipName = name,
            Size = ShipSize.Corvette,
            Role = ShipRole.Mining,
            Material = material,
            Seed = _random.Next(),
            AddWings = false,
            AddWeapons = true,
            AddCargo = true,
            AddHyperdrive = false,
            DesiredWeaponMounts = 1,
            MinimumEngines = 1
        };
        return _generator.GenerateShip(config);
    }

    /// <summary>
    /// Create a trader ship
    /// </summary>
    public ModularGeneratedShip CreateTrader(string name, string material = "Iron")
    {
        var config = new ModularShipConfig
        {
            ShipName = name,
            Size = ShipSize.Corvette,
            Role = ShipRole.Trading,
            Material = material,
            Seed = _random.Next(),
            AddWings = false,
            AddWeapons = true,
            AddCargo = true,
            AddHyperdrive = true,
            DesiredWeaponMounts = 1,
            MinimumEngines = 1
        };
        return _generator.GenerateShip(config);
    }

    /// <summary>
    /// Create a capital ship (battleship or cruiser)
    /// </summary>
    public ModularGeneratedShip CreateCapitalShip(string name, string material = "Titanium")
    {
        var config = new ModularShipConfig
        {
            ShipName = name,
            Size = _random.Next(2) == 0 ? ShipSize.Cruiser : ShipSize.Battleship,
            Role = ShipRole.Combat,
            Material = material,
            Seed = _random.Next(),
            AddWings = false,
            AddWeapons = true,
            AddCargo = true,
            AddHyperdrive = true,
            DesiredWeaponMounts = 5,
            MinimumEngines = 3
        };
        return _generator.GenerateShip(config);
    }

    /// <summary>
    /// Create a ship with custom configuration
    /// </summary>
    public ModularGeneratedShip CreateCustomShip(ModularShipConfig config)
    {
        return _generator.GenerateShip(config);
    }
}
