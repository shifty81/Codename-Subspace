using ImGuiNET;
using System.Numerics;

namespace AvorionLike.Core.UI;

/// <summary>
/// Menu system for managing game menus (main menu, pause menu, settings, etc.)
/// </summary>
public class MenuSystem
{
    private readonly GameEngine _gameEngine;
    private MenuState _currentMenu = MenuState.None;
    private bool _showMainMenu = false;
    private bool _showPauseMenu = false;
    private bool _showSettingsMenu = false;
    private bool _showSaveDialog = false;
    private bool _showLoadDialog = false;
    private string _saveGameName = "QuickSave";
    private List<string> _availableSaves = new List<string>();
    private int _selectedSaveIndex = 0;
    
    // Settings values
    private float _masterVolume = 1.0f;
    private float _musicVolume = 0.8f;
    private float _sfxVolume = 0.9f;
    private int _targetFPS = 60;
    private bool _vsync = true;
    private int _selectedResolution = 0;
    private readonly string[] _resolutions = new[] { "1280x720", "1920x1080", "2560x1440", "3840x2160" };
    private float _mouseSensitivity = 0.5f;
    private float _gameplayDifficulty = 1.0f; // 0.5 = Easy, 1.0 = Normal, 1.5 = Hard, 2.0 = Very Hard
    private bool _showFPS = true;
    private bool _autoSave = true;
    private int _autoSaveInterval = 5; // minutes
    
    public bool IsMenuOpen => _currentMenu != MenuState.None;
    
    public enum MenuState
    {
        None,
        MainMenu,
        PauseMenu,
        Settings
    }
    
    public MenuSystem(GameEngine gameEngine)
    {
        _gameEngine = gameEngine;
    }
    
    public void ShowMainMenu()
    {
        _currentMenu = MenuState.MainMenu;
        _showMainMenu = true;
    }
    
    public void ShowPauseMenu()
    {
        _currentMenu = MenuState.PauseMenu;
        _showPauseMenu = true;
    }
    
    public void HideMenu()
    {
        _currentMenu = MenuState.None;
        _showMainMenu = false;
        _showPauseMenu = false;
        _showSettingsMenu = false;
    }
    
    public void Render()
    {
        if (_showMainMenu)
            RenderMainMenu();
        
        if (_showPauseMenu)
            RenderPauseMenu();
        
        if (_showSettingsMenu)
            RenderSettingsMenu();
        
        if (_showSaveDialog)
            RenderSaveDialog();
        
        if (_showLoadDialog)
            RenderLoadDialog();
    }
    
    private void RenderMainMenu()
    {
        // Center the menu window
        var io = ImGui.GetIO();
        ImGui.SetNextWindowPos(new Vector2(io.DisplaySize.X * 0.5f, io.DisplaySize.Y * 0.5f), ImGuiCond.Always, new Vector2(0.5f, 0.5f));
        ImGui.SetNextWindowSize(new Vector2(400, 500), ImGuiCond.FirstUseEver);
        
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags.NoResize | 
                                       ImGuiWindowFlags.NoCollapse |
                                       ImGuiWindowFlags.NoMove;
        
        if (ImGui.Begin("AvorionLike - Main Menu", ref _showMainMenu, windowFlags))
        {
            ImGui.PushStyleVar(ImGuiStyleVar.FramePadding, new Vector2(10, 10));
            
            ImGui.Dummy(new Vector2(0, 20));
            
            // Title
            var titleText = "AVORION-LIKE";
            var titleSize = ImGui.CalcTextSize(titleText);
            ImGui.SetCursorPosX((ImGui.GetWindowWidth() - titleSize.X) * 0.5f);
            ImGui.TextColored(new Vector4(0.3f, 0.7f, 1.0f, 1.0f), titleText);
            
            ImGui.Dummy(new Vector2(0, 10));
            ImGui.Separator();
            ImGui.Dummy(new Vector2(0, 20));
            
            // Menu buttons
            Vector2 buttonSize = new Vector2(ImGui.GetWindowWidth() - 40, 40);
            
            ImGui.SetCursorPosX(20);
            if (ImGui.Button("New Game", buttonSize))
            {
                Console.WriteLine("Starting new game...");
                HideMenu();
            }
            
            ImGui.SetCursorPosX(20);
            if (ImGui.Button("Continue", buttonSize))
            {
                Console.WriteLine("Continuing game...");
                HideMenu();
            }
            
            ImGui.SetCursorPosX(20);
            if (ImGui.Button("Load Game", buttonSize))
            {
                _availableSaves = _gameEngine.GetSaveGames();
                _showLoadDialog = true;
            }
            
            ImGui.SetCursorPosX(20);
            if (ImGui.Button("Settings", buttonSize))
            {
                _showMainMenu = false;
                _showSettingsMenu = true;
                _currentMenu = MenuState.Settings;
            }
            
            ImGui.Dummy(new Vector2(0, 20));
            ImGui.Separator();
            ImGui.Dummy(new Vector2(0, 10));
            
            ImGui.SetCursorPosX(20);
            if (ImGui.Button("Exit", buttonSize))
            {
                Console.WriteLine("Exiting game...");
                Environment.Exit(0);
            }
            
            ImGui.PopStyleVar();
        }
        ImGui.End();
        
        if (!_showMainMenu)
            HideMenu();
    }
    
