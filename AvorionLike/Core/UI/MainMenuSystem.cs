using System;
using System.Collections.Generic;
using System.IO;
using ImGuiNET;
using System.Numerics;
using AvorionLike.Core.Configuration;
using AvorionLike.Core.Persistence;

namespace AvorionLike.Core.UI;

/// <summary>
/// Main Menu System with options for New Game, Load Game, Multiplayer (Host/Join), and Settings
/// Provides extensive customization for galaxy generation and game parameters
/// </summary>
public class MainMenuSystem
{
    private readonly GameEngine _gameEngine;
    
    // Menu state
    private MenuState _currentState = MenuState.MainMenu;
    private bool _showMenu = true;
    
    // New Game state
    private NewGameSettings _newGameSettings = new();
    private NewGamePage _currentNewGamePage = NewGamePage.Presets;
    private string _selectedPreset = "normal";
    
    // Save/Load state
    private List<SaveGameInfo> _availableSaves = new();
    private int _selectedSaveIndex = 0;
    private bool _showDeleteConfirmation = false;
    private int _deleteConfirmIndex = -1;
    
    // Multiplayer state
    private string _serverName = "My Server";
    private string _serverAddress = "127.0.0.1";
    private int _serverPort = 27015;
    private int _maxPlayers = 16;
    private string _playerName = "Player";
    private List<string> _serverList = new();
    private int _selectedServerIndex = 0;
    
    // Settings state
    private float _masterVolume = 1.0f;
    private float _musicVolume = 0.8f;
    private float _sfxVolume = 0.9f;
    private int _targetFPS = 60;
    private bool _vsync = true;
    private int _selectedResolution = 0;
    private readonly string[] _resolutions = new[] { "1280x720", "1920x1080", "2560x1440", "3840x2160" };
    private float _mouseSensitivity = 0.5f;
    private float _gameplayDifficulty = 1.0f;
    private bool _showFPS = true;
    private bool _autoSave = true;
    private int _autoSaveInterval = 5;
    
    // Callbacks
    private Action<NewGameSettings>? _onNewGameStart;
    private Action<string>? _onLoadGame;
    private Action<string, int, int>? _onHostMultiplayer;
    private Action<string, int, string>? _onJoinMultiplayer;
    
    public bool IsMenuVisible => _showMenu;
    
    public enum MenuState
    {
        MainMenu,
        NewGame,
        LoadGame,
        MultiplayerHost,
        MultiplayerJoin,
        Settings,
        Quit
    }
    
    public enum NewGamePage
    {
        Presets,
        Galaxy,
        Sectors,
        Factions,
        AI,
        Starting,
        Summary
    }
    
    public MainMenuSystem(GameEngine gameEngine)
    {
        _gameEngine = gameEngine;
        LoadAvailableSaves();
        
        // Load player name from config
        _playerName = ConfigurationManager.Instance.Config.Gameplay.PlayerName;
        
        // Load settings from configuration
        LoadSettingsFromConfig();
    }
    
    /// <summary>
    /// Load current settings from configuration manager
    /// </summary>
    private void LoadSettingsFromConfig()
    {
        var config = ConfigurationManager.Instance.Config;
        
        // Graphics settings
        _vsync = config.Graphics.VSync;
        _targetFPS = config.Graphics.TargetFrameRate;
        _showFPS = config.Development.ShowDebugOverlay;
        
        // Determine resolution index
        string currentRes = $"{config.Graphics.ResolutionWidth}x{config.Graphics.ResolutionHeight}";
        for (int i = 0; i < _resolutions.Length; i++)
        {
            if (_resolutions[i] == currentRes)
            {
                _selectedResolution = i;
                break;
            }
        }
        
        // Audio settings
        _masterVolume = config.Audio.MasterVolume;
        _musicVolume = config.Audio.MusicVolume;
        _sfxVolume = config.Audio.SfxVolume;
        
        // Gameplay settings
        _autoSave = config.Gameplay.EnableAutoSave;
        _autoSaveInterval = config.Gameplay.AutoSaveIntervalSeconds / 60;
        _gameplayDifficulty = config.Gameplay.Difficulty switch
        {
            0 => 0.5f,
            1 => 1.0f,
            2 => 1.5f,
            _ => 1.0f
        };
    }
    
    /// <summary>
    /// Apply settings to configuration
    /// </summary>
    private void ApplySettings()
    {
        Console.WriteLine("Applying settings...");
        
        var config = ConfigurationManager.Instance.Config;
        
        // Parse selected resolution with error handling
        var resParts = _resolutions[_selectedResolution].Split('x');
        if (resParts.Length == 2 && 
            int.TryParse(resParts[0], out int width) && 
            int.TryParse(resParts[1], out int height))
        {
            config.Graphics.ResolutionWidth = width;
            config.Graphics.ResolutionHeight = height;
        }
        
        // Apply graphics settings
        config.Graphics.VSync = _vsync;
        config.Graphics.TargetFrameRate = _targetFPS;
        config.Development.ShowDebugOverlay = _showFPS;
        
        // Apply audio settings
        config.Audio.MasterVolume = _masterVolume;
        config.Audio.MusicVolume = _musicVolume;
        config.Audio.SfxVolume = _sfxVolume;
        
        // Apply gameplay settings
        config.Gameplay.EnableAutoSave = _autoSave;
        config.Gameplay.AutoSaveIntervalSeconds = _autoSaveInterval * 60;
        
        config.Gameplay.Difficulty = _gameplayDifficulty switch
        {
            <= 0.75f => 0,
            <= 1.25f => 1,
            <= 1.75f => 2,
            _ => 2
        };
        
        // Save configuration to disk
        ConfigurationManager.Instance.SaveConfiguration();
        ConfigurationManager.Instance.ApplyConfiguration();
        
        Console.WriteLine("Settings applied and saved.");
    }
    
    /// <summary>
    /// Set callbacks for menu actions
    /// </summary>
    public void SetCallbacks(
        Action<NewGameSettings>? onNewGameStart = null,
        Action<string>? onLoadGame = null,
        Action<string, int, int>? onHostMultiplayer = null,
        Action<string, int, string>? onJoinMultiplayer = null)
    {
        _onNewGameStart = onNewGameStart;
        _onLoadGame = onLoadGame;
        _onHostMultiplayer = onHostMultiplayer;
        _onJoinMultiplayer = onJoinMultiplayer;
    }
    
