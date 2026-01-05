using System.Numerics;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Modular;

/// <summary>
/// Procedurally generates modular stations
/// Similar to ModularProceduralShipGenerator but for stations
/// </summary>
public class ModularProceduralStationGenerator
{
    private readonly StationModuleLibrary _moduleLibrary;
    private readonly Logger _logger = Logger.Instance;
    private readonly Random _random;
    
    public ModularProceduralStationGenerator(StationModuleLibrary moduleLibrary, int seed = 0)
    {
        _moduleLibrary = moduleLibrary;
        _random = seed == 0 ? new Random() : new Random(seed);
    }
    
    /// <summary>
    /// Generate a complete modular station
    /// </summary>
    public ModularStationComponent GenerateStation(StationType type, string materialType, int complexity = 3)
    {
        var station = new ModularStationComponent
        {
            EntityId = Guid.NewGuid(),
            Name = GenerateStationName(type),
            Type = type
        };
        
        _logger.Info("StationGenerator", $"Generating {type} station with complexity {complexity}");
        
        // Create core hub module
        var coreModule = CreateCoreModule(materialType);
        station.AddModule(coreModule);
        station.CoreModuleId = coreModule.Id;
        
        // Add modules based on station type and complexity
        AddTypeSpecificModules(station, type, materialType, complexity);
        
        // Add connectors to improve structure
        AddStructuralConnectors(station, materialType, complexity / 2);
        
        station.RecalculateStats();
        
        _logger.Info("StationGenerator", 
            $"Generated {station.Name}: {station.Modules.Count} modules, " +
            $"{station.TotalMass:F0}t mass, {station.MaxTotalHealth:F0} health");
        
        return station;
    }
    
    /// <summary>
    /// Create the core hub module
    /// </summary>
    private StationModulePart CreateCoreModule(string materialType)
    {
        var definition = _moduleLibrary.GetDefinition("station_hub_basic");
        if (definition == null)
        {
            throw new InvalidOperationException("Core hub module definition not found");
        }
        
        return CreateModuleFromDefinition(definition, Vector3.Zero, Vector3.Zero, materialType);
    }
    
    /// <summary>
    /// Add modules specific to station type
    /// </summary>
    private void AddTypeSpecificModules(ModularStationComponent station, StationType type, 
        string materialType, int complexity)
    {
        switch (type)
        {
            case StationType.TradingPost:
                AddTradingPostModules(station, materialType, complexity);
                break;
            case StationType.Shipyard:
                AddShipyardModules(station, materialType, complexity);
                break;
            case StationType.Factory:
                AddFactoryModules(station, materialType, complexity);
                break;
            case StationType.MiningStation:
                AddMiningStationModules(station, materialType, complexity);
                break;
            case StationType.ResearchStation:
                AddResearchStationModules(station, materialType, complexity);
                break;
            case StationType.DefensePlatform:
                AddDefensePlatformModules(station, materialType, complexity);
                break;
            case StationType.RefuelingDepot:
                AddRefuelingDepotModules(station, materialType, complexity);
                break;
            case StationType.CommandCenter:
                AddCommandCenterModules(station, materialType, complexity);
                break;
            case StationType.Habitat:
                AddHabitatModules(station, materialType, complexity);
                break;
        }
    }
    
    private void AddTradingPostModules(ModularStationComponent station, string materialType, int complexity)
    {
        // Trading posts need: docking bays, storage, trade modules
        AddModulesInPattern(station, "station_trade_market", materialType, 1 + complexity);
        AddModulesInPattern(station, "station_docking_small", materialType, 2 + complexity);
        AddModulesInPattern(station, "station_storage_basic", materialType, 2 + complexity * 2);
        AddModulesInPattern(station, "station_habitat_basic", materialType, 1 + complexity / 2);
    }
    