    private void RenderPauseMenu()
    {
        var io = ImGui.GetIO();
        ImGui.SetNextWindowPos(new Vector2(io.DisplaySize.X * 0.5f, io.DisplaySize.Y * 0.5f), ImGuiCond.Always, new Vector2(0.5f, 0.5f));
        ImGui.SetNextWindowSize(new Vector2(350, 400), ImGuiCond.FirstUseEver);
        
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags.NoResize | 
                                       ImGuiWindowFlags.NoCollapse |
                                       ImGuiWindowFlags.NoMove;
        
        if (ImGui.Begin("Game Paused", ref _showPauseMenu, windowFlags))
        {
            ImGui.PushStyleVar(ImGuiStyleVar.FramePadding, new Vector2(10, 10));
            
            ImGui.Dummy(new Vector2(0, 20));
            
            var titleText = "PAUSED";
            var titleSize = ImGui.CalcTextSize(titleText);
            ImGui.SetCursorPosX((ImGui.GetWindowWidth() - titleSize.X) * 0.5f);
            ImGui.TextColored(new Vector4(1.0f, 0.8f, 0.2f, 1.0f), titleText);
            
            ImGui.Dummy(new Vector2(0, 10));
            ImGui.Separator();
            ImGui.Dummy(new Vector2(0, 20));
            
            Vector2 buttonSize = new Vector2(ImGui.GetWindowWidth() - 40, 40);
            
            ImGui.SetCursorPosX(20);
            if (ImGui.Button("Resume", buttonSize))
            {
                HideMenu();
            }
            
            ImGui.SetCursorPosX(20);
            if (ImGui.Button("Settings", buttonSize))
            {
                _showPauseMenu = false;
                _showSettingsMenu = true;
                _currentMenu = MenuState.Settings;
            }
            
            ImGui.SetCursorPosX(20);
            if (ImGui.Button("Save Game", buttonSize))
            {
                _showSaveDialog = true;
            }
            
            ImGui.SetCursorPosX(20);
            if (ImGui.Button("Load Game", buttonSize))
            {
                _availableSaves = _gameEngine.GetSaveGames();
                _showLoadDialog = true;
            }
            
            ImGui.Dummy(new Vector2(0, 20));
            ImGui.Separator();
            ImGui.Dummy(new Vector2(0, 10));
            
            ImGui.SetCursorPosX(20);
            if (ImGui.Button("Main Menu", buttonSize))
            {
                _showPauseMenu = false;
                _showMainMenu = true;
                _currentMenu = MenuState.MainMenu;
            }
            
            ImGui.PopStyleVar();
        }
        ImGui.End();
        
        if (!_showPauseMenu && _currentMenu == MenuState.PauseMenu)
            HideMenu();
    }
    