    public void Show()
    {
        _showMenu = true;
        _currentState = MenuState.MainMenu;
    }
    
    public void Hide()
    {
        _showMenu = false;
    }
    
    public void Render()
    {
        if (!_showMenu) return;
        
        switch (_currentState)
        {
            case MenuState.MainMenu:
                RenderMainMenu();
                break;
            case MenuState.NewGame:
                RenderNewGameMenu();
                break;
            case MenuState.LoadGame:
                RenderLoadGameMenu();
                break;
            case MenuState.MultiplayerHost:
                RenderHostMultiplayerMenu();
                break;
            case MenuState.MultiplayerJoin:
                RenderJoinMultiplayerMenu();
                break;
            case MenuState.Settings:
                RenderSettingsMenu();
                break;
        }
    }
    
    private void RenderMainMenu()
    {
        var io = ImGui.GetIO();
        ImGui.SetNextWindowPos(new Vector2(io.DisplaySize.X * 0.5f, io.DisplaySize.Y * 0.5f), ImGuiCond.Always, new Vector2(0.5f, 0.5f));
        ImGui.SetNextWindowSize(new Vector2(500, 600), ImGuiCond.FirstUseEver);
        
        ImGuiWindowFlags flags = ImGuiWindowFlags.NoResize | ImGuiWindowFlags.NoCollapse | ImGuiWindowFlags.NoMove;
        
        if (ImGui.Begin("Codename: Subspace - Main Menu", flags))
        {
            ImGui.PushStyleVar(ImGuiStyleVar.FramePadding, new Vector2(10, 10));
            
            // Title
            ImGui.Dummy(new Vector2(0, 20));
            var titleText = "CODENAME: SUBSPACE";
            var titleSize = ImGui.CalcTextSize(titleText);
            ImGui.SetCursorPosX((ImGui.GetWindowWidth() - titleSize.X) * 0.5f);
            ImGui.TextColored(new Vector4(0.3f, 0.7f, 1.0f, 1.0f), titleText);
            
            ImGui.Dummy(new Vector2(0, 10));
            ImGui.Separator();
            ImGui.Dummy(new Vector2(0, 30));
            
            // Menu buttons
            Vector2 buttonSize = new Vector2(ImGui.GetWindowWidth() - 80, 50);
            
            ImGui.SetCursorPosX(40);
            if (ImGui.Button("New Game", buttonSize))
            {
                _currentState = MenuState.NewGame;
                _newGameSettings = new NewGameSettings();
                _currentNewGamePage = NewGamePage.Presets;
            }
            
            ImGui.Dummy(new Vector2(0, 10));
            
            ImGui.SetCursorPosX(40);
            if (ImGui.Button("Load Game", buttonSize))
            {
                _currentState = MenuState.LoadGame;
                LoadAvailableSaves();
            }
            
            ImGui.Dummy(new Vector2(0, 10));
            
            ImGui.SetCursorPosX(40);
            if (ImGui.Button("Host Multiplayer", buttonSize))
            {
                _currentState = MenuState.MultiplayerHost;
            }
            
            ImGui.Dummy(new Vector2(0, 10));
            
            ImGui.SetCursorPosX(40);
            if (ImGui.Button("Join Multiplayer", buttonSize))
            {
                _currentState = MenuState.MultiplayerJoin;
                RefreshServerList();
            }
            
            ImGui.Dummy(new Vector2(0, 10));
            
            ImGui.SetCursorPosX(40);
            if (ImGui.Button("Settings", buttonSize))
            {
                _currentState = MenuState.Settings;
            }
            
            ImGui.Dummy(new Vector2(0, 30));
            ImGui.Separator();
            ImGui.Dummy(new Vector2(0, 10));
            
            ImGui.SetCursorPosX(40);
            if (ImGui.Button("Quit", buttonSize))
            {
                _currentState = MenuState.Quit;
                Environment.Exit(0);
            }
            
            ImGui.PopStyleVar();
        }
        ImGui.End();
    }
    
    private void RenderNewGameMenu()
    {
        var io = ImGui.GetIO();
        ImGui.SetNextWindowPos(new Vector2(io.DisplaySize.X * 0.5f, io.DisplaySize.Y * 0.5f), ImGuiCond.Always, new Vector2(0.5f, 0.5f));
        ImGui.SetNextWindowSize(new Vector2(900, 700), ImGuiCond.FirstUseEver);
        
        if (ImGui.Begin("New Game Configuration", ImGuiWindowFlags.NoCollapse))
        {
            // Tab bar for different configuration pages
            if (ImGui.BeginTabBar("NewGameTabs"))
            {
                if (ImGui.BeginTabItem("Presets"))
                {
                    _currentNewGamePage = NewGamePage.Presets;
                    RenderPresetsPage();
                    ImGui.EndTabItem();
                }
                
                if (ImGui.BeginTabItem("Galaxy"))
                {
                    _currentNewGamePage = NewGamePage.Galaxy;
                    RenderGalaxyPage();
                    ImGui.EndTabItem();
                }
                
                if (ImGui.BeginTabItem("Sectors"))
                {
                    _currentNewGamePage = NewGamePage.Sectors;
                    RenderSectorsPage();
                    ImGui.EndTabItem();
                }
                
                if (ImGui.BeginTabItem("Factions"))
                {
                    _currentNewGamePage = NewGamePage.Factions;
                    RenderFactionsPage();
                    ImGui.EndTabItem();
                }
                
                if (ImGui.BeginTabItem("AI & Difficulty"))
                {
                    _currentNewGamePage = NewGamePage.AI;
                    RenderAIPage();
                    ImGui.EndTabItem();
                }
                
                if (ImGui.BeginTabItem("Starting Conditions"))
                {
                    _currentNewGamePage = NewGamePage.Starting;
                    RenderStartingPage();
                    ImGui.EndTabItem();
                }
                
                if (ImGui.BeginTabItem("Summary"))
                {
                    _currentNewGamePage = NewGamePage.Summary;
                    RenderSummaryPage();
                    ImGui.EndTabItem();
                }
                
                ImGui.EndTabBar();
            }
            
            ImGui.Dummy(new Vector2(0, 20));
            ImGui.Separator();
            ImGui.Dummy(new Vector2(0, 10));
            
            // Bottom buttons
            Vector2 buttonSize = new Vector2(150, 40);
            float totalWidth = buttonSize.X * 2 + 20;
            ImGui.SetCursorPosX((ImGui.GetWindowWidth() - totalWidth) * 0.5f);
            
            if (ImGui.Button("Start Game", buttonSize))
            {
                _newGameSettings.Validate();
                _onNewGameStart?.Invoke(_newGameSettings);
                _showMenu = false;
            }
            
            ImGui.SameLine(0, 20);
            
            if (ImGui.Button("Back", buttonSize))
            {
                _currentState = MenuState.MainMenu;
            }
        }
        ImGui.End();
    }
    
