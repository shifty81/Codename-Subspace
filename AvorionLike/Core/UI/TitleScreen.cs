using ImGuiNET;
using System.Numerics;

namespace AvorionLike.Core.UI;

/// <summary>
/// Title screen shown when the game first launches
/// Displays game title, version, and start prompts
/// </summary>
public class TitleScreen
{
    private readonly GameEngine _gameEngine;
    private bool _isActive = true;
    private float _titlePulse = 0f;
    private readonly string[] _stars = new string[] { "⭐", "✨", "🌟", "💫" };
    private readonly Random _random = new();
    private bool _showSettings = false;
    
    // Settings values (read from config on construction)
    private float _masterVolume;
    private float _musicVolume;
    private float _sfxVolume;
    private int _targetFPS = 60;
    private bool _vsync = true;
    
    public bool IsActive => _isActive;
    
    // Callback for when new game is requested
    public Action? OnNewGameRequested { get; set; }
    
    // Callback for when settings menu is requested
    public Action? OnSettingsRequested { get; set; }
    
    public TitleScreen(GameEngine gameEngine)
    {
        _gameEngine = gameEngine;
        var config = Configuration.ConfigurationManager.Instance.Config;
        _masterVolume = config.Audio.MasterVolume;
        _musicVolume = config.Audio.MusicVolume;
        _sfxVolume = config.Audio.SfxVolume;
    }
    
    public void Update(float deltaTime)
    {
        _titlePulse += deltaTime;
    }
    
    public void Render()
    {
        if (!_isActive) return;
        
        var io = ImGui.GetIO();
        
        // Full-screen semi-transparent overlay
        ImGui.SetNextWindowPos(Vector2.Zero);
        ImGui.SetNextWindowSize(io.DisplaySize);
        
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags.NoDecoration | 
                                       ImGuiWindowFlags.NoMove | 
                                       ImGuiWindowFlags.NoSavedSettings |
                                       ImGuiWindowFlags.NoBackground;
        
        if (ImGui.Begin("TitleScreen", windowFlags))
        {
            // Dark background overlay
            var drawList = ImGui.GetWindowDrawList();
            drawList.AddRectFilled(
                Vector2.Zero,
                io.DisplaySize,
                ImGui.ColorConvertFloat4ToU32(new Vector4(0, 0, 0, 0.85f))
            );
            
            float centerX = io.DisplaySize.X * 0.5f;
            float centerY = io.DisplaySize.Y * 0.5f;
            
            // Animated title
            ImGui.SetCursorPos(new Vector2(centerX, centerY - 200));
            
            // Main title with pulsing effect
            float pulseScale = 1.0f + MathF.Sin(_titlePulse * 2f) * 0.05f;
            ImGui.SetWindowFontScale(3.0f * pulseScale);
            
            var titleText = "CODENAME:SUBSPACE";
            var titleSize = ImGui.CalcTextSize(titleText);
            ImGui.SetCursorPos(new Vector2(centerX - titleSize.X * 0.5f, centerY - 200));
            
            // Gradient-like title effect with multiple colors
            ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.3f, 0.8f, 1.0f, 1.0f));
            ImGui.Text(titleText);
            ImGui.PopStyleColor();
            
            ImGui.SetWindowFontScale(1.0f);
            
            // Subtitle
            ImGui.SetCursorPos(new Vector2(centerX, centerY - 130));
            var subtitleText = "A Voxel-Based Space Exploration Game";
            var subtitleSize = ImGui.CalcTextSize(subtitleText);
            ImGui.SetCursorPos(new Vector2(centerX - subtitleSize.X * 0.5f, centerY - 130));
            ImGui.TextColored(new Vector4(0.7f, 0.7f, 0.8f, 0.9f), subtitleText);
            
            // Version info
            ImGui.SetCursorPos(new Vector2(centerX, centerY - 100));
            var versionText = "Version 1.0 - Early Access";
            var versionSize = ImGui.CalcTextSize(versionText);
            ImGui.SetCursorPos(new Vector2(centerX - versionSize.X * 0.5f, centerY - 100));
            ImGui.TextColored(new Vector4(0.5f, 0.5f, 0.6f, 0.8f), versionText);
            
            // Decorative stars
            DrawDecorativeStars(centerX, centerY - 250, drawList);
            
