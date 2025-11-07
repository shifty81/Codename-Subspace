using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Physics;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Resources;
using ImGuiNET;

namespace AvorionLike.Core.UI;

/// <summary>
/// Main game HUD with both custom OpenGL rendering and ImGui text
/// Custom renderer for shapes/graphics, ImGui for text labels
/// </summary>
public class GameHUD
{
    private readonly GameEngine _gameEngine;
    private readonly CustomUIRenderer _renderer;
    private bool _enabled = true;
    private Guid? _playerShipId;
    private float _screenWidth;
    private float _screenHeight;
    
    public bool Enabled
    {
        get => _enabled;
        set => _enabled = value;
    }
    
    public Guid? PlayerShipId
    {
        get => _playerShipId;
        set => _playerShipId = value;
    }
    
    public GameHUD(GameEngine gameEngine, CustomUIRenderer renderer, float screenWidth, float screenHeight)
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
    
    public void Update(float deltaTime)
    {
        _renderer.Update(deltaTime);
    }
    
    public void Render()
    {
        if (!_enabled) return;
        
        // Render shapes with custom renderer
        _renderer.BeginRender();
        RenderCrosshair();
        RenderCornerFrames();
        RenderShipStatusShapes();
        RenderRadar();
        _renderer.EndRender();
        
        // Render text with ImGui
        RenderShipStatusText();
        RenderRadarText();
    }
    
    private void RenderCrosshair()
    {
        _renderer.DrawCrosshair();
    }
    
    private void RenderCornerFrames()
    {
        _renderer.DrawCornerFrames();
    }
    
    private void RenderShipStatusShapes()
    {
        if (!_playerShipId.HasValue) return;
        
        // Get player ship components
        var physics = _gameEngine.EntityManager.GetComponent<PhysicsComponent>(_playerShipId.Value);
        if (physics == null) return;
        
        // Draw ship status panel in top-left (below corner frame)
        float panelX = 25f;
        float panelY = 100f;
        float panelWidth = 250f;
        float panelHeight = 150f;
        
        Vector4 panelBgColor = new Vector4(0.0f, 0.1f, 0.15f, 0.6f);
        Vector4 panelBorderColor = new Vector4(0.0f, 0.8f, 1.0f, 0.8f);
        
        // Background
        _renderer.DrawRectFilled(new Vector2(panelX, panelY), new Vector2(panelWidth, panelHeight), panelBgColor);
        
        // Border
        _renderer.DrawRect(new Vector2(panelX, panelY), new Vector2(panelWidth, panelHeight), panelBorderColor, 2f);
        
        // Hull integrity bar
        float barX = panelX + 15f;
        float barY = panelY + 50f;
        float barWidth = panelWidth - 30f;
        float barHeight = 20f;
        
        Vector4 barBgColor = new Vector4(0.1f, 0.1f, 0.1f, 0.8f);
        Vector4 barFillColor = new Vector4(0.0f, 1.0f, 0.5f, 0.9f);
        
        // Get voxel structure for hull integrity
        var voxelStructure = _gameEngine.EntityManager.GetComponent<VoxelStructureComponent>(_playerShipId.Value);
        float hullPercent = 1.0f; // Default to 100%
        if (voxelStructure != null && voxelStructure.Blocks.Count > 0)
        {
            // For now, use a simple calculation - could be enhanced with actual damage tracking
            hullPercent = MathF.Min(1.0f, voxelStructure.Blocks.Count / 100.0f);
        }
        
        // Background bar
        _renderer.DrawRectFilled(new Vector2(barX, barY), new Vector2(barWidth, barHeight), barBgColor);
        
        // Fill bar with color based on health
        Vector4 healthColor = hullPercent > 0.5f ? barFillColor : 
                             hullPercent > 0.25f ? new Vector4(1.0f, 0.8f, 0.0f, 0.9f) :
                             new Vector4(1.0f, 0.2f, 0.2f, 0.9f);
        _renderer.DrawRectFilled(new Vector2(barX, barY), new Vector2(barWidth * hullPercent, barHeight), healthColor);
        
        // Bar border
        _renderer.DrawRect(new Vector2(barX, barY), new Vector2(barWidth, barHeight), panelBorderColor, 1.5f);
        
        // Energy bar
        barY += 35f;
        var inventory = _gameEngine.EntityManager.GetComponent<InventoryComponent>(_playerShipId.Value);
        float energyPercent = 0.6f; // Default
        
        Vector4 energyBarColor = new Vector4(0.2f, 0.6f, 1.0f, 0.9f);
        _renderer.DrawRectFilled(new Vector2(barX, barY), new Vector2(barWidth, barHeight), barBgColor);
        _renderer.DrawRectFilled(new Vector2(barX, barY), new Vector2(barWidth * energyPercent, barHeight), energyBarColor);
        _renderer.DrawRect(new Vector2(barX, barY), new Vector2(barWidth, barHeight), panelBorderColor, 1.5f);
        
        // Shield bar
        barY += 35f;
        float shieldPercent = 0.75f; // Default
        Vector4 shieldBarColor = new Vector4(0.0f, 0.8f, 1.0f, 0.9f);
        _renderer.DrawRectFilled(new Vector2(barX, barY), new Vector2(barWidth, barHeight), barBgColor);
        _renderer.DrawRectFilled(new Vector2(barX, barY), new Vector2(barWidth * shieldPercent, barHeight), shieldBarColor);
        _renderer.DrawRect(new Vector2(barX, barY), new Vector2(barWidth, barHeight), panelBorderColor, 1.5f);
    }
    