    private void RenderPresetsPage()
    {
        ImGui.Text("Quick Start Presets");
        ImGui.Separator();
        ImGui.Dummy(new Vector2(0, 10));
        
        ImGui.TextWrapped("Choose a preset to quickly configure your game, or customize individual settings in other tabs.");
        ImGui.Dummy(new Vector2(0, 20));
        
        if (ImGui.RadioButton("Easy - Relaxed gameplay with abundant resources", _selectedPreset == "easy"))
        {
            _selectedPreset = "easy";
            _newGameSettings = NewGameSettings.CreatePreset("easy");
        }
        ImGui.Dummy(new Vector2(0, 10));
        
        if (ImGui.RadioButton("Normal - Balanced gameplay experience", _selectedPreset == "normal"))
        {
            _selectedPreset = "normal";
            _newGameSettings = NewGameSettings.CreatePreset("normal");
        }
        ImGui.Dummy(new Vector2(0, 10));
        
        if (ImGui.RadioButton("Hard - Challenging with scarce resources", _selectedPreset == "hard"))
        {
            _selectedPreset = "hard";
            _newGameSettings = NewGameSettings.CreatePreset("hard");
        }
        ImGui.Dummy(new Vector2(0, 10));
        
        if (ImGui.RadioButton("Ironman - Hardcore mode with permadeath", _selectedPreset == "ironman"))
        {
            _selectedPreset = "ironman";
            _newGameSettings = NewGameSettings.CreatePreset("ironman");
        }
        ImGui.Dummy(new Vector2(0, 10));
        
        if (ImGui.RadioButton("Sandbox - Creative mode with unlimited resources", _selectedPreset == "sandbox"))
        {
            _selectedPreset = "sandbox";
            _newGameSettings = NewGameSettings.CreatePreset("sandbox");
        }
        ImGui.Dummy(new Vector2(0, 10));
        
        if (ImGui.RadioButton("Dense Galaxy - Many sectors and factions", _selectedPreset == "dense"))
        {
            _selectedPreset = "dense";
            _newGameSettings = NewGameSettings.CreatePreset("dense");
        }
        ImGui.Dummy(new Vector2(0, 10));
        
        if (ImGui.RadioButton("Sparse Galaxy - Fewer sectors, more exploration", _selectedPreset == "sparse"))
        {
            _selectedPreset = "sparse";
            _newGameSettings = NewGameSettings.CreatePreset("sparse");
        }
    }
    
    private void RenderGalaxyPage()
    {
        ImGui.Text("Galaxy Generation Settings");
        ImGui.Separator();
        ImGui.Dummy(new Vector2(0, 10));
        
        ImGui.Text("Galaxy Seed:");
        ImGui.SameLine();
        ImGui.SetNextItemWidth(200);
        int seed = _newGameSettings.GalaxySeed;
        if (ImGui.InputInt("##seed", ref seed))
        {
            _newGameSettings.GalaxySeed = seed;
        }
        ImGui.SameLine();
        if (ImGui.Button("Random##seed"))
        {
            _newGameSettings.GalaxySeed = -1;
        }
        
        ImGui.Dummy(new Vector2(0, 10));
        
        ImGui.Text($"Galaxy Radius: {_newGameSettings.GalaxyRadius} sectors");
        ImGui.SetNextItemWidth(400);
        int radius = _newGameSettings.GalaxyRadius;
        if (ImGui.SliderInt("##radius", ref radius, 100, 1000))
        {
            _newGameSettings.GalaxyRadius = radius;
        }
        
        ImGui.Dummy(new Vector2(0, 10));
        
        ImGui.Text($"Galaxy Density: {_newGameSettings.GalaxyDensity:F1}x");
        ImGui.SetNextItemWidth(400);
        float density = _newGameSettings.GalaxyDensity;
        if (ImGui.SliderFloat("##density", ref density, 0.1f, 5.0f))
        {
            _newGameSettings.GalaxyDensity = density;
        }
        ImGui.TextWrapped("Higher density means more populated sectors and resources");
        
        ImGui.Dummy(new Vector2(0, 10));
        
        ImGui.Text($"Total Sectors: {_newGameSettings.TotalSectors:N0}");
        ImGui.SetNextItemWidth(400);
        int sectors = _newGameSettings.TotalSectors;
        if (ImGui.SliderInt("##sectors", ref sectors, 1000, 100000))
        {
            _newGameSettings.TotalSectors = sectors;
        }
    }
    
