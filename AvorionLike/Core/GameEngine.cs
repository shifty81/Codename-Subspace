using AvorionLike.Core.ECS;
using AvorionLike.Core.Physics;
using AvorionLike.Core.Scripting;
using AvorionLike.Core.Networking;
using AvorionLike.Core.Procedural;
using AvorionLike.Core.Resources;
using AvorionLike.Core.RPG;
using AvorionLike.Core.Configuration;
using AvorionLike.Core.Logging;
using AvorionLike.Core.Events;
using AvorionLike.Core.Combat;
using AvorionLike.Core.Mining;
using AvorionLike.Core.Fleet;
using AvorionLike.Core.Navigation;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Economy;
using AvorionLike.Core.Persistence;
using AvorionLike.Core.Power;

namespace AvorionLike.Core;

/// <summary>
/// Main game engine that manages all core systems
/// </summary>
public class GameEngine
{
    // Core ECS
    public EntityManager EntityManager { get; private set; } = null!;
    
    // Systems
    public PhysicsSystem PhysicsSystem { get; private set; } = null!;
    public ScriptingEngine ScriptingEngine { get; private set; } = null!;
    public GalaxyGenerator GalaxyGenerator { get; private set; } = null!;
    public CraftingSystem CraftingSystem { get; private set; } = null!;
    public LootSystem LootSystem { get; private set; } = null!;
    public TradingSystem TradingSystem { get; private set; } = null!;
    public PodDockingSystem PodDockingSystem { get; private set; } = null!;
    public PodAbilitySystem PodAbilitySystem { get; private set; } = null!;
    
    // New systems
    public CombatSystem CombatSystem { get; private set; } = null!;
    public DamageSystem DamageSystem { get; private set; } = null!;
    public MiningSystem MiningSystem { get; private set; } = null!;
    public FleetManagementSystem FleetManagementSystem { get; private set; } = null!;
    public NavigationSystem NavigationSystem { get; private set; } = null!;
    public BuildSystem BuildSystem { get; private set; } = null!;
    public EconomySystem EconomySystem { get; private set; } = null!;
    public CollisionSystem CollisionSystem { get; private set; } = null!;
    public PowerSystem PowerSystem { get; private set; } = null!;
    
    // Networking
    public GameServer? GameServer { get; private set; }
    
    // State
    public bool IsRunning { get; private set; }
    private DateTime _lastUpdateTime;
    private readonly int _galaxySeed;

    public GameEngine(int galaxySeed = 0)
    {
        _galaxySeed = galaxySeed;
        Initialize();
    }