            ImGui.Dummy(new Vector2(0, 50));
            
            // Menu buttons
            float buttonWidth = 300f;
            float buttonHeight = 50f;
            float buttonSpacing = 15f;
            
            ImGui.PushStyleVar(ImGuiStyleVar.FramePadding, new Vector2(20, 15));
            ImGui.PushStyleColor(ImGuiCol.Button, new Vector4(0.1f, 0.3f, 0.5f, 0.8f));
            ImGui.PushStyleColor(ImGuiCol.ButtonHovered, new Vector4(0.2f, 0.5f, 0.8f, 0.9f));
            ImGui.PushStyleColor(ImGuiCol.ButtonActive, new Vector4(0.3f, 0.6f, 0.9f, 1.0f));
            ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(1.0f, 1.0f, 1.0f, 1.0f));
            
            // New Game button
            ImGui.SetCursorPos(new Vector2(centerX - buttonWidth * 0.5f, centerY + 50));
            if (ImGui.Button("🚀 START NEW GAME", new Vector2(buttonWidth, buttonHeight)))
            {
                OnNewGameRequested?.Invoke();
            }
            
            // Settings button
            ImGui.SetCursorPos(new Vector2(centerX - buttonWidth * 0.5f, centerY + 50 + buttonHeight + buttonSpacing));
            if (ImGui.Button("⚙️ SETTINGS", new Vector2(buttonWidth, buttonHeight)))
            {
                _showSettings = true;
                OnSettingsRequested?.Invoke();
            }
            
            // Exit button
            ImGui.SetCursorPos(new Vector2(centerX - buttonWidth * 0.5f, centerY + 50 + (buttonHeight + buttonSpacing) * 2));
            if (ImGui.Button("🚪 EXIT", new Vector2(buttonWidth, buttonHeight)))
            {
                Environment.Exit(0);
            }
            
            ImGui.PopStyleColor(4);
            ImGui.PopStyleVar();
            
            // Feature highlights
            ImGui.SetCursorPos(new Vector2(centerX, centerY + 120));
            RenderFeatureHighlights(centerX, centerY + 120);
            
            // Credits at bottom
            ImGui.SetCursorPos(new Vector2(centerX, io.DisplaySize.Y - 50));
            var creditsText = "Built with C# & .NET 9.0 | OpenGL Graphics | ImGui UI";
            var creditsSize = ImGui.CalcTextSize(creditsText);
            ImGui.SetCursorPos(new Vector2(centerX - creditsSize.X * 0.5f, io.DisplaySize.Y - 50));
            ImGui.TextColored(new Vector4(0.4f, 0.4f, 0.5f, 0.7f), creditsText);
        }
        ImGui.End();
        
        if (_showSettings)
        {
            RenderSettingsPanel();
        }
    }
    
    private void RenderSettingsPanel()
    {
        var io = ImGui.GetIO();
        ImGui.SetNextWindowPos(new Vector2(io.DisplaySize.X * 0.5f, io.DisplaySize.Y * 0.5f),
            ImGuiCond.Always, new Vector2(0.5f, 0.5f));
        ImGui.SetNextWindowSize(new Vector2(500, 450), ImGuiCond.FirstUseEver);
        
        if (ImGui.Begin("Settings", ref _showSettings))
        {
            if (ImGui.BeginTabBar("TitleSettingsTabs"))
            {
                if (ImGui.BeginTabItem("Video"))
                {
                    ImGui.Dummy(new Vector2(0, 10));
                    ImGui.Checkbox("VSync", ref _vsync);
                    ImGui.Dummy(new Vector2(0, 10));
                    ImGui.Text($"Target FPS: {_targetFPS}");
                    ImGui.SliderInt("##TargetFPS", ref _targetFPS, 30, 144);
                    ImGui.EndTabItem();
                }
                
                if (ImGui.BeginTabItem("Audio"))
                {
                    ImGui.Dummy(new Vector2(0, 10));
                    ImGui.Text($"Master Volume: {_masterVolume * 100:F0}%");
                    ImGui.SliderFloat("##MasterVol", ref _masterVolume, 0.0f, 1.0f);
                    ImGui.Dummy(new Vector2(0, 10));
                    ImGui.Text($"Music Volume: {_musicVolume * 100:F0}%");
                    ImGui.SliderFloat("##MusicVol", ref _musicVolume, 0.0f, 1.0f);
                    ImGui.Dummy(new Vector2(0, 10));
                    ImGui.Text($"SFX Volume: {_sfxVolume * 100:F0}%");
                    ImGui.SliderFloat("##SfxVol", ref _sfxVolume, 0.0f, 1.0f);
                    ImGui.Dummy(new Vector2(0, 10));
                    ImGui.TextWrapped("Audio playback will be implemented in future updates.");
                    ImGui.EndTabItem();
                }
                
                if (ImGui.BeginTabItem("Controls"))
                {
                    ImGui.Dummy(new Vector2(0, 10));
                    ImGui.Text("Camera Controls:");
                    ImGui.BulletText("WASD - Move horizontally");
                    ImGui.BulletText("Space - Move up");
                    ImGui.BulletText("Shift - Move down");
                    ImGui.BulletText("Mouse - Look around");
                    ImGui.Dummy(new Vector2(0, 10));
                    ImGui.Text("UI Controls:");
                    ImGui.BulletText("ESC - Pause Menu");
                    ImGui.BulletText("F1 - Debug Info");
                    ImGui.BulletText("H - Tutorial Overlay");
                    ImGui.BulletText("J - Quest Log");
                    ImGui.EndTabItem();
                }
                
                ImGui.EndTabBar();
            }
            
            ImGui.Dummy(new Vector2(0, 15));
            
            float buttonWidth = 120f;
            float totalWidth = buttonWidth * 2 + 10;
            ImGui.SetCursorPosX((ImGui.GetWindowWidth() - totalWidth) * 0.5f);
            
            if (ImGui.Button("Apply", new Vector2(buttonWidth, 30)))
            {
                var config = Configuration.ConfigurationManager.Instance.Config;
                config.Audio.MasterVolume = _masterVolume;
                config.Audio.MusicVolume = _musicVolume;
                config.Audio.SfxVolume = _sfxVolume;
            }
            ImGui.SameLine(0, 10);
            if (ImGui.Button("Back", new Vector2(buttonWidth, 30)))
            {
                _showSettings = false;
            }
        }
        ImGui.End();
    }
    
    private void DrawDecorativeStars(float centerX, float centerY, ImDrawListPtr drawList)
    {
        // Draw some decorative stars around the title
        for (int i = 0; i < 6; i++)
        {
            float angle = (i / 6f) * MathF.PI * 2f + _titlePulse * 0.5f;
            float radius = 150f + MathF.Sin(_titlePulse * 2f + i) * 20f;
            float x = centerX + MathF.Cos(angle) * radius;
            float y = centerY + MathF.Sin(angle) * radius;
            
            float size = 3f + MathF.Sin(_titlePulse * 3f + i) * 1.5f;
            uint color = ImGui.ColorConvertFloat4ToU32(new Vector4(0.8f, 0.9f, 1.0f, 0.6f));
            
            // Draw a star shape
            drawList.AddCircleFilled(new Vector2(x, y), size, color);
        }
    }
    
    private void RenderFeatureHighlights(float centerX, float startY)
    {
        string[] features = new[]
        {
            "🚀 Fully controllable player ship with 6DOF movement",
            "🏗️ Dynamic ship building with voxel blocks",
            "🌌 Procedurally generated galaxy to explore",
            "⚔️ Combat system with shields and weapons",
            "📦 Resource management and trading"
        };
        
        float lineHeight = 25f;
        
        for (int i = 0; i < features.Length; i++)
        {
            var featureSize = ImGui.CalcTextSize(features[i]);
            ImGui.SetCursorPos(new Vector2(centerX - featureSize.X * 0.5f, startY + i * lineHeight));
            ImGui.TextColored(new Vector4(0.6f, 0.8f, 0.9f, 0.8f), features[i]);
        }
    }
    
    public void HandleInput()
    {
        if (!_isActive) return;
        
        // Input is handled through ImGui buttons
        // ESC key can still dismiss
        if (ImGui.IsKeyPressed(ImGuiKey.Escape))
        {
            Environment.Exit(0);
        }
    }
    
    public void Dismiss()
    {
        _isActive = false;
    }
    
    public void Show()
    {
        _isActive = true;
        _titlePulse = 0f;
    }
}