    private void RenderSectorsPage()
    {
        ImGui.Text("Sector & Asteroid Generation");
        ImGui.Separator();
        ImGui.Dummy(new Vector2(0, 10));
        
        ImGui.Text($"Asteroids Per Belt: {_newGameSettings.AsteroidsPerBelt}");
        ImGui.SetNextItemWidth(400);
        int count = _newGameSettings.AsteroidsPerBelt;
        if (ImGui.SliderInt("##asteroidcount", ref count, 10, 200))
        {
            _newGameSettings.AsteroidsPerBelt = count;
        }
        
        ImGui.Dummy(new Vector2(0, 10));
        
        ImGui.Text($"Resource Richness: {_newGameSettings.AsteroidResourceRichness:F1}x");
        ImGui.SetNextItemWidth(400);
        float richness = _newGameSettings.AsteroidResourceRichness;
        if (ImGui.SliderFloat("##richness", ref richness, 0.1f, 10.0f))
        {
            _newGameSettings.AsteroidResourceRichness = richness;
        }
        ImGui.TextWrapped("Affects how many resources each asteroid contains");
        
        ImGui.Dummy(new Vector2(0, 10));
        
        ImGui.Text($"Asteroid Size Variation: {_newGameSettings.AsteroidSizeVariation:F1}x");
        ImGui.SetNextItemWidth(400);
        float sizeVar = _newGameSettings.AsteroidSizeVariation;
        if (ImGui.SliderFloat("##sizevar", ref sizeVar, 0.1f, 3.0f))
        {
            _newGameSettings.AsteroidSizeVariation = sizeVar;
        }
        
        ImGui.Dummy(new Vector2(0, 10));
        
        ImGui.Text($"Min Asteroids Per Sector: {_newGameSettings.MinAsteroidsPerSector}");
        ImGui.SetNextItemWidth(400);
        int min = _newGameSettings.MinAsteroidsPerSector;
        if (ImGui.SliderInt("##minast", ref min, 0, 50))
        {
            _newGameSettings.MinAsteroidsPerSector = min;
        }
        
        ImGui.Dummy(new Vector2(0, 10));
        
        ImGui.Text($"Max Asteroids Per Sector: {_newGameSettings.MaxAsteroidsPerSector}");
        ImGui.SetNextItemWidth(400);
        int max = _newGameSettings.MaxAsteroidsPerSector;
        if (ImGui.SliderInt("##maxast", ref max, _newGameSettings.MinAsteroidsPerSector, 200))
        {
            _newGameSettings.MaxAsteroidsPerSector = max;
        }
        
        ImGui.Dummy(new Vector2(0, 20));
        ImGui.Separator();
        ImGui.Dummy(new Vector2(0, 10));
        
        ImGui.Text("Special Features");
        bool massiveAst = _newGameSettings.EnableMassiveAsteroids;
        bool stations = _newGameSettings.EnableSpaceStations;
        bool anomalies = _newGameSettings.EnableAnomalies;
        bool wormholes = _newGameSettings.EnableWormholes;
        
        if (ImGui.Checkbox("Enable Massive Claimable Asteroids", ref massiveAst))
            _newGameSettings.EnableMassiveAsteroids = massiveAst;
        if (ImGui.Checkbox("Enable Space Stations", ref stations))
            _newGameSettings.EnableSpaceStations = stations;
        if (ImGui.Checkbox("Enable Anomalies & Events", ref anomalies))
            _newGameSettings.EnableAnomalies = anomalies;
        if (ImGui.Checkbox("Enable Wormholes", ref wormholes))
            _newGameSettings.EnableWormholes = wormholes;
    }
    
    private void RenderFactionsPage()
    {
        ImGui.Text("Faction Configuration");
        ImGui.Separator();
        ImGui.Dummy(new Vector2(0, 10));
        
        ImGui.Text($"Number of AI Factions: {_newGameSettings.FactionCount}");
        ImGui.SetNextItemWidth(400);
        int factionCount = _newGameSettings.FactionCount;
        if (ImGui.SliderInt("##factions", ref factionCount, 1, 50))
        {
            _newGameSettings.FactionCount = factionCount;
        }
        
        ImGui.Dummy(new Vector2(0, 10));
        
        ImGui.Text($"Faction War Frequency: {_newGameSettings.FactionWarFrequency:F1}x");
        ImGui.SetNextItemWidth(400);
        float warFreq = _newGameSettings.FactionWarFrequency;
        if (ImGui.SliderFloat("##warfreq", ref warFreq, 0.0f, 3.0f))
        {
            _newGameSettings.FactionWarFrequency = warFreq;
        }
        ImGui.TextWrapped("How often factions declare war on each other");
        
        ImGui.Dummy(new Vector2(0, 20));
        ImGui.Separator();
        ImGui.Dummy(new Vector2(0, 10));
        
        ImGui.Text("Pirates");
        bool enablePirates = _newGameSettings.EnablePirates;
        if (ImGui.Checkbox("Enable Pirates", ref enablePirates))
        {
            _newGameSettings.EnablePirates = enablePirates;
        }
        
        if (_newGameSettings.EnablePirates)
        {
            ImGui.Dummy(new Vector2(0, 10));
            ImGui.Text("Pirate Aggression:");
            int pirateAggr = _newGameSettings.PirateAggression;
            if (ImGui.RadioButton("Low##pirate", ref pirateAggr, 0))
                _newGameSettings.PirateAggression = pirateAggr;
            ImGui.SameLine();
            if (ImGui.RadioButton("Normal##pirate", ref pirateAggr, 1))
                _newGameSettings.PirateAggression = pirateAggr;
            ImGui.SameLine();
            if (ImGui.RadioButton("High##pirate", ref pirateAggr, 2))
                _newGameSettings.PirateAggression = pirateAggr;
        }
    }
    
