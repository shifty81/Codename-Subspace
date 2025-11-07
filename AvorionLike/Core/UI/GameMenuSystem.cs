using System.Numerics;
using System.Collections.Generic;
using Silk.NET.Input;
using ImGuiNET;
using AvorionLike.Core.Configuration;
using AvorionLike.Core.Persistence;

namespace AvorionLike.Core.UI;

/// <summary>
/// Custom-rendered game menu system (pause menu, settings, etc.)
/// Uses CustomUIRenderer for graphics and ImGui for text rendering
/// </summary>
public class GameMenuSystem
{
    private readonly GameEngine _gameEngine;
    private readonly CustomUIRenderer _renderer;
    private float _screenWidth;
    private float _screenHeight;
    
    private bool _isPauseMenuOpen = false;
    private bool _isSettingsMenuOpen = false;
    private bool _isSaveDialogOpen = false;
    private bool _isLoadDialogOpen = false;
    private bool _isConfirmDialogOpen = false;
    private string _confirmDialogMessage = "";
    private Action? _confirmAction = null;
    private int _selectedMenuItem = 0;
    private int _pauseMenuItemCount = 5; // Resume, Settings, Save, Load, Main Menu
    
    // Settings menu state
    private int _selectedSettingsTab = 0;
    private int _settingsTabCount = 3; // Graphics, Audio, Gameplay
    private int _selectedSetting = 0;
    private int _settingsPerTab = 4; // Number of editable settings per tab
    
    // Save/Load dialog state
    private int _selectedSaveSlot = 0;
    private string _newSaveName = "New Save";
    
    // Mouse state
    private Vector2 _mousePos;
    private int _hoveredMenuItem = -1;
    
    // Track key presses to prevent repeat actions
    private readonly HashSet<Key> _keysPressed = new HashSet<Key>();
    private readonly HashSet<Key> _keysPressedLastFrame = new HashSet<Key>();
    
    public bool IsMenuOpen => _isPauseMenuOpen || _isSettingsMenuOpen || _isSaveDialogOpen || _isLoadDialogOpen || _isConfirmDialogOpen;
    
    public GameMenuSystem(GameEngine gameEngine, CustomUIRenderer renderer, float screenWidth, float screenHeight)
    {
        _gameEngine = gameEngine;
        _renderer = renderer;
        _screenWidth = screenWidth;
        _screenHeight = screenHeight;
    }
    
    public void UpdateScreenSize(float width, float height)
    {
        _screenWidth = width;
        _screenHeight = height;
    }
    
    public void TogglePauseMenu()
    {
        if (_isSettingsMenuOpen)
        {
            // Close settings and go back to pause menu
            _isSettingsMenuOpen = false;
            _isPauseMenuOpen = true;
        }
        else if (_isPauseMenuOpen)
        {
            // Close pause menu
            _isPauseMenuOpen = false;
        }
        else
        {
            // Open pause menu
            _isPauseMenuOpen = true;
            _selectedMenuItem = 0;
        }
    }
    
    public void HandleMouseMove(Vector2 position)
    {
        _mousePos = position;
        
        // Update hovered and selected menu item
        if (_isPauseMenuOpen)
        {
            _hoveredMenuItem = GetHoveredPauseMenuItem(position);
            if (_hoveredMenuItem >= 0)
            {
                _selectedMenuItem = _hoveredMenuItem;
            }
        }
    }
    
    public void HandleMouseClick(Vector2 position, MouseButton button)
    {
        if (button != MouseButton.Left) return;
        
        if (_isPauseMenuOpen)
        {
            int clickedItem = GetHoveredPauseMenuItem(position);
            if (clickedItem >= 0)
            {
                _selectedMenuItem = clickedItem;
                ExecutePauseMenuItem(_selectedMenuItem);
            }
        }
        else if (_isConfirmDialogOpen)
        {
            // Check if Yes or No button was clicked
            float dialogWidth = 400f;
            float dialogHeight = 200f;
            float dialogX = (_screenWidth - dialogWidth) * 0.5f;
            float dialogY = (_screenHeight - dialogHeight) * 0.5f;
            
            float buttonWidth = 120f;
            float buttonHeight = 40f;
            float buttonSpacing = 20f;
            float totalButtonWidth = buttonWidth * 2 + buttonSpacing;
            float buttonY = dialogY + dialogHeight - buttonHeight - 30f;
            float yesButtonX = dialogX + (dialogWidth - totalButtonWidth) * 0.5f;
            float noButtonX = yesButtonX + buttonWidth + buttonSpacing;
            
            // Check Yes button
            if (position.X >= yesButtonX && position.X <= yesButtonX + buttonWidth &&
                position.Y >= buttonY && position.Y <= buttonY + buttonHeight)
            {
                _confirmAction?.Invoke();
                _isConfirmDialogOpen = false;
                _confirmAction = null;
            }
            // Check No button
            else if (position.X >= noButtonX && position.X <= noButtonX + buttonWidth &&
                     position.Y >= buttonY && position.Y <= buttonY + buttonHeight)
            {
                _isConfirmDialogOpen = false;
                _isPauseMenuOpen = true;
                _confirmAction = null;
            }
        }
    }
    
    private int GetHoveredPauseMenuItem(Vector2 position)
    {
        float panelWidth = 400f;
        float panelHeight = 500f;
        float panelX = (_screenWidth - panelWidth) * 0.5f;
        float panelY = (_screenHeight - panelHeight) * 0.5f;
        float titleBarHeight = 60f;
        float contentY = panelY + titleBarHeight + 40f;
        float menuItemHeight = 60f;
        float menuItemSpacing = 15f;
        
        // Check each menu item
        for (int i = 0; i < _pauseMenuItemCount; i++)
        {
            float itemY = contentY + i * (menuItemHeight + menuItemSpacing);
            float itemX = panelX + 50f;
            float itemWidth = panelWidth - 100f;
            
            if (position.X >= itemX && position.X <= itemX + itemWidth &&
                position.Y >= itemY && position.Y <= itemY + menuItemHeight)
            {
                return i;
            }
        }
        
        return -1;
    }
    
    public void HandleInput(IKeyboard keyboard)
    {
        // ESC key handling is done externally
        
        if (!IsMenuOpen) return;
        
        // Update current keys pressed - only check the keys we care about
        _keysPressed.Clear();
        Key[] keysToCheck = { Key.Up, Key.Down, Key.W, Key.S, Key.Enter, Key.Space, Key.Backspace, Key.Escape, Key.Left, Key.Right, Key.A, Key.D };
        foreach (var key in keysToCheck)
        {
            if (keyboard.IsKeyPressed(key))
            {
                _keysPressed.Add(key);
            }
        }
        
        // Handle keyboard navigation for different menu states
        if (_isPauseMenuOpen)
        {
            HandlePauseMenuInput();
        }
        else if (_isSettingsMenuOpen)
        {
            HandleSettingsMenuInput();
        }
        else if (_isSaveDialogOpen)
        {
            HandleSaveDialogInput();
        }
        else if (_isLoadDialogOpen)
        {
            HandleLoadDialogInput();
        }
        
        // Copy current keys to last frame
        _keysPressedLastFrame.Clear();
        foreach (var key in _keysPressed)
        {
            _keysPressedLastFrame.Add(key);
        }
    }
    
