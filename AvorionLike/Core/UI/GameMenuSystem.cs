using System.Numerics;
using Silk.NET.Input;

namespace AvorionLike.Core.UI;

/// <summary>
/// Custom-rendered game menu system (pause menu, settings, etc.)
/// Uses CustomUIRenderer instead of ImGui
/// </summary>
public class GameMenuSystem
{
    private readonly GameEngine _gameEngine;
    private readonly CustomUIRenderer _renderer;
    private float _screenWidth;
    private float _screenHeight;
    
    private bool _isPauseMenuOpen = false;
    private bool _isSettingsMenuOpen = false;
    private int _selectedMenuItem = 0;
    
#pragma warning disable CS0414 // Field is assigned but its value is never used - reserved for future settings tab implementation
    private int _selectedSettingsTab = 0;
#pragma warning restore CS0414
    
    // Settings values
    private float _difficulty = 1.0f;
    
#pragma warning disable CS0414 // Field is assigned but its value is never used - reserved for future settings implementation
    private float _mouseSensitivity = 0.5f;
    private float _masterVolume = 1.0f;
#pragma warning restore CS0414
    
    public bool IsMenuOpen => _isPauseMenuOpen || _isSettingsMenuOpen;
    
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
    
    public void HandleInput(IKeyboard keyboard)
    {
        // ESC key handling is done externally
        
        if (!IsMenuOpen) return;
        
        // TODO: Handle keyboard navigation through menu items
        // For now, this is a placeholder - full keyboard/mouse menu interaction
        // would require more sophisticated input handling
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
        
        _renderer.EndRender();
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
        
        // TODO: Add text rendering for "PAUSED" title
        
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
            
            // Button border
            _renderer.DrawRect(new Vector2(buttonX, currentY), new Vector2(buttonWidth, buttonHeight), buttonBorderColor, 2f);
            
            // TODO: Render button text
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
        
        // TODO: Add tabs for Video/Sound/Gameplay settings
        // TODO: Add sliders for difficulty, volume, etc.
        // TODO: Add "Back" button
        
        // For now, just render a placeholder settings panel
        float contentY = panelY + titleBarHeight + 30f;
        float sliderX = panelX + 50f;
        float sliderWidth = panelWidth - 100f;
        float sliderHeight = 10f;
        
        // Difficulty slider placeholder
        Vector4 sliderBgColor = new Vector4(0.1f, 0.1f, 0.1f, 0.8f);
        Vector4 sliderFillColor = new Vector4(0.0f, 0.8f, 1.0f, 0.9f);
        
        _renderer.DrawRectFilled(new Vector2(sliderX, contentY), new Vector2(sliderWidth, sliderHeight), sliderBgColor);
        _renderer.DrawRectFilled(new Vector2(sliderX, contentY), new Vector2(sliderWidth * _difficulty / 2.0f, sliderHeight), sliderFillColor);
        _renderer.DrawRect(new Vector2(sliderX, contentY), new Vector2(sliderWidth, sliderHeight), panelBorderColor, 1.5f);
    }
}