    private void RenderAIPage()
    {
        ImGui.Text("AI & Difficulty Settings");
        ImGui.Separator();
        ImGui.Dummy(new Vector2(0, 10));
        
        ImGui.Text("AI Difficulty:");
        int aiDiff = _newGameSettings.AIDifficulty;
        if (ImGui.RadioButton("Easy##ai", ref aiDiff, 0))
            _newGameSettings.AIDifficulty = aiDiff;
        ImGui.SameLine();
        if (ImGui.RadioButton("Normal##ai", ref aiDiff, 1))
            _newGameSettings.AIDifficulty = aiDiff;
        ImGui.SameLine();
        if (ImGui.RadioButton("Hard##ai", ref aiDiff, 2))
            _newGameSettings.AIDifficulty = aiDiff;
        ImGui.SameLine();
        if (ImGui.RadioButton("Very Hard##ai", ref aiDiff, 3))
            _newGameSettings.AIDifficulty = aiDiff;
        
        ImGui.Dummy(new Vector2(0, 10));
        
        ImGui.Text($"AI Competence: {_newGameSettings.AICompetence:F1}x");
        ImGui.SetNextItemWidth(400);
        float competence = _newGameSettings.AICompetence;
        if (ImGui.SliderFloat("##aicomp", ref competence, 0.1f, 5.0f))
        {
            _newGameSettings.AICompetence = competence;
        }
        ImGui.TextWrapped("Affects AI decision-making quality and strategic planning");
        
        ImGui.Dummy(new Vector2(0, 10));
        
        ImGui.Text($"AI Reaction Speed: {_newGameSettings.AIReactionSpeed:F1}x");
        ImGui.SetNextItemWidth(400);
        float reaction = _newGameSettings.AIReactionSpeed;
        if (ImGui.SliderFloat("##aireact", ref reaction, 0.1f, 3.0f))
        {
            _newGameSettings.AIReactionSpeed = reaction;
        }
        
        ImGui.Dummy(new Vector2(0, 10));
        
        ImGui.Text($"AI Economic Advantage: {_newGameSettings.AIEconomicCheat:F1}x");
        ImGui.SetNextItemWidth(400);
        float econ = _newGameSettings.AIEconomicCheat;
        if (ImGui.SliderFloat("##aiecon", ref econ, 0.1f, 10.0f))
        {
            _newGameSettings.AIEconomicCheat = econ;
        }
        ImGui.TextWrapped("Resource generation multiplier for AI factions");
        
        ImGui.Dummy(new Vector2(0, 20));
        ImGui.Separator();
        ImGui.Dummy(new Vector2(0, 10));
        
        ImGui.Text("AI Behaviors");
        bool aiExpansion = _newGameSettings.EnableAIExpansion;
        bool aiTrading = _newGameSettings.EnableAITrading;
        bool aiMining = _newGameSettings.EnableAIMining;
        
        if (ImGui.Checkbox("Enable AI Expansion", ref aiExpansion))
            _newGameSettings.EnableAIExpansion = aiExpansion;
        if (ImGui.Checkbox("Enable AI Trading", ref aiTrading))
            _newGameSettings.EnableAITrading = aiTrading;
        if (ImGui.Checkbox("Enable AI Mining", ref aiMining))
            _newGameSettings.EnableAIMining = aiMining;
        
        ImGui.Dummy(new Vector2(0, 20));
        ImGui.Separator();
        ImGui.Dummy(new Vector2(0, 10));
        
        ImGui.Text("Player Difficulty:");
        int playerDiff = _newGameSettings.PlayerDifficulty;
        if (ImGui.RadioButton("Easy##player", ref playerDiff, 0))
            _newGameSettings.PlayerDifficulty = playerDiff;
        ImGui.SameLine();
        if (ImGui.RadioButton("Normal##player", ref playerDiff, 1))
            _newGameSettings.PlayerDifficulty = playerDiff;
        ImGui.SameLine();
        if (ImGui.RadioButton("Hard##player", ref playerDiff, 2))
            _newGameSettings.PlayerDifficulty = playerDiff;
        
        ImGui.Dummy(new Vector2(0, 10));
        
        ImGui.Text($"Enemy Strength: {_newGameSettings.EnemyStrengthMultiplier:F1}x");
        ImGui.SetNextItemWidth(400);
        float enemyStr = _newGameSettings.EnemyStrengthMultiplier;
        if (ImGui.SliderFloat("##enemystr", ref enemyStr, 0.1f, 3.0f))
        {
            _newGameSettings.EnemyStrengthMultiplier = enemyStr;
        }
        
        ImGui.Dummy(new Vector2(0, 10));
        
        bool permaDeath = _newGameSettings.PermaDeath;
        bool ironman = _newGameSettings.IronmanMode;
        
        if (ImGui.Checkbox("Permadeath (single life)", ref permaDeath))
            _newGameSettings.PermaDeath = permaDeath;
        if (ImGui.Checkbox("Ironman Mode (can't reload saves)", ref ironman))
            _newGameSettings.IronmanMode = ironman;
    }
    
    private void RenderStartingPage()
    {
        ImGui.Text("Starting Conditions");
        ImGui.Separator();
        ImGui.Dummy(new Vector2(0, 10));
        
        ImGui.Text("Player Name:");
        ImGui.SetNextItemWidth(300);
        string playerName = _newGameSettings.PlayerName;
        if (ImGui.InputText("##playername", ref playerName, 50))
        {
            _newGameSettings.PlayerName = playerName;
        }
        
        ImGui.Dummy(new Vector2(0, 10));
        
        ImGui.Text("Starting Region:");
        bool isRim = _newGameSettings.StartingRegionType == "Rim";
        bool isMid = _newGameSettings.StartingRegionType == "Mid";
        bool isCore = _newGameSettings.StartingRegionType == "Core";
        
        if (ImGui.RadioButton("Galaxy Rim (Iron Tier)", isRim))
            _newGameSettings.StartingRegionType = "Rim";
        ImGui.SameLine();
        if (ImGui.RadioButton("Mid-Galaxy (Titanium Tier)", isMid))
            _newGameSettings.StartingRegionType = "Mid";
        ImGui.SameLine();
        if (ImGui.RadioButton("Core (Avorion Tier)", isCore))
            _newGameSettings.StartingRegionType = "Core";
        
        ImGui.Dummy(new Vector2(0, 10));
        
        ImGui.Text($"Starting Credits: {_newGameSettings.StartingCredits:N0}");
        ImGui.SetNextItemWidth(400);
        int credits = _newGameSettings.StartingCredits;
        if (ImGui.SliderInt("##credits", ref credits, 0, 1000000))
        {
            _newGameSettings.StartingCredits = credits;
        }
        
        ImGui.Dummy(new Vector2(0, 10));
        
        ImGui.Text("Starting Ship Class:");
        bool isStarter = _newGameSettings.StartingShipClass == "Starter";
        bool isFighter = _newGameSettings.StartingShipClass == "Fighter";
        bool isMiner = _newGameSettings.StartingShipClass == "Miner";
        bool isTrader = _newGameSettings.StartingShipClass == "Trader";
        
        if (ImGui.RadioButton("Starter Pod", isStarter))
            _newGameSettings.StartingShipClass = "Starter";
        ImGui.SameLine();
        if (ImGui.RadioButton("Fighter", isFighter))
            _newGameSettings.StartingShipClass = "Fighter";
        ImGui.SameLine();
        if (ImGui.RadioButton("Miner", isMiner))
            _newGameSettings.StartingShipClass = "Miner";
        ImGui.SameLine();
        if (ImGui.RadioButton("Trader", isTrader))
            _newGameSettings.StartingShipClass = "Trader";
        
        ImGui.Dummy(new Vector2(0, 10));
        
        ImGui.Text($"Resource Gathering Speed: {_newGameSettings.ResourceGatheringMultiplier:F1}x");
        ImGui.SetNextItemWidth(400);
        float gathering = _newGameSettings.ResourceGatheringMultiplier;
        if (ImGui.SliderFloat("##gathering", ref gathering, 0.1f, 5.0f))
        {
            _newGameSettings.ResourceGatheringMultiplier = gathering;
        }
    }
    
    private void RenderSummaryPage()
    {
        ImGui.Text("Configuration Summary");
        ImGui.Separator();
        ImGui.Dummy(new Vector2(0, 10));
        
        ImGui.TextWrapped(_newGameSettings.GetSummary());
        
        ImGui.Dummy(new Vector2(0, 20));
        
        ImGui.TextColored(new Vector4(0.3f, 1.0f, 0.3f, 1.0f), "Review your settings above, then click 'Start Game' to begin!");
    }
    