    private bool WasKeyJustPressed(Key key)
    {
        return _keysPressed.Contains(key) && !_keysPressedLastFrame.Contains(key);
    }
    
    private void HandlePauseMenuInput()
    {
        // Navigate up
        if (WasKeyJustPressed(Key.Up) || WasKeyJustPressed(Key.W))
        {
            _selectedMenuItem--;
            if (_selectedMenuItem < 0)
                _selectedMenuItem = _pauseMenuItemCount - 1;
        }
        
        // Navigate down
        if (WasKeyJustPressed(Key.Down) || WasKeyJustPressed(Key.S))
        {
            _selectedMenuItem++;
            if (_selectedMenuItem >= _pauseMenuItemCount)
                _selectedMenuItem = 0;
        }
        
        // Activate selected item
        if (WasKeyJustPressed(Key.Enter) || WasKeyJustPressed(Key.Space))
        {
            ExecutePauseMenuItem(_selectedMenuItem);
        }
    }
    
    private void HandleSettingsMenuInput()
    {
        // Tab switching with Left/Right or A/D
        if (WasKeyJustPressed(Key.Left) || WasKeyJustPressed(Key.A))
        {
            _selectedSettingsTab = (_selectedSettingsTab - 1 + _settingsTabCount) % _settingsTabCount;
            _selectedSetting = 0; // Reset selection when changing tabs
        }
        else if (WasKeyJustPressed(Key.Right) || WasKeyJustPressed(Key.D))
        {
            _selectedSettingsTab = (_selectedSettingsTab + 1) % _settingsTabCount;
            _selectedSetting = 0; // Reset selection when changing tabs
        }
        
        // Navigate settings with Up/Down or W/S
        if (WasKeyJustPressed(Key.Up) || WasKeyJustPressed(Key.W))
        {
            _selectedSetting = (_selectedSetting - 1 + _settingsPerTab) % _settingsPerTab;
        }
        else if (WasKeyJustPressed(Key.Down) || WasKeyJustPressed(Key.S))
        {
            _selectedSetting = (_selectedSetting + 1) % _settingsPerTab;
        }
        
        // Adjust setting values with Enter (toggle) or Left/Right (adjust slider)
        if (WasKeyJustPressed(Key.Enter) || WasKeyJustPressed(Key.Space))
        {
            ToggleSelectedSetting();
        }
        
        // Back to pause menu
        if (WasKeyJustPressed(Key.Backspace) || WasKeyJustPressed(Key.Escape))
        {
            // Save settings before going back
            SaveSettings();
            _isSettingsMenuOpen = false;
            _isPauseMenuOpen = true;
        }
    }
    
    private void ToggleSelectedSetting()
    {
        var config = ConfigurationManager.Instance.Config;
        
        switch (_selectedSettingsTab)
        {
            case 0: // Graphics
                switch (_selectedSetting)
                {
                    case 0: config.Graphics.Fullscreen = !config.Graphics.Fullscreen; break;
                    case 1: config.Graphics.VSync = !config.Graphics.VSync; break;
                    case 2: config.Graphics.EnableShadows = !config.Graphics.EnableShadows; break;
                    case 3: config.Graphics.EnableParticles = !config.Graphics.EnableParticles; break;
                }
                break;
            case 1: // Audio
                switch (_selectedSetting)
                {
                    case 0: config.Audio.Muted = !config.Audio.Muted; break;
                    case 1: config.Audio.MasterVolume = (config.Audio.MasterVolume >= 1.0f) ? 0.0f : 1.0f; break;
                    case 2: config.Audio.MusicVolume = (config.Audio.MusicVolume >= 1.0f) ? 0.0f : 1.0f; break;
                    case 3: config.Audio.SfxVolume = (config.Audio.SfxVolume >= 1.0f) ? 0.0f : 1.0f; break;
                }
                break;
            case 2: // Gameplay
                switch (_selectedSetting)
                {
                    case 0: config.Gameplay.EnableAutoSave = !config.Gameplay.EnableAutoSave; break;
                    case 1: config.Gameplay.ShowTutorials = !config.Gameplay.ShowTutorials; break;
                    case 2: config.Gameplay.EnableHints = !config.Gameplay.EnableHints; break;
                    case 3: 
                        // Cycle through difficulty levels
                        config.Gameplay.Difficulty = (config.Gameplay.Difficulty + 1) % 3;
                        break;
                }
                break;
        }
    }
    
    private void SaveSettings()
    {
        try
        {
            // Save configuration to disk
            var configManager = ConfigurationManager.Instance;
            configManager.SaveConfiguration();
            Console.WriteLine("Settings saved successfully");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error saving settings: {ex.Message}");
        }
    }
    
    private void HandleSaveDialogInput()
    {
        var saveManager = SaveGameManager.Instance;
        var saves = saveManager.ListSaveGames();
        // Limit navigation to visible slots (max 5)
        int slotCount = Math.Min(5, Math.Max(1, saves.Count + 1));
        
        // Navigate save slots
        if (WasKeyJustPressed(Key.Up) || WasKeyJustPressed(Key.W))
        {
            _selectedSaveSlot = (_selectedSaveSlot - 1 + slotCount) % slotCount;
        }
        else if (WasKeyJustPressed(Key.Down) || WasKeyJustPressed(Key.S))
        {
            _selectedSaveSlot = (_selectedSaveSlot + 1) % slotCount;
        }
        
        // Confirm save
        if (WasKeyJustPressed(Key.Enter) || WasKeyJustPressed(Key.Space))
        {
            PerformSave();
            _isSaveDialogOpen = false;
            _isPauseMenuOpen = true;
        }
        
        // Cancel
        if (WasKeyJustPressed(Key.Backspace) || WasKeyJustPressed(Key.Escape))
        {
            _isSaveDialogOpen = false;
            _isPauseMenuOpen = true;
        }
    }
    
    private void HandleLoadDialogInput()
    {
        var saveManager = SaveGameManager.Instance;
        var saves = saveManager.ListSaveGames();
        
        if (saves.Count == 0)
        {
            // No saves available, return to menu
            if (WasKeyJustPressed(Key.Backspace) || WasKeyJustPressed(Key.Escape) || 
                WasKeyJustPressed(Key.Enter) || WasKeyJustPressed(Key.Space))
            {
                _isLoadDialogOpen = false;
                _isPauseMenuOpen = true;
            }
            return;
        }
        
        // Navigate save slots
        if (WasKeyJustPressed(Key.Up) || WasKeyJustPressed(Key.W))
        {
            _selectedSaveSlot = (_selectedSaveSlot - 1 + saves.Count) % saves.Count;
        }
        else if (WasKeyJustPressed(Key.Down) || WasKeyJustPressed(Key.S))
        {
            _selectedSaveSlot = (_selectedSaveSlot + 1) % saves.Count;
        }
        
        // Confirm load
        if (WasKeyJustPressed(Key.Enter) || WasKeyJustPressed(Key.Space))
        {
            PerformLoad();
            _isLoadDialogOpen = false;
            _isPauseMenuOpen = false; // Close menu after loading
        }
        
        // Cancel
        if (WasKeyJustPressed(Key.Backspace) || WasKeyJustPressed(Key.Escape))
        {
            _isLoadDialogOpen = false;
            _isPauseMenuOpen = true;
        }
    }
    