    private void RenderSettingsMenu()
    {
        var io = ImGui.GetIO();
        ImGui.SetNextWindowPos(new Vector2(io.DisplaySize.X * 0.5f, io.DisplaySize.Y * 0.5f), ImGuiCond.Always, new Vector2(0.5f, 0.5f));
        ImGui.SetNextWindowSize(new Vector2(600, 650), ImGuiCond.FirstUseEver);
        
        if (ImGui.Begin("Settings", ref _showSettingsMenu))
        {
            if (ImGui.BeginTabBar("SettingsTabs"))
            {
                if (ImGui.BeginTabItem("Video Settings"))
                {
                    ImGui.Dummy(new Vector2(0, 10));
                    
                    ImGui.Text("Display Settings");
                    ImGui.Separator();
                    ImGui.Dummy(new Vector2(0, 5));
                    
                    ImGui.Text("Resolution:");
                    ImGui.Combo("##Resolution", ref _selectedResolution, _resolutions, _resolutions.Length);
                    
                    ImGui.Dummy(new Vector2(0, 10));
                    
                    ImGui.Checkbox("VSync", ref _vsync);
                    ImGui.Checkbox("Show FPS Counter", ref _showFPS);
                    
                    ImGui.Dummy(new Vector2(0, 10));
                    
                    ImGui.Text($"Target FPS: {_targetFPS}");
                    ImGui.SliderInt("##TargetFPS", ref _targetFPS, 30, 144);
                    
                    ImGui.Dummy(new Vector2(0, 20));
                    ImGui.Text("Graphics Quality");
                    ImGui.Separator();
                    ImGui.Dummy(new Vector2(0, 5));
                    
                    ImGui.TextWrapped("Rendering quality settings will be added in future updates.");
                    
                    ImGui.Dummy(new Vector2(0, 20));
                    ImGui.Separator();
                    ImGui.Dummy(new Vector2(0, 10));
                    
                    ImGui.TextWrapped("Note: Some settings require a restart to take effect.");
                    
                    ImGui.EndTabItem();
                }
                
                if (ImGui.BeginTabItem("Sound Settings"))
                {
                    ImGui.Dummy(new Vector2(0, 10));
                    
                    ImGui.Text("Volume Controls");
                    ImGui.Separator();
                    ImGui.Dummy(new Vector2(0, 10));
                    
                    ImGui.Text($"Master Volume: {_masterVolume * 100:F0}%");
                    ImGui.SliderFloat("##MasterVolume", ref _masterVolume, 0.0f, 1.0f);
                    
                    ImGui.Dummy(new Vector2(0, 10));
                    
                    ImGui.Text($"Music Volume: {_musicVolume * 100:F0}%");
                    ImGui.SliderFloat("##MusicVolume", ref _musicVolume, 0.0f, 1.0f);
                    
                    ImGui.Dummy(new Vector2(0, 10));
                    
                    ImGui.Text($"SFX Volume: {_sfxVolume * 100:F0}%");
                    ImGui.SliderFloat("##SFXVolume", ref _sfxVolume, 0.0f, 1.0f);
                    
                    ImGui.Dummy(new Vector2(0, 20));
                    ImGui.Separator();
                    ImGui.Dummy(new Vector2(0, 10));
                    
                    ImGui.TextWrapped("Audio system will be implemented in future updates.");
                    
                    ImGui.EndTabItem();
                }
                
                if (ImGui.BeginTabItem("Gameplay Settings"))
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
                    if (ImGui.SliderFloat("##Difficulty", ref _gameplayDifficulty, 0.5f, 2.0f))
                    {
                        // Snap to specific values for better UX
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
                    ImGui.SliderFloat("##MouseSens", ref _mouseSensitivity, 0.1f, 2.0f);
                    
                    ImGui.Dummy(new Vector2(0, 20));
                    ImGui.Text("Auto-Save");
                    ImGui.Separator();
                    ImGui.Dummy(new Vector2(0, 10));
                    
                    ImGui.Checkbox("Enable Auto-Save", ref _autoSave);
                    
                    if (_autoSave)
                    {
                        ImGui.Text($"Auto-Save Interval: {_autoSaveInterval} minutes");
                        ImGui.SliderInt("##AutoSaveInterval", ref _autoSaveInterval, 1, 30);
                    }
                    
                    ImGui.EndTabItem();
                }
                
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
                    ImGui.BulletText("I - Toggle Inventory");
                    ImGui.BulletText("B - Toggle Ship Builder");
                    
                    ImGui.Dummy(new Vector2(0, 20));
                    ImGui.TextWrapped("Key remapping will be added in future updates.");
                    
                    ImGui.EndTabItem();
                }
                
                ImGui.EndTabBar();
            }
            
            ImGui.Dummy(new Vector2(0, 20));
            
            // Bottom buttons
            Vector2 buttonSize = new Vector2(150, 30);
            float buttonSpacing = 10;
            float totalWidth = buttonSize.X * 2 + buttonSpacing;
            ImGui.SetCursorPosX((ImGui.GetWindowWidth() - totalWidth) * 0.5f);
            
            if (ImGui.Button("Apply", buttonSize))
            {
                ApplySettings();
            }
            
            ImGui.SameLine(0, buttonSpacing);
            
            if (ImGui.Button("Back", buttonSize))
            {
                _showSettingsMenu = false;
                if (_currentMenu == MenuState.PauseMenu)
                    _showPauseMenu = true;
                else
                    _showMainMenu = true;
            }
        }
        ImGui.End();
        