    private void RenderLoadGameMenu()
    {
        var io = ImGui.GetIO();
        ImGui.SetNextWindowPos(new Vector2(io.DisplaySize.X * 0.5f, io.DisplaySize.Y * 0.5f), ImGuiCond.Always, new Vector2(0.5f, 0.5f));
        ImGui.SetNextWindowSize(new Vector2(700, 600), ImGuiCond.FirstUseEver);
        
        if (ImGui.Begin("Load Game", ImGuiWindowFlags.NoCollapse))
        {
            ImGui.Text("Saved Games");
            ImGui.Separator();
            ImGui.Dummy(new Vector2(0, 10));
            
            if (_availableSaves.Count == 0)
            {
                ImGui.TextWrapped("No saved games found. Start a new game to create your first save!");
            }
            else
            {
                // Save list
                if (ImGui.BeginChild("SaveList", new Vector2(0, 400), true))
                {
                    for (int i = 0; i < _availableSaves.Count; i++)
                    {
                        var save = _availableSaves[i];
                        bool isSelected = (_selectedSaveIndex == i);
                        
                        if (ImGui.Selectable($"{save.SaveName}##save{i}", isSelected, ImGuiSelectableFlags.None, new Vector2(0, 60)))
                        {
                            _selectedSaveIndex = i;
                        }
                        
                        // Show save details
                        if (ImGui.IsItemHovered())
                        {
                            ImGui.BeginTooltip();
                            ImGui.Text($"Saved: {save.SaveTime:yyyy-MM-dd HH:mm:ss}");
                            ImGui.Text($"Version: {save.Version}");
                            ImGui.EndTooltip();
                        }
                        
                        // Context menu for delete
                        if (ImGui.IsItemClicked(ImGuiMouseButton.Right))
                        {
                            _showDeleteConfirmation = true;
                            _deleteConfirmIndex = i;
                        }
                    }
                    ImGui.EndChild();
                }
            }
            
            ImGui.Dummy(new Vector2(0, 10));
            ImGui.Separator();
            ImGui.Dummy(new Vector2(0, 10));
            
            // Buttons
            Vector2 buttonSize = new Vector2(150, 40);
            
            if (_availableSaves.Count > 0 && _selectedSaveIndex >= 0 && _selectedSaveIndex < _availableSaves.Count)
            {
                if (ImGui.Button("Load", buttonSize))
                {
                    string saveName = _availableSaves[_selectedSaveIndex].FileName.Replace(".save", "");
                    _onLoadGame?.Invoke(saveName);
                    _showMenu = false;
                }
                
                ImGui.SameLine();
                
                if (ImGui.Button("Delete", buttonSize))
                {
                    _showDeleteConfirmation = true;
                    _deleteConfirmIndex = _selectedSaveIndex;
                }
            }
            
            ImGui.SameLine();
            
            if (ImGui.Button("Back", buttonSize))
            {
                _currentState = MenuState.MainMenu;
            }
            
            // Delete confirmation dialog
            if (_showDeleteConfirmation)
            {
                ImGui.OpenPopup("Delete Save?");
            }
            
            if (ImGui.BeginPopupModal("Delete Save?", ref _showDeleteConfirmation, ImGuiWindowFlags.AlwaysAutoResize))
            {
                if (_deleteConfirmIndex >= 0 && _deleteConfirmIndex < _availableSaves.Count)
                {
                    ImGui.Text($"Are you sure you want to delete '{_availableSaves[_deleteConfirmIndex].SaveName}'?");
                    ImGui.Text("This action cannot be undone!");
                    ImGui.Dummy(new Vector2(0, 10));
                    
                    if (ImGui.Button("Delete", new Vector2(120, 0)))
                    {
                        DeleteSave(_deleteConfirmIndex);
                        _showDeleteConfirmation = false;
                        _deleteConfirmIndex = -1;
                    }
                    
                    ImGui.SameLine();
                    
                    if (ImGui.Button("Cancel", new Vector2(120, 0)))
                    {
                        _showDeleteConfirmation = false;
                        _deleteConfirmIndex = -1;
                    }
                }
                
                ImGui.EndPopup();
            }
        }
        ImGui.End();
    }
    
    private void RenderHostMultiplayerMenu()
    {
        var io = ImGui.GetIO();
        ImGui.SetNextWindowPos(new Vector2(io.DisplaySize.X * 0.5f, io.DisplaySize.Y * 0.5f), ImGuiCond.Always, new Vector2(0.5f, 0.5f));
        ImGui.SetNextWindowSize(new Vector2(600, 500), ImGuiCond.FirstUseEver);
        
        if (ImGui.Begin("Host Multiplayer Server", ImGuiWindowFlags.NoCollapse))
        {
            ImGui.Text("Server Configuration");
            ImGui.Separator();
            ImGui.Dummy(new Vector2(0, 10));
            
            ImGui.Text("Server Name:");
            ImGui.SetNextItemWidth(400);
            ImGui.InputText("##servername", ref _serverName, 100);
            
            ImGui.Dummy(new Vector2(0, 10));
            
            ImGui.Text("Server Port:");
            ImGui.SetNextItemWidth(200);
            ImGui.InputInt("##port", ref _serverPort);
            
            ImGui.Dummy(new Vector2(0, 10));
            
            ImGui.Text("Max Players:");
            ImGui.SetNextItemWidth(200);
            ImGui.SliderInt("##maxplayers", ref _maxPlayers, 2, 50);
            
            ImGui.Dummy(new Vector2(0, 20));
            ImGui.Separator();
            ImGui.Dummy(new Vector2(0, 10));
            
            ImGui.TextWrapped("Use the New Game settings to configure the game world. All players will join your game world.");
            
            ImGui.Dummy(new Vector2(0, 20));
            
            // Buttons
            Vector2 buttonSize = new Vector2(150, 40);
            float totalWidth = buttonSize.X * 2 + 20;
            ImGui.SetCursorPosX((ImGui.GetWindowWidth() - totalWidth) * 0.5f);
            
            if (ImGui.Button("Start Server", buttonSize))
            {
                _onHostMultiplayer?.Invoke(_serverName, _serverPort, _maxPlayers);
                _currentState = MenuState.NewGame; // Configure new game settings
            }
            
            ImGui.SameLine(0, 20);
            
            if (ImGui.Button("Back", buttonSize))
            {
                _currentState = MenuState.MainMenu;
            }
        }
        ImGui.End();
    }
    