    private void AddShipyardModules(ModularStationComponent station, string materialType, int complexity)
    {
        // Shipyards need: large docking bays, factories, storage
        AddModulesInPattern(station, "station_docking_large", materialType, 2 + complexity);
        AddModulesInPattern(station, "station_factory_basic", materialType, 2 + complexity);
        AddModulesInPattern(station, "station_storage_large", materialType, 1 + complexity);
        AddModulesInPattern(station, "station_habitat_basic", materialType, 2 + complexity);
    }
    
    private void AddFactoryModules(ModularStationComponent station, string materialType, int complexity)
    {
        // Factories need: production, storage, power
        AddModulesInPattern(station, "station_factory_basic", materialType, 3 + complexity * 2);
        AddModulesInPattern(station, "station_refinery", materialType, 1 + complexity);
        AddModulesInPattern(station, "station_storage_large", materialType, 2 + complexity);
        AddModulesInPattern(station, "station_power_generator", materialType, 2 + complexity);
        AddModulesInPattern(station, "station_habitat_basic", materialType, 1 + complexity);
    }
    
    private void AddMiningStationModules(ModularStationComponent station, string materialType, int complexity)
    {
        // Mining stations need: refineries, storage, docking
        AddModulesInPattern(station, "station_refinery", materialType, 2 + complexity * 2);
        AddModulesInPattern(station, "station_storage_large", materialType, 3 + complexity * 2);
        AddModulesInPattern(station, "station_docking_small", materialType, 2 + complexity);
        AddModulesInPattern(station, "station_power_generator", materialType, 1 + complexity);
    }
    
    private void AddResearchStationModules(ModularStationComponent station, string materialType, int complexity)
    {
        // Research stations need: labs, habitat, power
        AddModulesInPattern(station, "station_research_lab", materialType, 2 + complexity * 2);
        AddModulesInPattern(station, "station_habitat_basic", materialType, 3 + complexity);
        AddModulesInPattern(station, "station_power_generator", materialType, 2 + complexity);
        AddModulesInPattern(station, "station_sensor_array", materialType, 1 + complexity);
    }
    
    private void AddDefensePlatformModules(ModularStationComponent station, string materialType, int complexity)
    {
        // Defense platforms need: turrets, shields, sensors
        AddModulesInPattern(station, "station_defense_turret", materialType, 4 + complexity * 3);
        AddModulesInPattern(station, "station_shield_generator", materialType, 2 + complexity);
        AddModulesInPattern(station, "station_sensor_array", materialType, 2 + complexity);
        AddModulesInPattern(station, "station_power_generator", materialType, 3 + complexity);
    }
    
    private void AddRefuelingDepotModules(ModularStationComponent station, string materialType, int complexity)
    {
        // Refueling depots need: docking, storage, power
        AddModulesInPattern(station, "station_docking_small", materialType, 3 + complexity * 2);
        AddModulesInPattern(station, "station_storage_basic", materialType, 4 + complexity * 2);
        AddModulesInPattern(station, "station_power_generator", materialType, 2 + complexity);
    }
    
    private void AddCommandCenterModules(ModularStationComponent station, string materialType, int complexity)
    {
        // Command centers need: advanced hub, sensors, habitat, defense
        AddModulesInPattern(station, "station_sensor_array", materialType, 3 + complexity);
        AddModulesInPattern(station, "station_habitat_basic", materialType, 4 + complexity);
        AddModulesInPattern(station, "station_defense_turret", materialType, 3 + complexity);
        AddModulesInPattern(station, "station_shield_generator", materialType, 2 + complexity);
        AddModulesInPattern(station, "station_power_generator", materialType, 2 + complexity);
    }
    
    private void AddHabitatModules(ModularStationComponent station, string materialType, int complexity)
    {
        // Habitats need: lots of crew quarters, life support
        AddModulesInPattern(station, "station_habitat_basic", materialType, 5 + complexity * 3);
        AddModulesInPattern(station, "station_docking_small", materialType, 2 + complexity);
        AddModulesInPattern(station, "station_power_generator", materialType, 2 + complexity);
    }
    