    /// <summary>
    /// Initialize all engine systems
    /// </summary>
    private void Initialize()
    {
        // Initialize logging first
        var config = ConfigurationManager.Instance.Config;
        
        if (config.Development.LogToFile)
        {
            var logPath = Path.Combine(
                Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
                "Codename-Subspace",
                "Logs"
            );
            Logger.Instance.EnableFileLogging(logPath);
        }

        // Set log level from configuration
        var logLevel = Enum.TryParse<LogLevel>(config.Development.LogLevel, out var level) 
            ? level 
            : LogLevel.Info;
        Logger.Instance.SetMinimumLevel(logLevel);

        Logger.Instance.Info("GameEngine", "Initializing Codename:Subspace Game Engine...");

        // Initialize ECS
        EntityManager = new EntityManager();
        Logger.Instance.Info("GameEngine", "EntityManager initialized");

        // Initialize systems
        PhysicsSystem = new PhysicsSystem(EntityManager);
        CollisionSystem = new CollisionSystem(EntityManager);
        ScriptingEngine = new ScriptingEngine();
        GalaxyGenerator = new GalaxyGenerator(_galaxySeed);
        CraftingSystem = new CraftingSystem();
        LootSystem = new LootSystem();
        TradingSystem = new TradingSystem();
        PodDockingSystem = new PodDockingSystem(EntityManager);
        PodAbilitySystem = new PodAbilitySystem(EntityManager);
        CombatSystem = new CombatSystem(EntityManager);
        DamageSystem = new DamageSystem(EntityManager);
        MiningSystem = new MiningSystem(EntityManager);
        FleetManagementSystem = new FleetManagementSystem(EntityManager);
        NavigationSystem = new NavigationSystem(EntityManager);
        BuildSystem = new BuildSystem(EntityManager);
        EconomySystem = new EconomySystem(EntityManager);
        PowerSystem = new PowerSystem(EntityManager, EventSystem.Instance, Logger.Instance);
        Logger.Instance.Info("GameEngine", "All systems initialized");

        // Register systems with entity manager
        EntityManager.RegisterSystem(PhysicsSystem);
        EntityManager.RegisterSystem(CollisionSystem);
        EntityManager.RegisterSystem(CombatSystem);
        EntityManager.RegisterSystem(DamageSystem);
        EntityManager.RegisterSystem(MiningSystem);
        EntityManager.RegisterSystem(FleetManagementSystem);
        EntityManager.RegisterSystem(NavigationSystem);
        EntityManager.RegisterSystem(BuildSystem);
        EntityManager.RegisterSystem(EconomySystem);
        EntityManager.RegisterSystem(PowerSystem);

        // Register engine API for scripting
        ScriptingEngine.RegisterObject("Engine", this);
        ScriptingEngine.RegisterObject("EntityManager", EntityManager);

        _lastUpdateTime = DateTime.UtcNow;

        // Publish game started event
        EventSystem.Instance.Publish(GameEvents.GameStarted, new GameEvent());
        
        Logger.Instance.Info("GameEngine", "Game Engine initialized successfully");
        Console.WriteLine("Game Engine initialized successfully");
    }

    /// <summary>
    /// Start the game engine
    /// </summary>
    public void Start()
    {
        if (IsRunning) return;

        IsRunning = true;
        _lastUpdateTime = DateTime.UtcNow;
        
        Logger.Instance.Info("GameEngine", "Game Engine started");
        Console.WriteLine("Game Engine started");
    }

    /// <summary>
    /// Stop the game engine
    /// </summary>
    public void Stop()
    {
        if (!IsRunning) return;

        IsRunning = false;
        
        Logger.Instance.Info("GameEngine", "Stopping Game Engine...");
        
        EntityManager.Shutdown();
        GameServer?.Stop();
        ScriptingEngine.Dispose();
        
        // Shutdown logger last
        Logger.Instance.Shutdown();
        
        Console.WriteLine("Game Engine stopped");
    }

    /// <summary>
    /// Update the game engine (call this each frame)
    /// </summary>
    public void Update()
    {
        if (!IsRunning) return;

        // Calculate delta time
        var currentTime = DateTime.UtcNow;
        float deltaTime = (float)(currentTime - _lastUpdateTime).TotalSeconds;
        _lastUpdateTime = currentTime;

        // Clamp delta time to prevent huge jumps
        deltaTime = Math.Min(deltaTime, 0.1f);

        // Process queued events
        EventSystem.Instance.ProcessQueuedEvents();

        // Update all systems
        EntityManager.UpdateSystems(deltaTime);
    }

    /// <summary>
    /// Start multiplayer server
    /// </summary>
    public void StartServer(int port = 27015)
    {
        if (GameServer != null && GameServer.IsRunning)
        {
            Console.WriteLine("Server is already running");
            return;
        }

        GameServer = new GameServer(port);
        GameServer.Start();
    }

    /// <summary>
    /// Stop multiplayer server
    /// </summary>
    public void StopServer()
    {
        GameServer?.Stop();
    }

    /// <summary>
    /// Load a mod script
    /// </summary>
    public bool LoadMod(string modPath)
    {
        return ScriptingEngine.LoadMod(modPath);
    }

    /// <summary>
    /// Execute a script command
    /// </summary>
    public object[]? ExecuteScript(string script)
    {
        return ScriptingEngine.ExecuteScript(script);
    }

    /// <summary>
    /// Generate a galaxy sector
    /// </summary>
    public Procedural.GalaxySector GenerateSector(int x, int y, int z)
    {
        return GalaxyGenerator.GenerateSector(x, y, z);
    }