    private void RenderJoinMultiplayerMenu()
    {
        var io = ImGui.GetIO();
        ImGui.SetNextWindowPos(new Vector2(io.DisplaySize.X * 0.5f, io.DisplaySize.Y * 0.5f), ImGuiCond.Always, new Vector2(0.5f, 0.5f));
        ImGui.SetNextWindowSize(new Vector2(700, 600), ImGuiCond.FirstUseEver);
        
        if (ImGui.Begin("Join Multiplayer Server", ImGuiWindowFlags.NoCollapse))
        {
            ImGui.Text("Available Servers");
            ImGui.Separator();
            ImGui.Dummy(new Vector2(0, 10));
            
            if (ImGui.Button("Refresh"))
            {
                RefreshServerList();
            }
            
            ImGui.Dummy(new Vector2(0, 10));
            
            // Server list
            if (ImGui.BeginChild("ServerList", new Vector2(0, 300), true))
            {
                if (_serverList.Count == 0)
                {
                    ImGui.TextWrapped("No servers found. Enter a direct IP address below to connect.");
                }
                else
                {
                    for (int i = 0; i < _serverList.Count; i++)
                    {
                        bool isSelected = (_selectedServerIndex == i);
                        if (ImGui.Selectable(_serverList[i], isSelected))
                        {
                            _selectedServerIndex = i;
                        }
                    }
                }
                ImGui.EndChild();
            }
            
            ImGui.Dummy(new Vector2(0, 10));
            ImGui.Separator();
            ImGui.Dummy(new Vector2(0, 10));
            
            ImGui.Text("Direct Connection");
            ImGui.Text("Server Address:");
            ImGui.SetNextItemWidth(300);
            ImGui.InputText("##address", ref _serverAddress, 100);
            ImGui.SameLine();
            ImGui.Text("Port:");
            ImGui.SameLine();
            ImGui.SetNextItemWidth(100);
            ImGui.InputInt("##joinport", ref _serverPort);
            
            ImGui.Dummy(new Vector2(0, 10));
            
            ImGui.Text("Player Name:");
            ImGui.SetNextItemWidth(300);
            ImGui.InputText("##playernamemp", ref _playerName, 50);
            
            ImGui.Dummy(new Vector2(0, 20));
            
            // Buttons
            Vector2 buttonSize = new Vector2(150, 40);
            float totalWidth = buttonSize.X * 2 + 20;
            ImGui.SetCursorPosX((ImGui.GetWindowWidth() - totalWidth) * 0.5f);
            
            if (ImGui.Button("Connect", buttonSize))
            {
                _onJoinMultiplayer?.Invoke(_serverAddress, _serverPort, _playerName);
                _showMenu = false;
            }
            
            ImGui.SameLine(0, 20);
            
            if (ImGui.Button("Back", buttonSize))
            {
                _currentState = MenuState.MainMenu;
            }
        }
        ImGui.End();
    }
    
