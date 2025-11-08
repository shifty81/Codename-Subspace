using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Physics;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Resources;
using AvorionLike.Core.Combat;
using ImGuiNET;

namespace AvorionLike.Core.UI;

/// <summary>
/// Main game HUD with both custom OpenGL rendering and ImGui text
/// Custom renderer for shapes/graphics, ImGui for text labels
/// Now fully responsive to different screen resolutions
/// </summary>
public class GameHUD
{
    private readonly GameEngine _gameEngine;
    private readonly CustomUIRenderer _renderer;
    private readonly ResponsiveUILayout _layout;
    private bool _enabled = true;
    private Guid? _playerShipId;
    
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
        _layout = new ResponsiveUILayout(screenWidth, screenHeight);
    }
    
    public void UpdateScreenSize(float width, float height)
    {
        _layout.UpdateScreenSize(width, height);
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
        
        // Get responsive layout
        var hudLayout = _layout.CalculateHUDLayout();
        Vector2 panelPos = hudLayout.ShipStatusPosition;
        Vector2 panelSize = hudLayout.ShipStatusSize;
        
        Vector4 panelBgColor = new Vector4(0.0f, 0.1f, 0.15f, 0.7f);
        Vector4 panelBgColorDark = new Vector4(0.0f, 0.05f, 0.08f, 0.85f);
        Vector4 panelBorderColor = new Vector4(0.0f, 0.9f, 1.0f, 0.9f);
        Vector4 panelGlowColor = new Vector4(0.0f, 0.8f, 1.0f, 0.3f);
        
        // Background with gradient and glow (responsive)
        float glowSize = _layout.GetGlowSize(10f);
        float borderThickness = _layout.GetLineThickness(3f);
        float innerBorderThickness = _layout.GetLineThickness(1f);
        
        _renderer.DrawRectWithGlow(panelPos, panelSize, panelBgColor, panelGlowColor, glowSize);
        _renderer.DrawRectGradient(panelPos, panelSize, panelBgColor, panelBgColorDark);
        
        // Enhanced border with double line
        _renderer.DrawRect(panelPos, panelSize, panelBorderColor, borderThickness);
        Vector2 innerOffset = new Vector2(_layout.Scale(3f), _layout.Scale(3f));
        _renderer.DrawRect(panelPos + innerOffset, panelSize - innerOffset * 2, 
                          new Vector4(panelBorderColor.X, panelBorderColor.Y, panelBorderColor.Z, 0.4f), innerBorderThickness);
        
        // Progress bars with enhanced visuals (responsive sizing)
        float barX = panelPos.X + _layout.Scale(20f);
        float barY = panelPos.Y + panelSize.Y * 0.35f; // Position bars relative to panel height
        float barWidth = panelSize.X - _layout.Scale(40f);
        float barHeight = _layout.Scale(22f);
        float barSpacing = _layout.Scale(38f);
        
        Vector4 barBgColor = new Vector4(0.05f, 0.05f, 0.1f, 0.9f);
        Vector4 barBorderColor = new Vector4(0.0f, 0.7f, 0.9f, 0.8f);
        
        // Get voxel structure for hull integrity
        var voxelStructure = _gameEngine.EntityManager.GetComponent<VoxelStructureComponent>(_playerShipId.Value);
        float hullPercent = 1.0f; // Default to 100%
        if (voxelStructure != null && voxelStructure.Blocks.Count > 0)
        {
            // For now, use a simple calculation - could be enhanced with actual damage tracking
            hullPercent = MathF.Min(1.0f, voxelStructure.Blocks.Count / 100.0f);
        }
        
        // Hull bar with dynamic color
        Vector4 healthColor = hullPercent > 0.5f ? new Vector4(0.0f, 1.0f, 0.5f, 1.0f) : 
                             hullPercent > 0.25f ? new Vector4(1.0f, 0.9f, 0.0f, 1.0f) :
                             new Vector4(1.0f, 0.2f, 0.2f, 1.0f);
        _renderer.DrawProgressBar(new Vector2(barX, barY), new Vector2(barWidth, barHeight), 
                                 hullPercent, healthColor, barBgColor, barBorderColor);
        
        // Energy bar
        barY += barSpacing;
        var combatComponent = _gameEngine.EntityManager.GetComponent<CombatComponent>(_playerShipId.Value);
        
        float energyPercent = 0.6f; // Default
        if (combatComponent != null && combatComponent.MaxEnergy > 0)
        {
            energyPercent = MathF.Max(0f, MathF.Min(1f, combatComponent.CurrentEnergy / combatComponent.MaxEnergy));
        }
        
        Vector4 energyBarColor = new Vector4(0.3f, 0.7f, 1.0f, 1.0f);
        _renderer.DrawProgressBar(new Vector2(barX, barY), new Vector2(barWidth, barHeight), 
                                 energyPercent, energyBarColor, barBgColor, barBorderColor);
        
        // Shield bar
        barY += barSpacing;
        float shieldPercent = 0.75f; // Default
        if (combatComponent != null && combatComponent.MaxShields > 0)
        {
            shieldPercent = MathF.Max(0f, MathF.Min(1f, combatComponent.CurrentShields / combatComponent.MaxShields));
        }
        
        Vector4 shieldBarColor = new Vector4(0.0f, 0.95f, 1.0f, 1.0f);
        _renderer.DrawProgressBar(new Vector2(barX, barY), new Vector2(barWidth, barHeight), 
                                 shieldPercent, shieldBarColor, barBgColor, barBorderColor);
    }
    
    private void RenderShipStatusText()
    {
        if (!_playerShipId.HasValue) return;
        
        var physics = _gameEngine.EntityManager.GetComponent<PhysicsComponent>(_playerShipId.Value);
        if (physics == null) return;
        
        var voxelStructure = _gameEngine.EntityManager.GetComponent<VoxelStructureComponent>(_playerShipId.Value);
        var inventory = _gameEngine.EntityManager.GetComponent<InventoryComponent>(_playerShipId.Value);
        var combatComponent = _gameEngine.EntityManager.GetComponent<CombatComponent>(_playerShipId.Value);
        
        // Get responsive layout
        var hudLayout = _layout.CalculateHUDLayout();
        Vector2 panelPos = hudLayout.ShipStatusPosition;
        Vector2 panelSize = hudLayout.ShipStatusSize;
        float fontScale = _layout.GetFontScale();
        
        // Set ImGui window to be transparent and positioned
        float padding = _layout.Scale(15f);
        ImGui.SetNextWindowPos(new Vector2(panelPos.X + padding, panelPos.Y + _layout.Scale(8f)));
        ImGui.SetNextWindowSize(new Vector2(panelSize.X - padding * 2, panelSize.Y - _layout.Scale(16f)));
        ImGui.PushStyleColor(ImGuiCol.WindowBg, new Vector4(0, 0, 0, 0));
        ImGui.PushStyleColor(ImGuiCol.Border, new Vector4(0, 0, 0, 0));
        ImGui.PushStyleVar(ImGuiStyleVar.ItemSpacing, new Vector2(8 * fontScale, 6 * fontScale));
        
        if (ImGui.Begin("##ShipStatus", ImGuiWindowFlags.NoTitleBar | ImGuiWindowFlags.NoResize | 
                        ImGuiWindowFlags.NoMove | ImGuiWindowFlags.NoScrollbar | ImGuiWindowFlags.NoInputs))
        {
            // Title with enhanced styling
            ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.0f, 1.0f, 1.0f, 1.0f));
            ImGui.SetWindowFontScale(1.15f * fontScale);
            ImGui.Text("⬡ SHIP STATUS");
            ImGui.SetWindowFontScale(1.0f * fontScale);
            ImGui.PopStyleColor();
            
            ImGui.Spacing();
            ImGui.Separator();
            ImGui.Spacing();
            
            // Calculate value column position based on panel width
            float valueColumnPos = panelSize.X - padding * 2 - _layout.Scale(60f);
            
            // Hull
            ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.8f, 0.8f, 0.9f, 1.0f));
            ImGui.Text("Hull Integrity");
            ImGui.PopStyleColor();
            float hullPercent = 100.0f;
            if (voxelStructure != null && voxelStructure.Blocks.Count > 0)
            {
                hullPercent = MathF.Min(100.0f, voxelStructure.Blocks.Count / 100.0f * 100.0f);
            }
            ImGui.SameLine(valueColumnPos);
            Vector4 hullColor = hullPercent > 50f ? new Vector4(0.0f, 1.0f, 0.6f, 1.0f) :
                                hullPercent > 25f ? new Vector4(1.0f, 0.9f, 0.0f, 1.0f) :
                                new Vector4(1.0f, 0.3f, 0.3f, 1.0f);
            ImGui.PushStyleColor(ImGuiCol.Text, hullColor);
            ImGui.Text($"{hullPercent:F0}%");
            ImGui.PopStyleColor();
            
            ImGui.Spacing();
            
            // Energy
            ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.8f, 0.8f, 0.9f, 1.0f));
            ImGui.Text("Energy");
            ImGui.PopStyleColor();
            ImGui.SameLine(valueColumnPos);
            if (combatComponent != null && combatComponent.MaxEnergy > 0)
            {
                float energyPct = (combatComponent.CurrentEnergy / combatComponent.MaxEnergy) * 100f;
                ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.4f, 0.8f, 1.0f, 1.0f));
                ImGui.Text($"{energyPct:F0}%");
                ImGui.PopStyleColor();
            }
            else
            {
                ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.5f, 0.5f, 0.5f, 1.0f));
                ImGui.Text("N/A");
                ImGui.PopStyleColor();
            }
            
            ImGui.Spacing();
            
            // Shields
            ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.8f, 0.8f, 0.9f, 1.0f));
            ImGui.Text("Shields");
            ImGui.PopStyleColor();
            ImGui.SameLine(valueColumnPos);
            if (combatComponent != null && combatComponent.MaxShields > 0)
            {
                float shieldPct = (combatComponent.CurrentShields / combatComponent.MaxShields) * 100f;
                ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.0f, 1.0f, 1.0f, 1.0f));
                ImGui.Text($"{shieldPct:F0}%");
                ImGui.PopStyleColor();
            }
            else
            {
                ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.5f, 0.5f, 0.5f, 1.0f));
                ImGui.Text("N/A");
                ImGui.PopStyleColor();
            }
        }
        
        ImGui.End();
        ImGui.PopStyleVar();
        ImGui.PopStyleColor(2);
        
        // Velocity and FPS display in top-right with enhanced styling (responsive)
        Vector2 velPos = hudLayout.VelocityPosition;
        Vector2 velSize = hudLayout.VelocitySize;
        
        ImGui.SetNextWindowPos(velPos);
        ImGui.SetNextWindowSize(velSize);
        ImGui.PushStyleColor(ImGuiCol.WindowBg, new Vector4(0.0f, 0.1f, 0.15f, 0.7f));
        ImGui.PushStyleColor(ImGuiCol.Border, new Vector4(0.0f, 0.9f, 1.0f, 0.9f));
        ImGui.PushStyleVar(ImGuiStyleVar.WindowPadding, new Vector2(12 * fontScale, 10 * fontScale));
        ImGui.PushStyleVar(ImGuiStyleVar.ItemSpacing, new Vector2(8 * fontScale, 5 * fontScale));
        
        if (ImGui.Begin("##Velocity", ImGuiWindowFlags.NoTitleBar | ImGuiWindowFlags.NoResize | 
                        ImGuiWindowFlags.NoMove | ImGuiWindowFlags.NoScrollbar | ImGuiWindowFlags.NoInputs))
        {
            ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.0f, 1.0f, 1.0f, 1.0f));
            ImGui.SetWindowFontScale(1.15f * fontScale);
            ImGui.Text("⬡ VELOCITY");
            ImGui.SetWindowFontScale(1.0f * fontScale);
            ImGui.PopStyleColor();
            
            ImGui.Separator();
            ImGui.Spacing();
            
            float speed = physics.Velocity.Length();
            float velValueColumn = velSize.X - _layout.Scale(140f);
            
            ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.8f, 0.8f, 0.9f, 1.0f));
            ImGui.Text("Speed:");
            ImGui.PopStyleColor();
            ImGui.SameLine(velValueColumn);
            ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.4f, 1.0f, 0.8f, 1.0f));
            ImGui.Text($"{speed:F1} m/s");
            ImGui.PopStyleColor();
            
            ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.8f, 0.8f, 0.9f, 1.0f));
            ImGui.Text("Mass:");
            ImGui.PopStyleColor();
            ImGui.SameLine(velValueColumn);
            ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.9f, 0.9f, 0.9f, 1.0f));
            ImGui.Text($"{physics.Mass:F0} kg");
            ImGui.PopStyleColor();
            
            // Add FPS counter with color coding
            var io = ImGui.GetIO();
            ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.8f, 0.8f, 0.9f, 1.0f));
            ImGui.Text("FPS:");
            ImGui.PopStyleColor();
            ImGui.SameLine(velValueColumn);
            Vector4 fpsColor = io.Framerate > 50 ? new Vector4(0.0f, 1.0f, 0.5f, 1.0f) :
                              io.Framerate > 30 ? new Vector4(1.0f, 0.9f, 0.0f, 1.0f) :
                              new Vector4(1.0f, 0.3f, 0.3f, 1.0f);
            ImGui.PushStyleColor(ImGuiCol.Text, fpsColor);
            ImGui.Text($"{io.Framerate:F0}");
            ImGui.PopStyleColor();
        }
        
        ImGui.End();
        ImGui.PopStyleVar(2);
        ImGui.PopStyleColor(2);
        
        // Resource display (responsive)
        if (inventory != null)
        {
            Vector2 resPos = hudLayout.ResourcesPosition;
            Vector2 resSize = hudLayout.ResourcesSize;
            
            ImGui.SetNextWindowPos(resPos);
            ImGui.SetNextWindowSize(resSize);
            ImGui.PushStyleColor(ImGuiCol.WindowBg, new Vector4(0.0f, 0.1f, 0.15f, 0.7f));
            ImGui.PushStyleColor(ImGuiCol.Border, new Vector4(0.0f, 0.9f, 1.0f, 0.9f));
            ImGui.PushStyleVar(ImGuiStyleVar.WindowPadding, new Vector2(12 * fontScale, 10 * fontScale));
            ImGui.PushStyleVar(ImGuiStyleVar.ItemSpacing, new Vector2(8 * fontScale, 5 * fontScale));
            
            if (ImGui.Begin("##Resources", ImGuiWindowFlags.NoTitleBar | ImGuiWindowFlags.NoResize | 
                            ImGuiWindowFlags.NoMove | ImGuiWindowFlags.NoScrollbar | ImGuiWindowFlags.NoInputs))
            {
                ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.0f, 1.0f, 1.0f, 1.0f));
                ImGui.SetWindowFontScale(1.15f * fontScale);
                ImGui.Text("⬡ RESOURCES");
                ImGui.SetWindowFontScale(1.0f * fontScale);
                ImGui.PopStyleColor();
                
                ImGui.Separator();
                ImGui.Spacing();
                
                // Display key resources with color coding
                var credits = inventory.Inventory.GetResourceAmount(ResourceType.Credits);
                ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(1.0f, 0.84f, 0.0f, 1.0f)); // Gold
                ImGui.Text("₵");
                ImGui.PopStyleColor();
                ImGui.SameLine();
                ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.9f, 0.9f, 0.9f, 1.0f));
                ImGui.Text($"Credits: {credits:N0}");
                ImGui.PopStyleColor();
                
                var iron = inventory.Inventory.GetResourceAmount(ResourceType.Iron);
                ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.7f, 0.7f, 0.7f, 1.0f)); // Gray
                ImGui.Text("▣");
                ImGui.PopStyleColor();
                ImGui.SameLine();
                ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.9f, 0.9f, 0.9f, 1.0f));
                ImGui.Text($"Iron: {iron:N0}");
                ImGui.PopStyleColor();
                
                var titanium = inventory.Inventory.GetResourceAmount(ResourceType.Titanium);
                ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.5f, 0.7f, 1.0f, 1.0f)); // Blue
                ImGui.Text("▣");
                ImGui.PopStyleColor();
                ImGui.SameLine();
                ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.9f, 0.9f, 0.9f, 1.0f));
                ImGui.Text($"Titanium: {titanium:N0}");
                ImGui.PopStyleColor();
            }
            
            ImGui.End();
            ImGui.PopStyleVar(2);
            ImGui.PopStyleColor(2);
        }
    }
    
    private void RenderRadar()
    {
        // Get responsive layout for radar
        var hudLayout = _layout.CalculateHUDLayout();
        Vector2 radarPos = hudLayout.RadarPosition;
        Vector2 radarSize = hudLayout.RadarSize;
        float radarRadius = radarSize.X * 0.42f;
        
        Vector2 radarCenter = radarPos + radarSize / 2f;
        
        Vector4 radarBgColor = new Vector4(0.0f, 0.1f, 0.15f, 0.75f);
        Vector4 radarBgColorDark = new Vector4(0.0f, 0.05f, 0.08f, 0.9f);
        Vector4 radarBorderColor = new Vector4(0.0f, 0.9f, 1.0f, 0.95f);
        Vector4 radarGlowColor = new Vector4(0.0f, 0.8f, 1.0f, 0.25f);
        Vector4 radarGridColor = new Vector4(0.0f, 0.7f, 0.9f, 0.5f);
        
        float glowSize = _layout.GetGlowSize(10f);
        float borderThickness = _layout.GetLineThickness(3f);
        float innerBorderThickness = _layout.GetLineThickness(1f);
        
        // Background with gradient and glow (responsive)
        _renderer.DrawRectWithGlow(radarPos, radarSize, radarBgColor, radarGlowColor, glowSize);
        _renderer.DrawRectGradient(radarPos, radarSize, radarBgColor, radarBgColorDark);
        
        // Enhanced border with double line
        _renderer.DrawRect(radarPos, radarSize, radarBorderColor, borderThickness);
        Vector2 innerOffset = new Vector2(_layout.Scale(3f), _layout.Scale(3f));
        _renderer.DrawRect(radarPos + innerOffset, radarSize - innerOffset * 2, 
                          new Vector4(radarBorderColor.X, radarBorderColor.Y, radarBorderColor.Z, 0.4f), innerBorderThickness);
        
        // Radar circles with varying opacity
        float lineThickness = _layout.GetLineThickness(2.5f);
        _renderer.DrawCircle(radarCenter, radarRadius, radarGridColor, 40, lineThickness);
        _renderer.DrawCircle(radarCenter, radarRadius * 0.66f, new Vector4(radarGridColor.X, radarGridColor.Y, radarGridColor.Z, radarGridColor.W * 0.7f), 32, lineThickness * 0.7f);
        _renderer.DrawCircle(radarCenter, radarRadius * 0.33f, new Vector4(radarGridColor.X, radarGridColor.Y, radarGridColor.Z, radarGridColor.W * 0.5f), 24, lineThickness * 0.6f);
        
        // Animated sweep line effect
        float sweepAngle = (_renderer as dynamic)?._animationTime ?? 0f;
        float sweepX = MathF.Cos(sweepAngle) * radarRadius;
        float sweepY = MathF.Sin(sweepAngle) * radarRadius;
        Vector4 sweepColor = new Vector4(0.0f, 1.0f, 0.8f, 0.3f);
        _renderer.DrawLine(radarCenter, radarCenter + new Vector2(sweepX, sweepY), sweepColor, _layout.GetLineThickness(2f));
        
        // Crosshair on radar with enhanced styling
        _renderer.DrawLine(
            radarCenter - new Vector2(radarRadius, 0),
            radarCenter + new Vector2(radarRadius, 0),
            radarGridColor, _layout.GetLineThickness(1.5f));
        _renderer.DrawLine(
            radarCenter - new Vector2(0, radarRadius),
            radarCenter + new Vector2(0, radarRadius),
            radarGridColor, _layout.GetLineThickness(1.5f));
        
        // Center dot (player position) with glow (responsive size)
        Vector4 playerDotColor = new Vector4(0.0f, 1.0f, 0.6f, 1.0f);
        Vector4 playerGlowColor = new Vector4(0.0f, 1.0f, 0.6f, 0.3f);
        float dotSize = _layout.Scale(3.5f);
        _renderer.DrawCircleFilled(radarCenter, _layout.Scale(5f), playerGlowColor, 16);
        _renderer.DrawCircleFilled(radarCenter, dotSize, playerDotColor, 16);
        
        // Draw nearby entities with enhanced visuals
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
                            
                            // Color based on entity type or distance
                            float distanceFactor = 1f - (distance / 1000f);
                            Vector4 dotColor = new Vector4(1.0f, 0.5f, 0.0f, 0.7f + distanceFactor * 0.3f); // Orange
                            Vector4 dotGlow = new Vector4(1.0f, 0.5f, 0.0f, 0.2f);
                            
                            float entityDotSize = _layout.Scale(2.2f);
                            _renderer.DrawCircleFilled(dotPos, _layout.Scale(3f), dotGlow, 12);
                            _renderer.DrawCircleFilled(dotPos, entityDotSize, dotColor, 12);
                        }
                    }
                }
            }
        }
    }
    
    private void RenderRadarText()
    {
        // Get responsive layout
        var hudLayout = _layout.CalculateHUDLayout();
        Vector2 radarPos = hudLayout.RadarPosition;
        float fontScale = _layout.GetFontScale();
        
        // Radar label with enhanced styling
        ImGui.SetNextWindowPos(new Vector2(radarPos.X + _layout.Scale(12f), radarPos.Y + _layout.Scale(8f)));
        ImGui.SetNextWindowSize(new Vector2(_layout.Scale(186f), _layout.Scale(35f)));
        ImGui.PushStyleColor(ImGuiCol.WindowBg, new Vector4(0, 0, 0, 0));
        ImGui.PushStyleColor(ImGuiCol.Border, new Vector4(0, 0, 0, 0));
        
        if (ImGui.Begin("##Radar", ImGuiWindowFlags.NoTitleBar | ImGuiWindowFlags.NoResize | 
                        ImGuiWindowFlags.NoMove | ImGuiWindowFlags.NoScrollbar | ImGuiWindowFlags.NoInputs))
        {
            ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.0f, 1.0f, 1.0f, 1.0f));
            ImGui.SetWindowFontScale(1.15f * fontScale);
            ImGui.Text("⬡ RADAR");
            ImGui.SetWindowFontScale(0.85f * fontScale);
            ImGui.SameLine();
            ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.6f, 0.9f, 1.0f, 0.8f));
            ImGui.Text("(1000m)");
            ImGui.PopStyleColor();
            ImGui.SetWindowFontScale(1.0f * fontScale);
            ImGui.PopStyleColor();
        }
        
        ImGui.End();
        ImGui.PopStyleColor(2);
        
        // Controls hint at bottom
        RenderControlsHint();
    }
    
    private void RenderControlsHint()
    {
        // Get responsive layout
        var hudLayout = _layout.CalculateHUDLayout();
        Vector2 controlsPos = hudLayout.ControlsPosition;
        Vector2 controlsSize = hudLayout.ControlsSize;
        float fontScale = _layout.GetFontScale();
        
        ImGui.SetNextWindowPos(controlsPos);
        ImGui.SetNextWindowSize(controlsSize);
        ImGui.PushStyleColor(ImGuiCol.WindowBg, new Vector4(0.0f, 0.1f, 0.15f, 0.75f));
        ImGui.PushStyleColor(ImGuiCol.Border, new Vector4(0.0f, 0.9f, 1.0f, 0.7f));
        ImGui.PushStyleVar(ImGuiStyleVar.WindowPadding, new Vector2(15 * fontScale, 10 * fontScale));
        ImGui.PushStyleVar(ImGuiStyleVar.ItemSpacing, new Vector2(8 * fontScale, 4 * fontScale));
        
        if (ImGui.Begin("##Controls", ImGuiWindowFlags.NoTitleBar | ImGuiWindowFlags.NoResize | 
                        ImGuiWindowFlags.NoMove | ImGuiWindowFlags.NoScrollbar | ImGuiWindowFlags.NoInputs))
        {
            ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.0f, 1.0f, 1.0f, 1.0f));
            ImGui.SetWindowFontScale(1.1f * fontScale);
            ImGui.Text("⬡ CONTROLS");
            ImGui.SetWindowFontScale(1.0f * fontScale);
            ImGui.PopStyleColor();
            
            ImGui.Separator();
            
            ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.85f, 0.85f, 0.95f, 1.0f));
            
            // Adapt text based on available width
            if (controlsSize.X < _layout.Scale(400f))
            {
                // Compact layout for smaller screens
                ImGui.Text("C: Toggle Mode  •  WASD: Move");
                ImGui.Text("Arrows+Q/E: Rotate  •  ESC: Exit");
            }
            else
            {
                // Full layout for larger screens
                ImGui.Text("C: Toggle Ship/Camera  •  WASD + Space/Shift: Move/Thrust");
                ImGui.Text("Arrow Keys + Q/E: Rotate  •  Mouse: Look  •  ESC: Exit");
            }
            
            ImGui.PopStyleColor();
        }
        
        ImGui.End();
        ImGui.PopStyleVar(2);
        ImGui.PopStyleColor(2);
    }
}