    /// <summary>
    /// Add modules in a radial pattern around the core
    /// </summary>
    private void AddModulesInPattern(ModularStationComponent station, string moduleId, 
        string materialType, int count)
    {
        var definition = _moduleLibrary.GetDefinition(moduleId);
        if (definition == null) return;
        
        // Get core module for positioning
        var coreModule = station.GetModule(station.CoreModuleId ?? Guid.Empty);
        if (coreModule == null) return;
        
        // Calculate positions in a radial pattern
        float radius = 20f; // Base radius
        float angleStep = 360f / count;
        float currentAngle = _random.Next(360); // Random starting angle
        
        for (int i = 0; i < count; i++)
        {
            float angle = currentAngle + angleStep * i;
            float angleRad = angle * MathF.PI / 180f;
            
            // Vary radius slightly for more organic look
            float actualRadius = radius + _random.Next(-5, 6);
            
            var position = new Vector3(
                MathF.Cos(angleRad) * actualRadius,
                _random.Next(-10, 11), // Some vertical variation
                MathF.Sin(angleRad) * actualRadius
            );
            
            var rotation = new Vector3(
                _random.Next(-15, 16),
                angle,
                _random.Next(-15, 16)
            );
            
            var module = CreateModuleFromDefinition(definition, position, rotation, materialType);
            station.AddModule(module);
            
            // Attach to core
            module.AttachedToModules.Add(coreModule.Id);
            coreModule.AttachModule(module.Id);
        }
    }
    
    /// <summary>
    /// Add structural connectors between modules
    /// </summary>
    private void AddStructuralConnectors(ModularStationComponent station, string materialType, int count)
    {
        var connectorDef = _moduleLibrary.GetDefinition("station_connector");
        if (connectorDef == null) return;
        
        var modules = station.Modules.Where(m => m.Category != StationModuleCategory.Structural).ToList();
        if (modules.Count < 2) return;
        
        for (int i = 0; i < count && i < modules.Count - 1; i++)
        {
            var module1 = modules[i];
            var module2 = modules[i + 1];
            
            // Position connector between two modules
            var midpoint = (module1.Position + module2.Position) / 2f;
            var direction = Vector3.Normalize(module2.Position - module1.Position);
            
            // Calculate rotation to align connector from module1 to module2
            // Using atan2 to get angle in XZ plane (horizontal rotation)
            var angleY = MathF.Atan2(direction.X, direction.Z) * (180f / MathF.PI);
            var rotation = new Vector3(0, angleY, 0);
            
            var connector = CreateModuleFromDefinition(connectorDef, midpoint, rotation, materialType);
            station.AddModule(connector);
            
            // Attach to both modules
            connector.AttachedToModules.Add(module1.Id);
            connector.AttachedToModules.Add(module2.Id);
            module1.AttachModule(connector.Id);
            module2.AttachModule(connector.Id);
        }
    }
    
    /// <summary>
    /// Create a module instance from a definition
    /// </summary>
    private StationModulePart CreateModuleFromDefinition(StationModuleDefinition definition, 
        Vector3 position, Vector3 rotation, string materialType)
    {
        return new StationModulePart
        {
            Id = Guid.NewGuid(),
            ModuleDefinitionId = definition.Id,
            Position = position,
            Rotation = rotation,
            MaterialType = materialType,
            Health = definition.GetHealthForMaterial(materialType),
            MaxHealth = definition.GetHealthForMaterial(materialType),
            Mass = definition.GetMassForMaterial(materialType),
            Category = definition.Category,
            FunctionalStats = definition.GetStatsForMaterial(materialType)
        };
    }
    
    /// <summary>
    /// Generate a name for the station
    /// </summary>
    private string GenerateStationName(StationType type)
    {
        var prefixes = new[] { "Alpha", "Beta", "Gamma", "Delta", "Epsilon", "Zeta", "Omega", "Nova" };
        var suffixes = new[] { "One", "Prime", "Central", "Outpost", "Station", "Complex" };
        
        var prefix = prefixes[_random.Next(prefixes.Length)];
        var suffix = suffixes[_random.Next(suffixes.Length)];
        
        return $"{prefix} {type} {suffix}";
    }
}