    /// <summary>
    /// Get engine statistics
    /// </summary>
    public EngineStatistics GetStatistics()
    {
        // Count all components across all component types
        int totalComponents = 0;
        foreach (var entity in EntityManager.GetAllEntities())
        {
            if (EntityManager.HasComponent<PhysicsComponent>(entity.Id)) totalComponents++;
            if (EntityManager.HasComponent<VoxelStructureComponent>(entity.Id)) totalComponents++;
            if (EntityManager.HasComponent<InventoryComponent>(entity.Id)) totalComponents++;
            if (EntityManager.HasComponent<ProgressionComponent>(entity.Id)) totalComponents++;
            if (EntityManager.HasComponent<CombatComponent>(entity.Id)) totalComponents++;
            if (EntityManager.HasComponent<HyperdriveComponent>(entity.Id)) totalComponents++;
            if (EntityManager.HasComponent<SectorLocationComponent>(entity.Id)) totalComponents++;
            if (EntityManager.HasComponent<FactionComponent>(entity.Id)) totalComponents++;
        }
        
        return new EngineStatistics
        {
            TotalEntities = EntityManager.GetAllEntities().Count(),
            TotalComponents = totalComponents,
            IsServerRunning = GameServer?.IsRunning ?? false,
            PhysicsEnabled = PhysicsSystem.IsEnabled
        };
    }

    /// <summary>
    /// Save the current game state
    /// </summary>
    /// <param name="saveName">Name of the save file</param>
    /// <returns>True if save was successful</returns>
    public bool SaveGame(string saveName)
    {
        try
        {
            Logger.Instance.Info("GameEngine", $"Saving game: {saveName}");
            
            var saveData = new SaveGameData
            {
                SaveName = saveName,
                SaveTime = DateTime.UtcNow,
                Version = "1.0.0",
                GalaxySeed = _galaxySeed,
                GameState = new Dictionary<string, object>
                {
                    ["IsRunning"] = IsRunning
                }
            };

            // Serialize all entities
            foreach (var entity in EntityManager.GetAllEntities())
            {
                var entityData = new EntityData
                {
                    EntityId = entity.Id,
                    EntityName = entity.Name,
                    IsActive = entity.IsActive
                };

                // Serialize all components that implement ISerializable
                SerializeComponent<PhysicsComponent>(entity, entityData);
                SerializeComponent<VoxelStructureComponent>(entity, entityData);
                SerializeComponent<InventoryComponent>(entity, entityData);
                SerializeComponent<ProgressionComponent>(entity, entityData);
                SerializeComponent<FactionComponent>(entity, entityData);

                saveData.Entities.Add(entityData);
            }

            // Save to file
            bool success = SaveGameManager.Instance.SaveGame(saveData, saveName);
            
            if (success)
            {
                Logger.Instance.Info("GameEngine", $"Game saved successfully: {saveName}");
                EventSystem.Instance.Publish(GameEvents.GameSaved, new GameEvent());
            }
            else
            {
                Logger.Instance.Error("GameEngine", $"Failed to save game: {saveName}");
            }
            
            return success;
        }
        catch (Exception ex)
        {
            Logger.Instance.Error("GameEngine", $"Error saving game: {ex.Message}");
            return false;
        }
    }

    /// <summary>
    /// Load a saved game state
    /// </summary>
    /// <param name="saveName">Name of the save file to load</param>
    /// <returns>True if load was successful</returns>
    public bool LoadGame(string saveName)
    {
        try
        {
            Logger.Instance.Info("GameEngine", $"Loading game: {saveName}");
            
            var saveData = SaveGameManager.Instance.LoadGame(saveName);
            if (saveData == null)
            {
                Logger.Instance.Error("GameEngine", $"Failed to load game: {saveName}");
                return false;
            }

            // Clear existing entities
            var existingEntities = EntityManager.GetAllEntities().ToList();
            foreach (var entity in existingEntities)
            {
                EntityManager.DestroyEntity(entity.Id);
            }

            // Restore entities
            foreach (var entityData in saveData.Entities)
            {
                var entity = EntityManager.CreateEntity(entityData.EntityName);
                entity.IsActive = entityData.IsActive;

                // Deserialize all components
                foreach (var componentData in entityData.Components)
                {
                    DeserializeComponent(entity.Id, componentData);
                }
            }

            Logger.Instance.Info("GameEngine", $"Game loaded successfully: {saveName}");
            EventSystem.Instance.Publish(GameEvents.GameLoaded, new GameEvent());
            
            return true;
        }
        catch (Exception ex)
        {
            Logger.Instance.Error("GameEngine", $"Error loading game: {ex.Message}");
            return false;
        }
    }