        if (!_showSettingsMenu && _currentMenu == MenuState.Settings)
        {
            _currentMenu = MenuState.None;
        }
    }
    
    private void ApplySettings()
    {
        Console.WriteLine("Applying settings...");
        Console.WriteLine($"  Resolution: {_resolutions[_selectedResolution]}");
        Console.WriteLine($"  VSync: {_vsync}");
        Console.WriteLine($"  Target FPS: {_targetFPS}");
        Console.WriteLine($"  Show FPS: {_showFPS}");
        Console.WriteLine($"  Master Volume: {_masterVolume * 100:F0}%");
        Console.WriteLine($"  Music Volume: {_musicVolume * 100:F0}%");
        Console.WriteLine($"  SFX Volume: {_sfxVolume * 100:F0}%");
        Console.WriteLine($"  Mouse Sensitivity: {_mouseSensitivity:F2}");
        Console.WriteLine($"  Difficulty: {_gameplayDifficulty:F2}x");
        Console.WriteLine($"  Auto-Save: {_autoSave} ({_autoSaveInterval} minutes)");
        
        // TODO: Apply settings to game engine
        // This would involve:
        // - Resizing window if resolution changed
        // - Updating VSync setting
        // - Applying volume changes to audio system
        // - Updating mouse sensitivity in camera/input systems
        // - Setting difficulty multipliers in game systems
    }
    
    private void RenderSaveDialog()
    {
        var io = ImGui.GetIO();
        ImGui.SetNextWindowPos(new Vector2(io.DisplaySize.X * 0.5f, io.DisplaySize.Y * 0.5f), ImGuiCond.Always, new Vector2(0.5f, 0.5f));
        ImGui.SetNextWindowSize(new Vector2(400, 200), ImGuiCond.Always);
        
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags.NoResize | 
                                       ImGuiWindowFlags.NoCollapse |
                                       ImGuiWindowFlags.NoMove;
        
        if (ImGui.Begin("Save Game", ref _showSaveDialog, windowFlags))
        {
            ImGui.Text("Enter save name:");
            ImGui.SetNextItemWidth(360);
            ImGui.InputText("##savename", ref _saveGameName, 100);
            
            ImGui.Dummy(new Vector2(0, 20));
            
            ImGui.SetCursorPosX(80);
            if (ImGui.Button("Save", new Vector2(100, 40)))
            {
                if (!string.IsNullOrWhiteSpace(_saveGameName))
                {
                    bool success = _gameEngine.SaveGame(_saveGameName);
                    if (success)
                    {
                        Console.WriteLine($"Game saved successfully: {_saveGameName}");
                        _showSaveDialog = false;
                    }
                    else
                    {
                        Console.WriteLine("Failed to save game!");
                    }
                }
            }
            
            ImGui.SameLine();
            ImGui.SetCursorPosX(220);
            if (ImGui.Button("Cancel", new Vector2(100, 40)))
            {
                _showSaveDialog = false;
            }
            
            ImGui.End();
        }
    }
    
    private void RenderLoadDialog()
    {
        var io = ImGui.GetIO();
        ImGui.SetNextWindowPos(new Vector2(io.DisplaySize.X * 0.5f, io.DisplaySize.Y * 0.5f), ImGuiCond.Always, new Vector2(0.5f, 0.5f));
        ImGui.SetNextWindowSize(new Vector2(400, 400), ImGuiCond.Always);
        
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags.NoResize | 
                                       ImGuiWindowFlags.NoCollapse |
                                       ImGuiWindowFlags.NoMove;
        
        if (ImGui.Begin("Load Game", ref _showLoadDialog, windowFlags))
        {
            ImGui.Text("Select a save to load:");
            ImGui.Dummy(new Vector2(0, 10));
            
            // List available saves
            if (_availableSaves.Count == 0)
            {
                ImGui.Text("No saved games found.");
            }
            else
            {
                if (ImGui.BeginChild("SavesList", new Vector2(360, 250)))
                {
                    for (int i = 0; i < _availableSaves.Count; i++)
                    {
                        bool isSelected = (_selectedSaveIndex == i);
                        if (ImGui.Selectable(_availableSaves[i], isSelected))
                        {
                            _selectedSaveIndex = i;
                        }
                    }
                    
                    ImGui.EndChild();
                }
            }
            
            ImGui.Dummy(new Vector2(0, 10));
            
            ImGui.SetCursorPosX(80);
            bool canLoad = _availableSaves.Count > 0 && _selectedSaveIndex >= 0 && _selectedSaveIndex < _availableSaves.Count;
            
            if (!canLoad)
                ImGui.BeginDisabled();
                
            if (ImGui.Button("Load", new Vector2(100, 40)))
            {
                if (canLoad)
                {
                    string selectedSave = _availableSaves[_selectedSaveIndex];
                    bool success = _gameEngine.LoadGame(selectedSave);
                    if (success)
                    {
                        Console.WriteLine($"Game loaded successfully: {selectedSave}");
                        _showLoadDialog = false;
                        HideMenu(); // Close all menus after loading
                    }
                    else
                    {
                        Console.WriteLine("Failed to load game!");
                    }
                }
            }
            
            if (!canLoad)
                ImGui.EndDisabled();
            
            ImGui.SameLine();
            ImGui.SetCursorPosX(220);
            if (ImGui.Button("Cancel", new Vector2(100, 40)))
            {
                _showLoadDialog = false;
            }
            
            ImGui.End();
        }
    }
    
    public void HandleInput()
    {
        // Toggle pause menu with ESC
        if (ImGui.IsKeyPressed(ImGuiKey.Escape))
        {
            if (_currentMenu == MenuState.PauseMenu)
            {
                // ESC pressed while pause menu is open - close it
                HideMenu();
            }
            else if (_currentMenu == MenuState.Settings)
            {
                // ESC pressed while in settings - go back to pause menu
                _showSettingsMenu = false;
                _showPauseMenu = true;
                _currentMenu = MenuState.PauseMenu;
            }
            else if (_currentMenu == MenuState.MainMenu)
            {
                // Don't do anything if we're in main menu
            }
            else if (_currentMenu == MenuState.None)
            {
                // ESC pressed during gameplay - open pause menu
                ShowPauseMenu();
            }
        }
    }
}
