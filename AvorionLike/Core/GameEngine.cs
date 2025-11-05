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
    
    // New systems
    public CombatSystem CombatSystem { get; private set; } = null!;
    public MiningSystem MiningSystem { get; private set; } = null!;
    public FleetManagementSystem FleetManagementSystem { get; private set; } = null!;
    public NavigationSystem NavigationSystem { get; private set; } = null!;
    public BuildSystem BuildSystem { get; private set; } = null!;
    public EconomySystem EconomySystem { get; private set; } = null!;
    
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
                "AvorionLike",
                "Logs"
            );
            Logger.Instance.EnableFileLogging(logPath);
        }

        // Set log level from configuration
        var logLevel = Enum.TryParse<LogLevel>(config.Development.LogLevel, out var level) 
            ? level 
            : LogLevel.Info;
        Logger.Instance.SetMinimumLevel(logLevel);

        Logger.Instance.Info("GameEngine", "Initializing AvorionLike Game Engine...");

        // Initialize ECS
        EntityManager = new EntityManager();
        Logger.Instance.Info("GameEngine", "EntityManager initialized");

        // Initialize systems
        PhysicsSystem = new PhysicsSystem(EntityManager);
        ScriptingEngine = new ScriptingEngine();
        GalaxyGenerator = new GalaxyGenerator(_galaxySeed);
        CraftingSystem = new CraftingSystem();
        LootSystem = new LootSystem();
        TradingSystem = new TradingSystem();
        CombatSystem = new CombatSystem(EntityManager);
        MiningSystem = new MiningSystem(EntityManager);
        FleetManagementSystem = new FleetManagementSystem(EntityManager);
        NavigationSystem = new NavigationSystem(EntityManager);
        BuildSystem = new BuildSystem(EntityManager);
        EconomySystem = new EconomySystem(EntityManager);
        Logger.Instance.Info("GameEngine", "All systems initialized");

        // Register systems with entity manager
        EntityManager.RegisterSystem(PhysicsSystem);
        EntityManager.RegisterSystem(CombatSystem);
        EntityManager.RegisterSystem(MiningSystem);
        EntityManager.RegisterSystem(FleetManagementSystem);
        EntityManager.RegisterSystem(NavigationSystem);
        EntityManager.RegisterSystem(BuildSystem);
        EntityManager.RegisterSystem(EconomySystem);

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
        return new EngineStatistics
        {
            TotalEntities = EntityManager.GetAllEntities().Count(),
            TotalComponents = EntityManager.GetAllComponents<IComponent>().Count(),
            IsServerRunning = GameServer?.IsRunning ?? false,
            PhysicsEnabled = PhysicsSystem.IsEnabled
        };
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