    private void RenderShipStatusText()
    {
        if (!_playerShipId.HasValue) return;
        
        var physics = _gameEngine.EntityManager.GetComponent<PhysicsComponent>(_playerShipId.Value);
        if (physics == null) return;
        
        var voxelStructure = _gameEngine.EntityManager.GetComponent<VoxelStructureComponent>(_playerShipId.Value);
        var inventory = _gameEngine.EntityManager.GetComponent<InventoryComponent>(_playerShipId.Value);
        
        // Ship status panel text
        float panelX = 25f;
        float panelY = 100f;
        
        // Set ImGui window to be transparent and positioned
        ImGui.SetNextWindowPos(new Vector2(panelX + 10, panelY + 5));
        ImGui.SetNextWindowSize(new Vector2(230, 140));
        ImGui.PushStyleColor(ImGuiCol.WindowBg, new Vector4(0, 0, 0, 0));
        ImGui.PushStyleColor(ImGuiCol.Border, new Vector4(0, 0, 0, 0));
        
        if (ImGui.Begin("##ShipStatus", ImGuiWindowFlags.NoTitleBar | ImGuiWindowFlags.NoResize | 
                        ImGuiWindowFlags.NoMove | ImGuiWindowFlags.NoScrollbar | ImGuiWindowFlags.NoInputs))
        {
            // Title
            ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.0f, 0.8f, 1.0f, 1.0f));
            ImGui.Text("SHIP STATUS");
            ImGui.PopStyleColor();
            
            ImGui.Spacing();
            