    private void RenderSettingsMenu()
    {
        var io = ImGui.GetIO();
        ImGui.SetNextWindowPos(new Vector2(io.DisplaySize.X * 0.5f, io.DisplaySize.Y * 0.5f), ImGuiCond.Always, new Vector2(0.5f, 0.5f));
        ImGui.SetNextWindowSize(new Vector2(650, 700), ImGuiCond.FirstUseEver);
        
        bool open = true;
        if (ImGui.Begin("Settings", ref open, ImGuiWindowFlags.NoCollapse))
        {
            if (ImGui.BeginTabBar("SettingsTabs"))
            {
                // Video Settings Tab
                if (ImGui.BeginTabItem("Video"))
                {
                    ImGui.Dummy(new Vector2(0, 10));
                    
                    ImGui.Text("Display Settings");
                    ImGui.Separator();
                    ImGui.Dummy(new Vector2(0, 5));
                    
                    ImGui.Text("Resolution:");
                    ImGui.SetNextItemWidth(200);
                    ImGui.Combo("##Resolution", ref _selectedResolution, _resolutions, _resolutions.Length);
                    
                    ImGui.Dummy(new Vector2(0, 10));
                    
                    ImGui.Checkbox("VSync", ref _vsync);
                    ImGui.Checkbox("Show FPS Counter", ref _showFPS);
                    
                    ImGui.Dummy(new Vector2(0, 10));
                    
                    ImGui.Text($"Target FPS: {_targetFPS}");
                    ImGui.SetNextItemWidth(200);
                    ImGui.SliderInt("##TargetFPS", ref _targetFPS, 30, 144);
                    
                    ImGui.Dummy(new Vector2(0, 20));
                    ImGui.Text("Graphics Quality");
                    ImGui.Separator();
                    ImGui.Dummy(new Vector2(0, 5));
                    
                    ImGui.TextWrapped("Additional rendering quality settings will be added in future updates.");
                    
                    ImGui.EndTabItem();
                }
                
                // Audio Settings Tab
                if (ImGui.BeginTabItem("Audio"))
                {
                    ImGui.Dummy(new Vector2(0, 10));
                    
                    ImGui.Text("Volume Controls");
                    ImGui.Separator();
                    ImGui.Dummy(new Vector2(0, 10));
                    
                    ImGui.Text($"Master Volume: {_masterVolume * 100:F0}%");
                    ImGui.SetNextItemWidth(250);
                    ImGui.SliderFloat("##MasterVolume", ref _masterVolume, 0.0f, 1.0f);
                    
                    ImGui.Dummy(new Vector2(0, 10));
                    
                    ImGui.Text($"Music Volume: {_musicVolume * 100:F0}%");
                    ImGui.SetNextItemWidth(250);
                    ImGui.SliderFloat("##MusicVolume", ref _musicVolume, 0.0f, 1.0f);
                    
                    ImGui.Dummy(new Vector2(0, 10));
                    
                    ImGui.Text($"SFX Volume: {_sfxVolume * 100:F0}%");
                    ImGui.SetNextItemWidth(250);
                    ImGui.SliderFloat("##SFXVolume", ref _sfxVolume, 0.0f, 1.0f);
                    
                    ImGui.Dummy(new Vector2(0, 20));
                    ImGui.Separator();
                    ImGui.Dummy(new Vector2(0, 10));
                    
                    ImGui.TextColored(new Vector4(0.5f, 0.7f, 1.0f, 1.0f), "Tip: Use mouse to drag the sliders!");
                    ImGui.TextWrapped("Audio playback will be implemented in future updates.");
                    
                    ImGui.EndTabItem();
                }
                
                // Gameplay Settings Tab
                if (ImGui.BeginTabItem("Gameplay"))
                {
                    ImGui.Dummy(new Vector2(0, 10));
                    
                    ImGui.Text("Difficulty");
                    ImGui.Separator();
                    ImGui.Dummy(new Vector2(0, 10));
                    
                    string difficultyLabel = _gameplayDifficulty switch
                    {
                        <= 0.75f => "Easy",
                        <= 1.25f => "Normal",
                        <= 1.75f => "Hard",
                        _ => "Very Hard"
                    };
                    
                    ImGui.Text($"Difficulty: {difficultyLabel} ({_gameplayDifficulty:F2}x)");
                    ImGui.SetNextItemWidth(250);
                    if (ImGui.SliderFloat("##Difficulty", ref _gameplayDifficulty, 0.5f, 2.0f))
                    {
                        if (Math.Abs(_gameplayDifficulty - 0.5f) < 0.1f) _gameplayDifficulty = 0.5f;
                        else if (Math.Abs(_gameplayDifficulty - 1.0f) < 0.1f) _gameplayDifficulty = 1.0f;
                        else if (Math.Abs(_gameplayDifficulty - 1.5f) < 0.1f) _gameplayDifficulty = 1.5f;
                        else if (Math.Abs(_gameplayDifficulty - 2.0f) < 0.1f) _gameplayDifficulty = 2.0f;
                    }
                    
                    ImGui.Dummy(new Vector2(0, 5));
                    ImGui.TextWrapped("Affects enemy strength, resource scarcity, and mission rewards.");
                    
                    ImGui.Dummy(new Vector2(0, 20));
                    ImGui.Text("Controls");
                    ImGui.Separator();
                    ImGui.Dummy(new Vector2(0, 10));
                    
                    ImGui.Text($"Mouse Sensitivity: {_mouseSensitivity:F2}");
                    ImGui.SetNextItemWidth(250);
                    ImGui.SliderFloat("##MouseSens", ref _mouseSensitivity, 0.1f, 2.0f);
                    
                    ImGui.Dummy(new Vector2(0, 20));
                    ImGui.Text("Auto-Save");
                    ImGui.Separator();
                    ImGui.Dummy(new Vector2(0, 10));
                    
                    ImGui.Checkbox("Enable Auto-Save", ref _autoSave);
                    
                    if (_autoSave)
                    {
                        ImGui.Text($"Auto-Save Interval: {_autoSaveInterval} minutes");
                        ImGui.SetNextItemWidth(200);
                        ImGui.SliderInt("##AutoSaveInterval", ref _autoSaveInterval, 1, 30);
                    }
                    
                    ImGui.EndTabItem();
                }
                
                // Controls Tab
                if (ImGui.BeginTabItem("Controls"))
                {
                    ImGui.Dummy(new Vector2(0, 10));
                    
                    ImGui.Text("Camera Controls:");
                    ImGui.BulletText("WASD - Move horizontally");
                    ImGui.BulletText("Space - Move up");
                    ImGui.BulletText("Shift - Move down");
                    ImGui.BulletText("Mouse - Look around (Free-look)");
                    
                    ImGui.Dummy(new Vector2(0, 10));
                    
                    ImGui.Text("UI Controls:");
                    ImGui.BulletText("ALT - Show mouse cursor (hold)");
                    ImGui.BulletText("ESC - Pause Menu (press again to close)");
                    ImGui.BulletText("F1 - Toggle Debug Info");
                    ImGui.BulletText("F2 - Toggle Entity List");
                    ImGui.BulletText("F3 - Toggle Resources");
                    ImGui.BulletText("M - Toggle Galaxy Map");
                    ImGui.BulletText("I - Toggle Inventory");
                    ImGui.BulletText("B - Toggle Ship Builder");
                    ImGui.BulletText("~ - Toggle Testing Console");
                    
                    ImGui.Dummy(new Vector2(0, 20));
                    ImGui.TextWrapped("Key remapping will be added in future updates.");
                    
                    ImGui.EndTabItem();
                }
                
                ImGui.EndTabBar();
            }
            
            ImGui.Dummy(new Vector2(0, 20));
            
            // Bottom buttons
            Vector2 buttonSize = new Vector2(150, 40);
            float buttonSpacing = 20;
            float totalWidth = buttonSize.X * 2 + buttonSpacing;
            ImGui.SetCursorPosX((ImGui.GetWindowWidth() - totalWidth) * 0.5f);
            
            if (ImGui.Button("Apply & Save", buttonSize))
            {
                ApplySettings();
            }
            
            ImGui.SameLine(0, buttonSpacing);
            
            if (ImGui.Button("Back", buttonSize))
            {
                _currentState = MenuState.MainMenu;
            }
            
            ImGui.Dummy(new Vector2(0, 10));
            ImGui.Separator();
            ImGui.Dummy(new Vector2(0, 5));
            ImGui.TextColored(new Vector4(0.6f, 0.6f, 0.6f, 1.0f), "Note: Some settings require a restart to take effect.");
        }
        ImGui.End();
        
        if (!open)
        {
            _currentState = MenuState.MainMenu;
        }
    }
    
    private void LoadAvailableSaves()
    {
        _availableSaves = _gameEngine.GetSaveGameInfo();
        if (_selectedSaveIndex >= _availableSaves.Count)
        {
            _selectedSaveIndex = Math.Max(0, _availableSaves.Count - 1);
        }
    }
    
    private void DeleteSave(int index)
    {
        if (index >= 0 && index < _availableSaves.Count)
        {
            try
            {
                string savePath = Path.Combine(SaveGameManager.Instance.GetSaveDirectory(), _availableSaves[index].FileName);
                if (File.Exists(savePath))
                {
                    File.Delete(savePath);
                    Console.WriteLine($"Deleted save: {_availableSaves[index].SaveName}");
                }
                LoadAvailableSaves();
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error deleting save: {ex.Message}");
            }
        }
    }
    
    private void RefreshServerList()
    {
        // TODO: Implement server discovery/listing
        // For now, just show placeholder
        _serverList.Clear();
        _serverList.Add("Local Server - 127.0.0.1:27015 (2/16 players)");
    }
}