    private void PerformSave()
    {
        try
        {
            // Use GameEngine's SaveGame method which handles full serialization
            if (_gameEngine.SaveGame(_newSaveName))
            {
                Console.WriteLine($"Game saved as {_newSaveName}");
            }
            else
            {
                Console.WriteLine("Failed to save game");
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error saving game: {ex.Message}");
        }
    }
    
    private void PerformLoad()
    {
        try
        {
            var saves = _gameEngine.GetSaveGameInfo();
            
            if (_selectedSaveSlot >= 0 && _selectedSaveSlot < saves.Count)
            {
                var saveInfo = saves[_selectedSaveSlot];
                
                // Use GameEngine's LoadGame method which handles full deserialization
                if (_gameEngine.LoadGame(saveInfo.FileName.Replace(".save", "")))
                {
                    Console.WriteLine($"Game loaded: {saveInfo.SaveName}");
                }
                else
                {
                    Console.WriteLine("Failed to load game");
                }
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error loading game: {ex.Message}");
        }
    }
    
    private void ExecutePauseMenuItem(int itemIndex)
    {
        switch (itemIndex)
        {
            case 0: // Resume
                _isPauseMenuOpen = false;
                Console.WriteLine("Game resumed");
                break;
            case 1: // Settings
                _isPauseMenuOpen = false;
                _isSettingsMenuOpen = true;
                Console.WriteLine("Opening settings menu");
                break;
            case 2: // Save Game
                _isPauseMenuOpen = false;
                _isSaveDialogOpen = true;
                _selectedSaveSlot = 0;
                Console.WriteLine("Opening save dialog");
                break;
            case 3: // Load Game
                _isPauseMenuOpen = false;
                _isLoadDialogOpen = true;
                _selectedSaveSlot = 0;
                Console.WriteLine("Opening load dialog");
                break;
            case 4: // Main Menu
                _confirmDialogMessage = "Return to main menu?\n\nUnsaved progress will be lost.";
                _confirmAction = () => 
                {
                    Console.WriteLine("Returning to main menu - closing game window");
                    // Signal to close the game window
                    Environment.Exit(0);
                };
                _isConfirmDialogOpen = true;
                _isPauseMenuOpen = false;
                break;
        }
    }
    
    public void Render()
    {
        if (!IsMenuOpen) return;
        
        _renderer.BeginRender();
        
        if (_isPauseMenuOpen)
        {
            RenderPauseMenu();
        }
        else if (_isSettingsMenuOpen)
        {
            RenderSettingsMenu();
        }
        else if (_isSaveDialogOpen)
        {
            RenderSaveDialog();
        }
        else if (_isLoadDialogOpen)
        {
            RenderLoadDialog();
        }
        else if (_isConfirmDialogOpen)
        {
            RenderConfirmDialog();
        }
        
        _renderer.EndRender();
        
        // Render text labels using ImGui after CustomUIRenderer is done
        RenderTextLabels();
    }
    
    private void RenderTextLabels()
    {
        if (!IsMenuOpen) return;
        
        var drawList = ImGui.GetForegroundDrawList();
        
        if (_isPauseMenuOpen)
        {
            RenderPauseMenuText(drawList);
        }
        else if (_isSettingsMenuOpen)
        {
            RenderSettingsMenuText(drawList);
        }
        else if (_isSaveDialogOpen)
        {
            RenderSaveDialogText(drawList);
        }
        else if (_isLoadDialogOpen)
        {
            RenderLoadDialogText(drawList);
        }
        else if (_isConfirmDialogOpen)
        {
            RenderConfirmDialogText(drawList);
        }
    }
    
    private void RenderPauseMenuText(ImDrawListPtr drawList)
    {
        float panelWidth = 400f;
        float panelHeight = 500f;
        float panelX = (_screenWidth - panelWidth) * 0.5f;
        float panelY = (_screenHeight - panelHeight) * 0.5f;
        float titleBarHeight = 60f;
        
        // Title text
        string titleText = "PAUSED";
        var titleSize = ImGui.CalcTextSize(titleText);
        float titleX = panelX + (panelWidth - titleSize.X) * 0.5f;
        float titleY = panelY + (titleBarHeight - titleSize.Y) * 0.5f;
        
        // Draw title with shadow for depth
        uint shadowColor = ImGui.ColorConvertFloat4ToU32(new Vector4(0f, 0f, 0f, 0.5f));
        uint titleColor = ImGui.ColorConvertFloat4ToU32(new Vector4(0.2f, 1.0f, 0.8f, 1.0f));
        drawList.AddText(new Vector2(titleX + 2, titleY + 2), shadowColor, titleText);
        drawList.AddText(new Vector2(titleX, titleY), titleColor, titleText);
        
        // Menu item labels
        string[] menuItems = { "Resume", "Settings", "Save Game", "Load Game", "Main Menu" };
        float buttonWidth = panelWidth - 80f;
        float buttonHeight = 50f;
        float buttonX = panelX + 40f;
        float buttonY = panelY + titleBarHeight + 40f;
        float buttonSpacing = 15f;
        
        for (int i = 0; i < menuItems.Length; i++)
        {
            float currentY = buttonY + i * (buttonHeight + buttonSpacing);
            
            // Center text in button
            var textSize = ImGui.CalcTextSize(menuItems[i]);
            float textX = buttonX + (buttonWidth - textSize.X) * 0.5f;
            float textY = currentY + (buttonHeight - textSize.Y) * 0.5f;
            
            // Text color (brighter for selected item)
            uint textColor = (i == _selectedMenuItem)
                ? ImGui.ColorConvertFloat4ToU32(new Vector4(1.0f, 1.0f, 1.0f, 1.0f))
                : ImGui.ColorConvertFloat4ToU32(new Vector4(0.7f, 0.9f, 1.0f, 1.0f));
            
            // Draw text with shadow
            drawList.AddText(new Vector2(textX + 1, textY + 1), shadowColor, menuItems[i]);
            drawList.AddText(new Vector2(textX, textY), textColor, menuItems[i]);
        }
    }
    
    private void RenderSettingsMenuText(ImDrawListPtr drawList)
    {
        float panelWidth = 700f;
        float panelHeight = 600f;
        float panelX = (_screenWidth - panelWidth) * 0.5f;
        float panelY = (_screenHeight - panelHeight) * 0.5f;
        float titleBarHeight = 60f;
        
        // Title text
        string titleText = "SETTINGS";
        var titleSize = ImGui.CalcTextSize(titleText);
        float titleX = panelX + (panelWidth - titleSize.X) * 0.5f;
        float titleY = panelY + (titleBarHeight - titleSize.Y) * 0.5f;
        
        // Draw title with shadow
        uint shadowColor = ImGui.ColorConvertFloat4ToU32(new Vector4(0f, 0f, 0f, 0.5f));
        uint titleColor = ImGui.ColorConvertFloat4ToU32(new Vector4(0.2f, 1.0f, 0.8f, 1.0f));
        drawList.AddText(new Vector2(titleX + 2, titleY + 2), shadowColor, titleText);
        drawList.AddText(new Vector2(titleX, titleY), titleColor, titleText);
        
        // Tab labels
        float tabY = panelY + titleBarHeight + 10f;
        float tabHeight = 40f;
        float tabWidth = (panelWidth - 80f) / 3f;
        float tabStartX = panelX + 40f;
        
        string[] tabs = { "Graphics", "Audio", "Gameplay" };
        uint activeTextColor = ImGui.ColorConvertFloat4ToU32(new Vector4(1.0f, 1.0f, 1.0f, 1.0f));
        uint inactiveTextColor = ImGui.ColorConvertFloat4ToU32(new Vector4(0.6f, 0.7f, 0.8f, 0.8f));
        
        for (int i = 0; i < tabs.Length; i++)
        {
            float tabX = tabStartX + i * tabWidth;
            var tabTextSize = ImGui.CalcTextSize(tabs[i]);
            float textX = tabX + (tabWidth - 5f - tabTextSize.X) * 0.5f;
            float textY = tabY + (tabHeight - tabTextSize.Y) * 0.5f;
            
            uint color = (i == _selectedSettingsTab) ? activeTextColor : inactiveTextColor;
            drawList.AddText(new Vector2(textX, textY), color, tabs[i]);
        }
        
        // Settings items
        var config = ConfigurationManager.Instance.Config;
        float settingsY = tabY + tabHeight + 20f;
        float settingHeight = 50f;
        float settingSpacing = 15f;
        float settingX = panelX + 50f;
        float settingWidth = panelWidth - 100f;
        
        string[] settingNames;
        string[] settingValues;
        
        switch (_selectedSettingsTab)
        {
            case 0: // Graphics
                settingNames = new[] { "Fullscreen", "VSync", "Shadows", "Particles" };
                settingValues = new[] {
                    config.Graphics.Fullscreen ? "ON" : "OFF",
                    config.Graphics.VSync ? "ON" : "OFF",
                    config.Graphics.EnableShadows ? "ON" : "OFF",
                    config.Graphics.EnableParticles ? "ON" : "OFF"
                };
                break;
            case 1: // Audio
                settingNames = new[] { "Mute All", "Master Volume", "Music Volume", "SFX Volume" };
                settingValues = new[] {
                    config.Audio.Muted ? "ON" : "OFF",
                    $"{(int)(config.Audio.MasterVolume * 100)}%",
                    $"{(int)(config.Audio.MusicVolume * 100)}%",
                    $"{(int)(config.Audio.SfxVolume * 100)}%"
                };
                break;
            case 2: // Gameplay
                settingNames = new[] { "Auto Save", "Show Tutorials", "Show Hints", "Difficulty" };
                string[] difficultyNames = { "Easy", "Normal", "Hard" };
                settingValues = new[] {
                    config.Gameplay.EnableAutoSave ? "ON" : "OFF",
                    config.Gameplay.ShowTutorials ? "ON" : "OFF",
                    config.Gameplay.EnableHints ? "ON" : "OFF",
                    difficultyNames[config.Gameplay.Difficulty]
                };
                break;
            default:
                settingNames = new[] { "", "", "", "" };
                settingValues = new[] { "", "", "", "" };
                break;
        }
        
        uint labelColor = ImGui.ColorConvertFloat4ToU32(new Vector4(0.7f, 0.9f, 1.0f, 1.0f));
        uint valueColor = ImGui.ColorConvertFloat4ToU32(new Vector4(0.2f, 1.0f, 0.8f, 1.0f));
        uint selectedValueColor = ImGui.ColorConvertFloat4ToU32(new Vector4(1.0f, 1.0f, 1.0f, 1.0f));
        
        for (int i = 0; i < _settingsPerTab; i++)
        {
            float currentY = settingsY + i * (settingHeight + settingSpacing);
            
            // Setting name (left aligned)
            float nameX = settingX + 15f;
            float nameY = currentY + (settingHeight - ImGui.GetTextLineHeight()) * 0.5f;
            drawList.AddText(new Vector2(nameX, nameY), labelColor, settingNames[i]);
            
            // Setting value (right aligned)
            var valueSize = ImGui.CalcTextSize(settingValues[i]);
            float valueX = settingX + settingWidth - valueSize.X - 15f;
            float valueY = currentY + (settingHeight - valueSize.Y) * 0.5f;
            uint vColor = (i == _selectedSetting) ? selectedValueColor : valueColor;
            drawList.AddText(new Vector2(valueX, valueY), vColor, settingValues[i]);
        }
        
        // Instructions at bottom
        float instructY = panelY + panelHeight - 100f;
        string[] instructions = {
            "Arrow Keys / WASD: Navigate  |  Enter/Space: Toggle",
            "Backspace/ESC: Save & Return to Pause Menu"
        };
        
        uint instructColor = ImGui.ColorConvertFloat4ToU32(new Vector4(0.5f, 0.6f, 0.7f, 0.8f));
        for (int i = 0; i < instructions.Length; i++)
        {
            var instrSize = ImGui.CalcTextSize(instructions[i]);
            float instrX = panelX + (panelWidth - instrSize.X) * 0.5f;
            float instrY = instructY + i * ImGui.GetTextLineHeightWithSpacing();
            drawList.AddText(new Vector2(instrX, instrY), instructColor, instructions[i]);
        }
        
        // "Back" button label
        float backButtonY = panelY + panelHeight - 70f;
        float backButtonHeight = 40f;
        string backText = "Back (Backspace)";
        var backSize = ImGui.CalcTextSize(backText);
        float backTextX = panelX + (panelWidth - backSize.X) * 0.5f;
        float backTextY = backButtonY + (backButtonHeight - backSize.Y) * 0.5f;
        drawList.AddText(new Vector2(backTextX, backTextY), labelColor, backText);
    }
    
    private void RenderPauseMenu()
    {
        // Semi-transparent background overlay
        Vector4 overlayColor = new Vector4(0.0f, 0.0f, 0.0f, 0.7f);
        _renderer.DrawRectFilled(Vector2.Zero, new Vector2(_screenWidth, _screenHeight), overlayColor);
        
        // Menu panel
        float panelWidth = 400f;
        float panelHeight = 500f;
        float panelX = (_screenWidth - panelWidth) * 0.5f;
        float panelY = (_screenHeight - panelHeight) * 0.5f;
        
        Vector4 panelBgColor = new Vector4(0.05f, 0.1f, 0.15f, 0.95f);
        Vector4 panelBorderColor = new Vector4(0.0f, 0.8f, 1.0f, 1.0f);
        Vector4 titleColor = new Vector4(0.2f, 1.0f, 0.8f, 1.0f);
        
        // Panel background
        _renderer.DrawRectFilled(new Vector2(panelX, panelY), new Vector2(panelWidth, panelHeight), panelBgColor);
        
        // Panel border
        _renderer.DrawRect(new Vector2(panelX, panelY), new Vector2(panelWidth, panelHeight), panelBorderColor, 3f);
        
        // Title bar
        float titleBarHeight = 60f;
        Vector4 titleBarColor = new Vector4(0.0f, 0.6f, 0.8f, 0.5f);
        _renderer.DrawRectFilled(new Vector2(panelX, panelY), new Vector2(panelWidth, titleBarHeight), titleBarColor);
        _renderer.DrawLine(
            new Vector2(panelX, panelY + titleBarHeight),
            new Vector2(panelX + panelWidth, panelY + titleBarHeight),
            panelBorderColor, 2f);
        
        // Text is rendered separately in RenderTextLabels() using ImGui
        
        // Menu items (rendered as buttons)
        string[] menuItems = { "Resume", "Settings", "Save Game", "Load Game", "Main Menu" };
        float buttonWidth = panelWidth - 80f;
        float buttonHeight = 50f;
        float buttonX = panelX + 40f;
        float buttonY = panelY + titleBarHeight + 40f;
        float buttonSpacing = 15f;
        
        Vector4 buttonColor = new Vector4(0.1f, 0.3f, 0.4f, 0.8f);
        Vector4 buttonHoverColor = new Vector4(0.2f, 0.5f, 0.6f, 0.9f);
        Vector4 buttonBorderColor = new Vector4(0.0f, 0.8f, 1.0f, 0.9f);
        
        for (int i = 0; i < menuItems.Length; i++)
        {
            float currentY = buttonY + i * (buttonHeight + buttonSpacing);
            
            // Button background (use hover color for selected item)
            Vector4 bgColor = (i == _selectedMenuItem) ? buttonHoverColor : buttonColor;
            _renderer.DrawRectFilled(new Vector2(buttonX, currentY), new Vector2(buttonWidth, buttonHeight), bgColor);
            
            // Button border (thicker for selected item)
            float borderThickness = (i == _selectedMenuItem) ? 3f : 2f;
            _renderer.DrawRect(new Vector2(buttonX, currentY), new Vector2(buttonWidth, buttonHeight), buttonBorderColor, borderThickness);
            
            // Selection indicator (arrow/marker on the left for selected item)
            if (i == _selectedMenuItem)
            {
                float markerSize = 20f;
                float markerX = buttonX - markerSize - 10f;
                float markerY = currentY + buttonHeight * 0.5f;
                
                // Draw a triangle pointing right
                Vector4 markerColor = new Vector4(0.0f, 1.0f, 0.8f, 1.0f);
                
                // Triangle vertices
                Vector2 point1 = new Vector2(markerX, markerY - markerSize * 0.4f);
                Vector2 point2 = new Vector2(markerX, markerY + markerSize * 0.4f);
                Vector2 point3 = new Vector2(markerX + markerSize * 0.7f, markerY);
                
                // Draw triangle outline
                _renderer.DrawLine(point1, point2, markerColor, 2f);
                _renderer.DrawLine(point2, point3, markerColor, 2f);
                _renderer.DrawLine(point3, point1, markerColor, 2f);
                
                // Fill triangle with horizontal lines
                const float fillLineSpacing = 1.5f;
                const float triangleSlope = 1.75f; // Calculated from triangle geometry: 0.7 / 0.4
                float triangleHeight = markerSize * 0.4f;
                
                for (float offset = 0; offset < triangleHeight; offset += fillLineSpacing)
                {
                    float xOffset = offset * triangleSlope;
                    Vector2 topPoint = new Vector2(markerX + xOffset, markerY - triangleHeight + offset);
                    Vector2 bottomPoint = new Vector2(markerX + xOffset, markerY + triangleHeight - offset);
                    _renderer.DrawLine(topPoint, bottomPoint, markerColor, fillLineSpacing);
                }
            }
            
            // Text is rendered separately in RenderTextLabels() using ImGui
        }
    }
    
    private void RenderSettingsMenu()
    {
        // Semi-transparent background overlay
        Vector4 overlayColor = new Vector4(0.0f, 0.0f, 0.0f, 0.7f);
        _renderer.DrawRectFilled(Vector2.Zero, new Vector2(_screenWidth, _screenHeight), overlayColor);
        
        // Settings panel (larger than pause menu)
        float panelWidth = 700f;
        float panelHeight = 600f;
        float panelX = (_screenWidth - panelWidth) * 0.5f;
        float panelY = (_screenHeight - panelHeight) * 0.5f;
        
        Vector4 panelBgColor = new Vector4(0.05f, 0.1f, 0.15f, 0.95f);
        Vector4 panelBorderColor = new Vector4(0.0f, 0.8f, 1.0f, 1.0f);
        
        // Panel background
        _renderer.DrawRectFilled(new Vector2(panelX, panelY), new Vector2(panelWidth, panelHeight), panelBgColor);
        
        // Panel border
        _renderer.DrawRect(new Vector2(panelX, panelY), new Vector2(panelWidth, panelHeight), panelBorderColor, 3f);
        
        // Title bar
        float titleBarHeight = 60f;
        Vector4 titleBarColor = new Vector4(0.0f, 0.6f, 0.8f, 0.5f);
        _renderer.DrawRectFilled(new Vector2(panelX, panelY), new Vector2(panelWidth, titleBarHeight), titleBarColor);
        _renderer.DrawLine(
            new Vector2(panelX, panelY + titleBarHeight),
            new Vector2(panelX + panelWidth, panelY + titleBarHeight),
            panelBorderColor, 2f);
        
        // Render tabs
        float tabY = panelY + titleBarHeight + 10f;
        float tabHeight = 40f;
        float tabWidth = (panelWidth - 80f) / 3f;
        float tabStartX = panelX + 40f;
        
        string[] tabs = { "Graphics", "Audio", "Gameplay" };
        Vector4 activeTabColor = new Vector4(0.0f, 0.6f, 0.8f, 0.8f);
        Vector4 inactiveTabColor = new Vector4(0.1f, 0.2f, 0.3f, 0.6f);
        
        for (int i = 0; i < tabs.Length; i++)
        {
            float tabX = tabStartX + i * tabWidth;
            Vector4 tabColor = (i == _selectedSettingsTab) ? activeTabColor : inactiveTabColor;
            
            _renderer.DrawRectFilled(new Vector2(tabX, tabY), new Vector2(tabWidth - 5f, tabHeight), tabColor);
            _renderer.DrawRect(new Vector2(tabX, tabY), new Vector2(tabWidth - 5f, tabHeight), panelBorderColor, 2f);
        }
        
        // Settings items area
        float settingsY = tabY + tabHeight + 20f;
        float settingHeight = 50f;
        float settingSpacing = 15f;
        float settingX = panelX + 50f;
        float settingWidth = panelWidth - 100f;
        
        Vector4 settingBgColor = new Vector4(0.1f, 0.2f, 0.3f, 0.6f);
        Vector4 settingSelectedColor = new Vector4(0.2f, 0.4f, 0.5f, 0.8f);
        
        // Render 4 setting items
        for (int i = 0; i < _settingsPerTab; i++)
        {
            float currentY = settingsY + i * (settingHeight + settingSpacing);
            Vector4 bgColor = (i == _selectedSetting) ? settingSelectedColor : settingBgColor;
            
            _renderer.DrawRectFilled(new Vector2(settingX, currentY), new Vector2(settingWidth, settingHeight), bgColor);
            _renderer.DrawRect(new Vector2(settingX, currentY), new Vector2(settingWidth, settingHeight), panelBorderColor, 1.5f);
            
            // Selection marker for selected setting
            if (i == _selectedSetting)
            {
                float markerSize = 15f;
                float markerX = settingX - markerSize - 8f;
                float markerY = currentY + settingHeight * 0.5f;
                
                Vector4 markerColor = new Vector4(0.0f, 1.0f, 0.8f, 1.0f);
                Vector2 p1 = new Vector2(markerX, markerY - markerSize * 0.3f);
                Vector2 p2 = new Vector2(markerX, markerY + markerSize * 0.3f);
                Vector2 p3 = new Vector2(markerX + markerSize * 0.6f, markerY);
                
                _renderer.DrawLine(p1, p2, markerColor, 2f);
                _renderer.DrawLine(p2, p3, markerColor, 2f);
                _renderer.DrawLine(p3, p1, markerColor, 2f);
            }
        }
        
        // Back button at bottom
        float backButtonY = panelY + panelHeight - 70f;
        float backButtonWidth = 200f;
        float backButtonHeight = 40f;
        float backButtonX = panelX + (panelWidth - backButtonWidth) * 0.5f;
        
        Vector4 buttonColor = new Vector4(0.1f, 0.3f, 0.4f, 0.8f);
        _renderer.DrawRectFilled(new Vector2(backButtonX, backButtonY), new Vector2(backButtonWidth, backButtonHeight), buttonColor);
        _renderer.DrawRect(new Vector2(backButtonX, backButtonY), new Vector2(backButtonWidth, backButtonHeight), panelBorderColor, 2f);
    }
    
    private void RenderSaveDialog()
    {
        // Semi-transparent background overlay
        Vector4 overlayColor = new Vector4(0.0f, 0.0f, 0.0f, 0.7f);
        _renderer.DrawRectFilled(Vector2.Zero, new Vector2(_screenWidth, _screenHeight), overlayColor);
        
        // Dialog panel
        float panelWidth = 500f;
        float panelHeight = 400f;
        float panelX = (_screenWidth - panelWidth) * 0.5f;
        float panelY = (_screenHeight - panelHeight) * 0.5f;
        
        Vector4 panelBgColor = new Vector4(0.05f, 0.1f, 0.15f, 0.95f);
        Vector4 panelBorderColor = new Vector4(0.0f, 0.8f, 1.0f, 1.0f);
        
        // Panel background and border
        _renderer.DrawRectFilled(new Vector2(panelX, panelY), new Vector2(panelWidth, panelHeight), panelBgColor);
        _renderer.DrawRect(new Vector2(panelX, panelY), new Vector2(panelWidth, panelHeight), panelBorderColor, 3f);
        
        // Title bar
        float titleBarHeight = 60f;
        Vector4 titleBarColor = new Vector4(0.0f, 0.6f, 0.8f, 0.5f);
        _renderer.DrawRectFilled(new Vector2(panelX, panelY), new Vector2(panelWidth, titleBarHeight), titleBarColor);
        _renderer.DrawLine(
            new Vector2(panelX, panelY + titleBarHeight),
            new Vector2(panelX + panelWidth, panelY + titleBarHeight),
            panelBorderColor, 2f);
        
        // Save slots area
        float contentY = panelY + titleBarHeight + 20f;
        float slotHeight = 50f;
        float slotSpacing = 10f;
        float slotX = panelX + 30f;
        float slotWidth = panelWidth - 60f;
        
        var saveManager = SaveGameManager.Instance;
        var saves = saveManager.ListSaveGames();
        int displaySlots = Math.Min(5, saves.Count + 1); // Show up to 5 slots
        
        Vector4 slotBgColor = new Vector4(0.1f, 0.2f, 0.3f, 0.6f);
        Vector4 slotSelectedColor = new Vector4(0.2f, 0.4f, 0.5f, 0.8f);
        
        for (int i = 0; i < displaySlots; i++)
        {
            float slotY = contentY + i * (slotHeight + slotSpacing);
            Vector4 bgColor = (i == _selectedSaveSlot) ? slotSelectedColor : slotBgColor;
            
            _renderer.DrawRectFilled(new Vector2(slotX, slotY), new Vector2(slotWidth, slotHeight), bgColor);
            _renderer.DrawRect(new Vector2(slotX, slotY), new Vector2(slotWidth, slotHeight), panelBorderColor, 2f);
            
            // Selection marker
            if (i == _selectedSaveSlot)
            {
                float markerSize = 12f;
                float markerX = slotX - markerSize - 6f;
                float markerY = slotY + slotHeight * 0.5f;
                Vector4 markerColor = new Vector4(0.0f, 1.0f, 0.8f, 1.0f);
                Vector2 p1 = new Vector2(markerX, markerY - markerSize * 0.3f);
                Vector2 p2 = new Vector2(markerX, markerY + markerSize * 0.3f);
                Vector2 p3 = new Vector2(markerX + markerSize * 0.6f, markerY);
                _renderer.DrawLine(p1, p2, markerColor, 2f);
                _renderer.DrawLine(p2, p3, markerColor, 2f);
                _renderer.DrawLine(p3, p1, markerColor, 2f);
            }
        }
    }
    
    private void RenderLoadDialog()
    {
        // Semi-transparent background overlay
        Vector4 overlayColor = new Vector4(0.0f, 0.0f, 0.0f, 0.7f);
        _renderer.DrawRectFilled(Vector2.Zero, new Vector2(_screenWidth, _screenHeight), overlayColor);
        
        // Dialog panel
        float panelWidth = 500f;
        float panelHeight = 400f;
        float panelX = (_screenWidth - panelWidth) * 0.5f;
        float panelY = (_screenHeight - panelHeight) * 0.5f;
        
        Vector4 panelBgColor = new Vector4(0.05f, 0.1f, 0.15f, 0.95f);
        Vector4 panelBorderColor = new Vector4(0.0f, 0.8f, 1.0f, 1.0f);
        
        // Panel background and border
        _renderer.DrawRectFilled(new Vector2(panelX, panelY), new Vector2(panelWidth, panelHeight), panelBgColor);
        _renderer.DrawRect(new Vector2(panelX, panelY), new Vector2(panelWidth, panelHeight), panelBorderColor, 3f);
        
        // Title bar
        float titleBarHeight = 60f;
        Vector4 titleBarColor = new Vector4(0.0f, 0.6f, 0.8f, 0.5f);
        _renderer.DrawRectFilled(new Vector2(panelX, panelY), new Vector2(panelWidth, titleBarHeight), titleBarColor);
        _renderer.DrawLine(
            new Vector2(panelX, panelY + titleBarHeight),
            new Vector2(panelX + panelWidth, panelY + titleBarHeight),
            panelBorderColor, 2f);
        
        // Load slots area
        float contentY = panelY + titleBarHeight + 20f;
        float slotHeight = 50f;
        float slotSpacing = 10f;
        float slotX = panelX + 30f;
        float slotWidth = panelWidth - 60f;
        
        var saveManager = SaveGameManager.Instance;
        var saves = saveManager.ListSaveGames();
        int displaySlots = Math.Min(5, saves.Count);
        
        if (saves.Count > 0)
        {
            Vector4 slotBgColor = new Vector4(0.1f, 0.2f, 0.3f, 0.6f);
            Vector4 slotSelectedColor = new Vector4(0.2f, 0.4f, 0.5f, 0.8f);
            
            for (int i = 0; i < displaySlots; i++)
            {
                float slotY = contentY + i * (slotHeight + slotSpacing);
                Vector4 bgColor = (i == _selectedSaveSlot) ? slotSelectedColor : slotBgColor;
                
                _renderer.DrawRectFilled(new Vector2(slotX, slotY), new Vector2(slotWidth, slotHeight), bgColor);
                _renderer.DrawRect(new Vector2(slotX, slotY), new Vector2(slotWidth, slotHeight), panelBorderColor, 2f);
                
                // Selection marker
                if (i == _selectedSaveSlot)
                {
                    float markerSize = 12f;
                    float markerX = slotX - markerSize - 6f;
                    float markerY = slotY + slotHeight * 0.5f;
                    Vector4 markerColor = new Vector4(0.0f, 1.0f, 0.8f, 1.0f);
                    Vector2 p1 = new Vector2(markerX, markerY - markerSize * 0.3f);
                    Vector2 p2 = new Vector2(markerX, markerY + markerSize * 0.3f);
                    Vector2 p3 = new Vector2(markerX + markerSize * 0.6f, markerY);
                    _renderer.DrawLine(p1, p2, markerColor, 2f);
                    _renderer.DrawLine(p2, p3, markerColor, 2f);
                    _renderer.DrawLine(p3, p1, markerColor, 2f);
                }
            }
        }
    }
    
    private void RenderSaveDialogText(ImDrawListPtr drawList)
    {
        float panelWidth = 500f;
        float panelHeight = 400f;
        float panelX = (_screenWidth - panelWidth) * 0.5f;
        float panelY = (_screenHeight - panelHeight) * 0.5f;
        float titleBarHeight = 60f;
        
        // Title
        string titleText = "SAVE GAME";
        var titleSize = ImGui.CalcTextSize(titleText);
        float titleX = panelX + (panelWidth - titleSize.X) * 0.5f;
        float titleY = panelY + (titleBarHeight - titleSize.Y) * 0.5f;
        
        uint shadowColor = ImGui.ColorConvertFloat4ToU32(new Vector4(0f, 0f, 0f, 0.5f));
        uint titleColor = ImGui.ColorConvertFloat4ToU32(new Vector4(0.2f, 1.0f, 0.8f, 1.0f));
        drawList.AddText(new Vector2(titleX + 2, titleY + 2), shadowColor, titleText);
        drawList.AddText(new Vector2(titleX, titleY), titleColor, titleText);
        
        // Save slots
        float contentY = panelY + titleBarHeight + 20f;
        float slotHeight = 50f;
        float slotSpacing = 10f;
        float slotX = panelX + 30f;
        
        var saveManager = SaveGameManager.Instance;
        var saves = saveManager.ListSaveGames();
        uint labelColor = ImGui.ColorConvertFloat4ToU32(new Vector4(0.7f, 0.9f, 1.0f, 1.0f));
        uint selectedColor = ImGui.ColorConvertFloat4ToU32(new Vector4(1.0f, 1.0f, 1.0f, 1.0f));
        
        int displaySlots = Math.Min(5, saves.Count + 1);
        for (int i = 0; i < displaySlots; i++)
        {
            float slotY = contentY + i * (slotHeight + slotSpacing);
            float textY = slotY + (slotHeight - ImGui.GetTextLineHeight()) * 0.5f;
            uint color = (i == _selectedSaveSlot) ? selectedColor : labelColor;
            
            string slotText = (i < saves.Count) ? saves[i].SaveName : "[New Save]";
            drawList.AddText(new Vector2(slotX + 15f, textY), color, slotText);
            
            if (i < saves.Count)
            {
                string dateText = saves[i].SaveTime.ToLocalTime().ToString("yyyy-MM-dd HH:mm");
                var dateSize = ImGui.CalcTextSize(dateText);
                drawList.AddText(new Vector2(slotX + 380f - dateSize.X, textY), color, dateText);
            }
        }
        
        // Instructions
        float instructY = panelY + panelHeight - 50f;
        string instr = "Up/Down: Select  |  Enter: Save  |  Backspace: Cancel";
        uint instructColor = ImGui.ColorConvertFloat4ToU32(new Vector4(0.5f, 0.6f, 0.7f, 0.8f));
        var instrSize = ImGui.CalcTextSize(instr);
        drawList.AddText(new Vector2(panelX + (panelWidth - instrSize.X) * 0.5f, instructY), instructColor, instr);
    }
    
    private void RenderLoadDialogText(ImDrawListPtr drawList)
    {
        float panelWidth = 500f;
        float panelHeight = 400f;
        float panelX = (_screenWidth - panelWidth) * 0.5f;
        float panelY = (_screenHeight - panelHeight) * 0.5f;
        float titleBarHeight = 60f;
        
        // Title
        string titleText = "LOAD GAME";
        var titleSize = ImGui.CalcTextSize(titleText);
        float titleX = panelX + (panelWidth - titleSize.X) * 0.5f;
        float titleY = panelY + (titleBarHeight - titleSize.Y) * 0.5f;
        
        uint shadowColor = ImGui.ColorConvertFloat4ToU32(new Vector4(0f, 0f, 0f, 0.5f));
        uint titleColor = ImGui.ColorConvertFloat4ToU32(new Vector4(0.2f, 1.0f, 0.8f, 1.0f));
        drawList.AddText(new Vector2(titleX + 2, titleY + 2), shadowColor, titleText);
        drawList.AddText(new Vector2(titleX, titleY), titleColor, titleText);
        
        // Save slots
        var saveManager = SaveGameManager.Instance;
        var saves = saveManager.ListSaveGames();
        
        if (saves.Count == 0)
        {
            // No saves message
            string noSavesText = "No saved games found";
            var textSize = ImGui.CalcTextSize(noSavesText);
            float textX = panelX + (panelWidth - textSize.X) * 0.5f;
            float textY = panelY + panelHeight * 0.5f;
            uint textColor = ImGui.ColorConvertFloat4ToU32(new Vector4(0.7f, 0.7f, 0.7f, 1.0f));
            drawList.AddText(new Vector2(textX, textY), textColor, noSavesText);
        }
        else
        {
            float contentY = panelY + titleBarHeight + 20f;
            float slotHeight = 50f;
            float slotSpacing = 10f;
            float slotX = panelX + 30f;
            
            uint labelColor = ImGui.ColorConvertFloat4ToU32(new Vector4(0.7f, 0.9f, 1.0f, 1.0f));
            uint selectedColor = ImGui.ColorConvertFloat4ToU32(new Vector4(1.0f, 1.0f, 1.0f, 1.0f));
            
            int displaySlots = Math.Min(5, saves.Count);
            for (int i = 0; i < displaySlots; i++)
            {
                float slotY = contentY + i * (slotHeight + slotSpacing);
                float textY = slotY + (slotHeight - ImGui.GetTextLineHeight()) * 0.5f;
                uint color = (i == _selectedSaveSlot) ? selectedColor : labelColor;
                
                string slotText = saves[i].SaveName;
                drawList.AddText(new Vector2(slotX + 15f, textY), color, slotText);
                
                string dateText = saves[i].SaveTime.ToLocalTime().ToString("yyyy-MM-dd HH:mm");
                var dateSize = ImGui.CalcTextSize(dateText);
                drawList.AddText(new Vector2(slotX + 380f - dateSize.X, textY), color, dateText);
            }
        }
        
        // Instructions
        float instructY = panelY + panelHeight - 50f;
        string instr = saves.Count > 0 ? "Up/Down: Select  |  Enter: Load  |  Backspace: Cancel" : "Press any key to return";
        uint instructColor = ImGui.ColorConvertFloat4ToU32(new Vector4(0.5f, 0.6f, 0.7f, 0.8f));
        var instrSize = ImGui.CalcTextSize(instr);
        drawList.AddText(new Vector2(panelX + (panelWidth - instrSize.X) * 0.5f, instructY), instructColor, instr);
    }
    
    private void RenderConfirmDialog()
    {
        // Semi-transparent background overlay
        Vector4 overlayColor = new Vector4(0.0f, 0.0f, 0.0f, 0.7f);
        _renderer.DrawRectFilled(Vector2.Zero, new Vector2(_screenWidth, _screenHeight), overlayColor);
        
        // Dialog panel
        float dialogWidth = 400f;
        float dialogHeight = 200f;
        float dialogX = (_screenWidth - dialogWidth) * 0.5f;
        float dialogY = (_screenHeight - dialogHeight) * 0.5f;
        
        Vector4 panelBgColor = new Vector4(0.05f, 0.1f, 0.15f, 0.95f);
        Vector4 panelBorderColor = new Vector4(0.0f, 0.8f, 1.0f, 1.0f);
        
        // Panel background and border
        _renderer.DrawRectFilled(new Vector2(dialogX, dialogY), new Vector2(dialogWidth, dialogHeight), panelBgColor);
        _renderer.DrawRect(new Vector2(dialogX, dialogY), new Vector2(dialogWidth, dialogHeight), panelBorderColor, 3f);
        
        // Render Yes/No buttons
        float buttonWidth = 120f;
        float buttonHeight = 40f;
        float buttonSpacing = 20f;
        float totalButtonWidth = buttonWidth * 2 + buttonSpacing;
        float buttonY = dialogY + dialogHeight - buttonHeight - 30f;
        float yesButtonX = dialogX + (dialogWidth - totalButtonWidth) * 0.5f;
        float noButtonX = yesButtonX + buttonWidth + buttonSpacing;
        
        // Handle keyboard input
        if (_keysPressed.Contains(Key.Enter) && !_keysPressedLastFrame.Contains(Key.Enter))
        {
            // Yes selected - execute confirm action
            _confirmAction?.Invoke();
            _isConfirmDialogOpen = false;
            _confirmAction = null;
        }
        else if (_keysPressed.Contains(Key.Escape) || 
                (_keysPressed.Contains(Key.Backspace) && !_keysPressedLastFrame.Contains(Key.Backspace)))
        {
            // Cancel
            _isConfirmDialogOpen = false;
            _isPauseMenuOpen = true;
            _confirmAction = null;
        }
        
        // Render Yes button (selected)
        Vector4 selectedButtonColor = new Vector4(0.2f, 0.6f, 0.8f, 0.8f);
        Vector4 buttonColor = new Vector4(0.1f, 0.2f, 0.3f, 0.6f);
        Vector4 buttonBorderColor = new Vector4(0.0f, 0.8f, 1.0f, 1.0f);
        
        _renderer.DrawRectFilled(new Vector2(yesButtonX, buttonY), new Vector2(buttonWidth, buttonHeight), selectedButtonColor);
        _renderer.DrawRect(new Vector2(yesButtonX, buttonY), new Vector2(buttonWidth, buttonHeight), buttonBorderColor, 2f);
        
        // Render No button
        _renderer.DrawRectFilled(new Vector2(noButtonX, buttonY), new Vector2(buttonWidth, buttonHeight), buttonColor);
        _renderer.DrawRect(new Vector2(noButtonX, buttonY), new Vector2(buttonWidth, buttonHeight), buttonBorderColor, 2f);
    }
    
    private void RenderConfirmDialogText(ImDrawListPtr drawList)
    {
        float dialogWidth = 400f;
        float dialogHeight = 200f;
        float dialogX = (_screenWidth - dialogWidth) * 0.5f;
        float dialogY = (_screenHeight - dialogHeight) * 0.5f;
        
        // Render message
        var lines = _confirmDialogMessage.Split('\n');
        float lineHeight = ImGui.GetTextLineHeight();
        float totalHeight = lines.Length * lineHeight;
        float startY = dialogY + (dialogHeight - totalHeight - 80f) * 0.5f;
        
        uint textColor = ImGui.ColorConvertFloat4ToU32(new Vector4(0.9f, 0.9f, 0.9f, 1.0f));
        
        for (int i = 0; i < lines.Length; i++)
        {
            var lineSize = ImGui.CalcTextSize(lines[i]);
            float lineX = dialogX + (dialogWidth - lineSize.X) * 0.5f;
            float lineY = startY + i * lineHeight;
            drawList.AddText(new Vector2(lineX, lineY), textColor, lines[i]);
        }
        
        // Render button labels
        float buttonY = dialogY + dialogHeight - 55f;
        
        // Yes button label
        string yesText = "Yes";
        var yesSize = ImGui.CalcTextSize(yesText);
        float yesX = dialogX + (dialogWidth * 0.5f - 140f);
        uint yesColor = ImGui.ColorConvertFloat4ToU32(new Vector4(1.0f, 1.0f, 1.0f, 1.0f));
        drawList.AddText(new Vector2(yesX, buttonY), yesColor, yesText);
        
        // No button label
        string noText = "No";
        var noSize = ImGui.CalcTextSize(noText);
        float noX = dialogX + (dialogWidth * 0.5f + 20f + (120f - noSize.X) * 0.5f);
        uint noColor = ImGui.ColorConvertFloat4ToU32(new Vector4(0.7f, 0.7f, 0.7f, 1.0f));
        drawList.AddText(new Vector2(noX, buttonY), noColor, noText);
        
        // Instructions
        string instr = "Enter: Yes  |  Esc: No";
        uint instructColor = ImGui.ColorConvertFloat4ToU32(new Vector4(0.5f, 0.6f, 0.7f, 0.8f));
        var instrSize = ImGui.CalcTextSize(instr);
        drawList.AddText(new Vector2(dialogX + (dialogWidth - instrSize.X) * 0.5f, dialogY + dialogHeight - 20f), instructColor, instr);
    }
}