    /// <summary>
    /// Quick save the current game
    /// </summary>
    public bool QuickSave()
    {
        return SaveGame("quicksave");
    }

    /// <summary>
    /// Quick load the most recent save
    /// </summary>
    public bool QuickLoad()
    {
        return LoadGame("quicksave");
    }

    /// <summary>
    /// Get list of available save games
    /// </summary>
    public List<string> GetSaveGames()
    {
        var saves = SaveGameManager.Instance.ListSaveGames();
        return saves.Select(s => s.SaveName).ToList();
    }

    /// <summary>
    /// Get detailed information about available save games
    /// </summary>
    public List<SaveGameInfo> GetSaveGameInfo()
    {
        return SaveGameManager.Instance.ListSaveGames();
    }

    /// <summary>
    /// Helper method to serialize a component if it exists
    /// </summary>
    private void SerializeComponent<T>(Entity entity, EntityData entityData) where T : class, IComponent, ISerializable
    {
        if (EntityManager.HasComponent<T>(entity.Id))
        {
            var component = EntityManager.GetComponent<T>(entity.Id);
            if (component != null)
            {
                var componentData = new ComponentData
                {
                    ComponentType = typeof(T).FullName ?? typeof(T).Name,
                    Data = component.Serialize()
                };
                entityData.Components.Add(componentData);
            }
        }
    }

    /// <summary>
    /// Helper method to deserialize a component
    /// </summary>
    private void DeserializeComponent(Guid entityId, ComponentData componentData)
    {
        try
        {
            switch (componentData.ComponentType)
            {
                case "AvorionLike.Core.Physics.PhysicsComponent":
                    var physicsComponent = new PhysicsComponent();
                    physicsComponent.Deserialize(componentData.Data);
                    EntityManager.AddComponent(entityId, physicsComponent);
                    break;

                case "AvorionLike.Core.Voxel.VoxelStructureComponent":
                    var voxelComponent = new VoxelStructureComponent();
                    voxelComponent.Deserialize(componentData.Data);
                    EntityManager.AddComponent(entityId, voxelComponent);
                    break;

                case "AvorionLike.Core.Resources.InventoryComponent":
                    var inventoryComponent = new InventoryComponent();
                    inventoryComponent.Deserialize(componentData.Data);
                    EntityManager.AddComponent(entityId, inventoryComponent);
                    break;

                case "AvorionLike.Core.RPG.ProgressionComponent":
                    var progressionComponent = new ProgressionComponent();
                    progressionComponent.Deserialize(componentData.Data);
                    EntityManager.AddComponent(entityId, progressionComponent);
                    break;

                case "AvorionLike.Core.RPG.FactionComponent":
                    var factionComponent = new FactionComponent();
                    factionComponent.Deserialize(componentData.Data);
                    EntityManager.AddComponent(entityId, factionComponent);
                    break;

                default:
                    Logger.Instance.Warning("GameEngine", $"Unknown component type: {componentData.ComponentType}");
                    break;
            }
        }
        catch (Exception ex)
        {
            Logger.Instance.Error("GameEngine", $"Error deserializing component {componentData.ComponentType}: {ex.Message}");
        }
    }
}

/// <summary>
/// Engine statistics data
/// </summary>
public class EngineStatistics
{
    public int TotalEntities { get; set; }
    public int TotalComponents { get; set; }
    public bool IsServerRunning { get; set; }
    public bool PhysicsEnabled { get; set; }
}