            // Hull
            ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.7f, 0.7f, 0.7f, 1.0f));
            ImGui.Text("Hull Integrity");
            ImGui.PopStyleColor();
            float hullPercent = 100.0f;
            if (voxelStructure != null && voxelStructure.Blocks.Count > 0)
            {
                hullPercent = MathF.Min(100.0f, voxelStructure.Blocks.Count / 100.0f * 100.0f);
            }
            ImGui.SameLine(180);
            ImGui.Text($"{hullPercent:F0}%");
            
            ImGui.Spacing();
            ImGui.Spacing();
            
            // Energy
            ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.7f, 0.7f, 0.7f, 1.0f));
            ImGui.Text("Energy");
            ImGui.PopStyleColor();
            ImGui.SameLine(180);
            ImGui.Text("60%");
            
            ImGui.Spacing();
            ImGui.Spacing();
            
            // Shields
            ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.7f, 0.7f, 0.7f, 1.0f));
            ImGui.Text("Shields");
            ImGui.PopStyleColor();
            ImGui.SameLine(180);
            ImGui.Text("75%");
        }
        
        ImGui.End();
        ImGui.PopStyleColor(2);
        
        // Velocity display in top-right
        float velX = _screenWidth - 250f;
        float velY = 100f;
        
        ImGui.SetNextWindowPos(new Vector2(velX, velY));
        ImGui.SetNextWindowSize(new Vector2(225, 80));
        ImGui.PushStyleColor(ImGuiCol.WindowBg, new Vector4(0.0f, 0.1f, 0.15f, 0.6f));
        ImGui.PushStyleColor(ImGuiCol.Border, new Vector4(0.0f, 0.8f, 1.0f, 0.8f));
        
        if (ImGui.Begin("##Velocity", ImGuiWindowFlags.NoTitleBar | ImGuiWindowFlags.NoResize | 
                        ImGuiWindowFlags.NoMove | ImGuiWindowFlags.NoScrollbar | ImGuiWindowFlags.NoInputs))
        {
            ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.0f, 0.8f, 1.0f, 1.0f));
            ImGui.Text("VELOCITY");
            ImGui.PopStyleColor();
            
            float speed = physics.Velocity.Length();
            ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.9f, 0.9f, 0.9f, 1.0f));
            ImGui.Text($"Speed: {speed:F1} m/s");
            ImGui.Text($"Mass: {physics.Mass:F0} kg");
            ImGui.PopStyleColor();
        }
        
        ImGui.End();
        ImGui.PopStyleColor(2);
    }
    
    private void RenderRadar()
    {
        // Draw radar in bottom-left corner
        float radarX = 25f;
        float radarY = _screenHeight - 225f;
        float radarSize = 200f;
        float radarRadius = radarSize * 0.4f;
        
        Vector2 radarCenter = new Vector2(radarX + radarSize * 0.5f, radarY + radarSize * 0.5f);
        
        Vector4 radarBgColor = new Vector4(0.0f, 0.1f, 0.15f, 0.6f);
        Vector4 radarBorderColor = new Vector4(0.0f, 0.8f, 1.0f, 0.8f);
        Vector4 radarGridColor = new Vector4(0.0f, 0.6f, 0.8f, 0.4f);
        
        // Background
        _renderer.DrawRectFilled(new Vector2(radarX, radarY), new Vector2(radarSize, radarSize), radarBgColor);
        
        // Radar circle
        _renderer.DrawCircle(radarCenter, radarRadius, radarGridColor, 32, 2f);
        _renderer.DrawCircle(radarCenter, radarRadius * 0.5f, radarGridColor, 32, 1f);
        _renderer.DrawCircle(radarCenter, radarRadius * 0.25f, radarGridColor, 32, 1f);
        
        // Crosshair on radar
        _renderer.DrawLine(
            radarCenter - new Vector2(radarRadius, 0),
            radarCenter + new Vector2(radarRadius, 0),
            radarGridColor, 1f);
        _renderer.DrawLine(
            radarCenter - new Vector2(0, radarRadius),
            radarCenter + new Vector2(0, radarRadius),
            radarGridColor, 1f);
        
        // Center dot (player position)
        Vector4 playerDotColor = new Vector4(0.0f, 1.0f, 0.5f, 1.0f);
        _renderer.DrawCircleFilled(radarCenter, 3f, playerDotColor);
        
        // Draw nearby entities
        if (_playerShipId.HasValue)
        {
            var playerPhysics = _gameEngine.EntityManager.GetComponent<PhysicsComponent>(_playerShipId.Value);
            if (playerPhysics != null)
            {
                // Get all entities with physics
                var entities = _gameEngine.EntityManager.GetAllEntities();
                foreach (var entity in entities)
                {
                    if (entity.Id == _playerShipId.Value) continue;
                    
                    var physics = _gameEngine.EntityManager.GetComponent<PhysicsComponent>(entity.Id);
                    if (physics != null)
                    {
                        // Calculate relative position
                        Vector3 relativePos = physics.Position - playerPhysics.Position;
                        float distance = relativePos.Length();
                        
                        // Only show entities within range (1000 units)
                        if (distance < 1000f && distance > 0.1f)
                        {
                            // Project onto radar (XZ plane)
                            float radarX_rel = (relativePos.X / 1000f) * radarRadius;
                            float radarY_rel = (relativePos.Z / 1000f) * radarRadius;
                            
                            Vector2 dotPos = radarCenter + new Vector2(radarX_rel, radarY_rel);
                            Vector4 dotColor = new Vector4(1.0f, 0.5f, 0.0f, 0.8f); // Orange for other entities
                            
                            _renderer.DrawCircleFilled(dotPos, 2f, dotColor);
                        }
                    }
                }
            }
        }
        
        // Border
        _renderer.DrawRect(new Vector2(radarX, radarY), new Vector2(radarSize, radarSize), radarBorderColor, 2f);
    }
    
    private void RenderRadarText()
    {
        // Radar label
        float radarX = 25f;
        float radarY = _screenHeight - 225f;
        
        ImGui.SetNextWindowPos(new Vector2(radarX + 10, radarY + 5));
        ImGui.SetNextWindowSize(new Vector2(180, 30));
        ImGui.PushStyleColor(ImGuiCol.WindowBg, new Vector4(0, 0, 0, 0));
        ImGui.PushStyleColor(ImGuiCol.Border, new Vector4(0, 0, 0, 0));
        
        if (ImGui.Begin("##Radar", ImGuiWindowFlags.NoTitleBar | ImGuiWindowFlags.NoResize | 
                        ImGuiWindowFlags.NoMove | ImGuiWindowFlags.NoScrollbar | ImGuiWindowFlags.NoInputs))
        {
            ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.0f, 0.8f, 1.0f, 1.0f));
            ImGui.Text("RADAR (1000m)");
            ImGui.PopStyleColor();
        }
        
        ImGui.End();
        ImGui.PopStyleColor(2);
    }
}
