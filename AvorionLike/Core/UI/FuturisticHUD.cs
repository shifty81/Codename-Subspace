using ImGuiNET;
using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Physics;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Resources;

namespace AvorionLike.Core.UI;

/// <summary>
/// Futuristic sci-fi HUD overlay with holographic-style elements
/// Includes radar, target info, ship status, navigation compass, and decorative frames
/// </summary>
public class FuturisticHUD
{
    private readonly GameEngine _gameEngine;
    private ResponsiveUILayout _layout;
    private bool _isEnabled = false;
    private Guid _selectedTargetId = Guid.Empty;
    private float _radarRange = 1000f;
    private float _animationTime = 0f;
    
    // Colors for futuristic theme
    private readonly Vector4 _primaryColor = new(0.0f, 0.8f, 1.0f, 1.0f);      // Cyan
    private readonly Vector4 _secondaryColor = new(0.2f, 1.0f, 0.8f, 1.0f);    // Bright teal
    private readonly Vector4 _warningColor = new(1.0f, 0.5f, 0.0f, 1.0f);      // Orange
    private readonly Vector4 _dangerColor = new(1.0f, 0.2f, 0.2f, 1.0f);       // Red
    private readonly Vector4 _frameColor = new(0.0f, 0.6f, 0.8f, 0.3f);        // Semi-transparent cyan
    private readonly Vector4 _lineColor = new(0.0f, 0.8f, 1.0f, 0.8f);         // Bright cyan
    
    public bool IsEnabled => _isEnabled;
    
    public FuturisticHUD(GameEngine gameEngine)
    {
        _gameEngine = gameEngine;
        _layout = new ResponsiveUILayout(1920f, 1080f);
    }
    
    /// <summary>
    /// Toggle the futuristic HUD on/off
    /// </summary>
    public void Toggle()
    {
        _isEnabled = !_isEnabled;
    }
    
    /// <summary>
    /// Enable the futuristic HUD
    /// </summary>
    public void Enable()
    {
        _isEnabled = true;
    }
    
    /// <summary>
    /// Disable the futuristic HUD
    /// </summary>
    public void Disable()
    {
        _isEnabled = false;
    }
    
    /// <summary>
    /// Render all futuristic HUD elements
    /// </summary>
    public void Render()
    {
        if (!_isEnabled) return;
        
        // Update layout for current screen resolution
        var io = ImGui.GetIO();
        _layout.UpdateScreenSize(io.DisplaySize.X, io.DisplaySize.Y);
        
        // Update animation time
        _animationTime += ImGui.GetIO().DeltaTime;
        
        // Render corner frames
        RenderCornerFrames();
        
        // Render main HUD elements
        RenderRadar();
        RenderShipStatus();
        RenderTargetInfo();
        RenderNavigationCompass();
        RenderScanLines();
    }
    
    /// <summary>
    /// Handle input for the futuristic HUD
    /// </summary>
    public void HandleInput()
    {
        // Toggle HUD with F4
        if (ImGui.IsKeyPressed(ImGuiKey.F4))
        {
            Toggle();
        }
        
        // Cycle through targets with Tab
        if (ImGui.IsKeyPressed(ImGuiKey.Tab) && _isEnabled)
        {
            CycleTarget();
        }
    }
    
    /// <summary>
    /// Render decorative corner frames for sci-fi aesthetic
    /// </summary>
    private void RenderCornerFrames()
    {
        var drawList = ImGui.GetBackgroundDrawList();
        var displaySize = ImGui.GetIO().DisplaySize;
        
        float cornerSize = _layout.Scale(80f);
        float thickness = _layout.GetLineThickness(3f);
        float innerOffset = _layout.Scale(15f);
        
        // Animate corner brightness
        float pulse = 0.7f + 0.3f * MathF.Sin(_animationTime * 2f);
        var frameColor = new Vector4(_frameColor.X, _frameColor.Y, _frameColor.Z, _frameColor.W * pulse);
        uint color = ImGui.ColorConvertFloat4ToU32(frameColor);
        
        // Top-left corner
        drawList.AddLine(new Vector2(innerOffset, innerOffset), 
                        new Vector2(cornerSize, innerOffset), color, thickness);
        drawList.AddLine(new Vector2(innerOffset, innerOffset), 
                        new Vector2(innerOffset, cornerSize), color, thickness);
        
        // Top-right corner
        drawList.AddLine(new Vector2(displaySize.X - innerOffset, innerOffset), 
                        new Vector2(displaySize.X - cornerSize, innerOffset), color, thickness);
        drawList.AddLine(new Vector2(displaySize.X - innerOffset, innerOffset), 
                        new Vector2(displaySize.X - innerOffset, cornerSize), color, thickness);
        
        // Bottom-left corner
        drawList.AddLine(new Vector2(innerOffset, displaySize.Y - innerOffset), 
                        new Vector2(cornerSize, displaySize.Y - innerOffset), color, thickness);
        drawList.AddLine(new Vector2(innerOffset, displaySize.Y - innerOffset), 
                        new Vector2(innerOffset, displaySize.Y - cornerSize), color, thickness);
        
        // Bottom-right corner
        drawList.AddLine(new Vector2(displaySize.X - innerOffset, displaySize.Y - innerOffset), 
                        new Vector2(displaySize.X - cornerSize, displaySize.Y - innerOffset), color, thickness);
        drawList.AddLine(new Vector2(displaySize.X - innerOffset, displaySize.Y - innerOffset), 
                        new Vector2(displaySize.X - innerOffset, displaySize.Y - cornerSize), color, thickness);
        
        // Add diagonal accents
        float accentSize = _layout.Scale(15f);
        drawList.AddLine(new Vector2(cornerSize, innerOffset), 
                        new Vector2(cornerSize - accentSize, innerOffset + accentSize), color, thickness);
        drawList.AddLine(new Vector2(displaySize.X - cornerSize, innerOffset), 
                        new Vector2(displaySize.X - cornerSize + accentSize, innerOffset + accentSize), color, thickness);
    }
    
    /// <summary>
    /// Render radar/scanner showing nearby entities
    /// </summary>
    private void RenderRadar()
    {
        var displaySize = ImGui.GetIO().DisplaySize;
        float radarSize = _layout.Scale(200f);
        var radarPos = new Vector2(displaySize.X - radarSize - _layout.Scale(30f), _layout.Scale(30f));
        
        ImGui.SetNextWindowPos(radarPos);
        ImGui.SetNextWindowSize(new Vector2(radarSize, radarSize));
        ImGui.SetNextWindowBgAlpha(0.2f);
        
        ImGuiWindowFlags flags = ImGuiWindowFlags.NoDecoration | 
                                ImGuiWindowFlags.NoMove | 
                                ImGuiWindowFlags.NoSavedSettings |
                                ImGuiWindowFlags.NoFocusOnAppearing;
        
        ImGui.PushStyleColor(ImGuiCol.Border, _lineColor);
        
        if (ImGui.Begin("##Radar", flags))
        {
            var drawList = ImGui.GetWindowDrawList();
            var center = new Vector2(radarPos.X + radarSize / 2, radarPos.Y + radarSize / 2);
            float radius = radarSize / 2 - _layout.Scale(10f);
            
            // Draw radar circles
            uint circleColor = ImGui.ColorConvertFloat4ToU32(_lineColor);
            drawList.AddCircle(center, radius, circleColor, 64, 2f);
            drawList.AddCircle(center, radius * 0.66f, circleColor, 64, 1f);
            drawList.AddCircle(center, radius * 0.33f, circleColor, 64, 1f);
            
            // Draw crosshairs
            drawList.AddLine(new Vector2(center.X - radius, center.Y), 
                           new Vector2(center.X + radius, center.Y), circleColor, 1f);
            drawList.AddLine(new Vector2(center.X, center.Y - radius), 
                           new Vector2(center.X, center.Y + radius), circleColor, 1f);
            
            // Draw scanning line (animated)
            float angle = _animationTime * 2f;
            var scanEnd = new Vector2(
                center.X + MathF.Cos(angle) * radius,
                center.Y + MathF.Sin(angle) * radius
            );
            uint scanColor = ImGui.ColorConvertFloat4ToU32(new Vector4(_secondaryColor.X, _secondaryColor.Y, _secondaryColor.Z, 0.5f));
            drawList.AddLine(center, scanEnd, scanColor, 2f);
            
            // Draw entities as blips
            var entities = _gameEngine.EntityManager.GetAllEntities();
            var playerEntity = entities.FirstOrDefault();
            
            if (playerEntity != null)
            {
                var playerPhysics = _gameEngine.EntityManager.GetComponent<PhysicsComponent>(playerEntity.Id);
                if (playerPhysics != null)
                {
                    Vector3 playerPos = playerPhysics.Position;
                    
                    foreach (var entity in entities)
                    {
                        if (entity.Id == playerEntity.Id) continue;
                        
                        var physics = _gameEngine.EntityManager.GetComponent<PhysicsComponent>(entity.Id);
                        if (physics != null)
                        {
                            Vector3 relativePos = physics.Position - playerPos;
                            float distance = relativePos.Length();
                            
                            if (distance < _radarRange)
                            {
                                // Scale position to radar
                                float scale = radius / _radarRange;
                                var blipPos = new Vector2(
                                    center.X + relativePos.X * scale,
                                    center.Y - relativePos.Z * scale  // Invert Z for top-down view
                                );
                                
                                // Color based on entity type or selection
                                var blipColor = entity.Id == _selectedTargetId ? _warningColor : _secondaryColor;
                                uint blipU32 = ImGui.ColorConvertFloat4ToU32(blipColor);
                                
                                drawList.AddCircleFilled(blipPos, _layout.Scale(4f), blipU32);
                                
                                // Pulse selected target
                                if (entity.Id == _selectedTargetId)
                                {
                                    float pulseRadius = _layout.Scale(8f) + _layout.Scale(4f) * MathF.Sin(_animationTime * 4f);
                                    drawList.AddCircle(blipPos, pulseRadius, blipU32, 16, _layout.GetLineThickness(2f));
                                }
                            }
                        }
                    }
                }
            }
            
            // Radar label
            ImGui.SetCursorPos(new Vector2(_layout.Scale(5f), _layout.Scale(5f)));
            ImGui.PushStyleColor(ImGuiCol.Text, _primaryColor);
            ImGui.Text("RADAR");
            ImGui.PopStyleColor();
            
            // Range indicator
            ImGui.SetCursorPos(new Vector2(_layout.Scale(5f), radarSize - _layout.Scale(25f)));
            ImGui.PushStyleColor(ImGuiCol.Text, _lineColor);
            ImGui.Text($"RNG: {_radarRange:F0}m");
            ImGui.PopStyleColor();
        }
        ImGui.End();
        ImGui.PopStyleColor(); // Border
    }
    
    /// <summary>
    /// Render ship status panel with visual gauges
    /// </summary>
    private void RenderShipStatus()
    {
        var displaySize = ImGui.GetIO().DisplaySize;
        float panelWidth = _layout.Scale(280f);
        float panelHeight = _layout.Scale(180f);
        var panelPos = new Vector2(_layout.Scale(30f), displaySize.Y - panelHeight - _layout.Scale(30f));
        
        ImGui.SetNextWindowPos(panelPos);
        ImGui.SetNextWindowSize(new Vector2(panelWidth, panelHeight));
        ImGui.SetNextWindowBgAlpha(0.2f);
        
        ImGuiWindowFlags flags = ImGuiWindowFlags.NoDecoration | 
                                ImGuiWindowFlags.NoMove | 
                                ImGuiWindowFlags.NoSavedSettings |
                                ImGuiWindowFlags.NoFocusOnAppearing;
        
        ImGui.PushStyleColor(ImGuiCol.Border, _lineColor);
        
        if (ImGui.Begin("##ShipStatus", flags))
        {
            ImGui.PushStyleColor(ImGuiCol.Text, _primaryColor);
            ImGui.Text("SHIP STATUS");
            ImGui.PopStyleColor();
            
            ImGui.Separator();
            
            // Get player ship
            var entities = _gameEngine.EntityManager.GetAllEntities();
            var playerShip = entities.FirstOrDefault();
            
            if (playerShip != null)
            {
                var voxelComp = _gameEngine.EntityManager.GetComponent<VoxelStructureComponent>(playerShip.Id);
                var physicsComp = _gameEngine.EntityManager.GetComponent<PhysicsComponent>(playerShip.Id);
                
                // Hull integrity (based on block count)
                float hullPercent = 1.0f;
                if (voxelComp != null && voxelComp.Blocks.Count > 0)
                {
                    // Simulate hull integrity (would be calculated from damage in real implementation)
                    hullPercent = 0.85f;
                }
                DrawGauge("HULL", hullPercent, _secondaryColor, _dangerColor);
                
                // Shield (simulated)
                float shieldPercent = 0.65f;
                DrawGauge("SHIELD", shieldPercent, _primaryColor, _warningColor);
                
                // Energy (simulated)
                float energyPercent = 0.90f;
                DrawGauge("ENERGY", energyPercent, new Vector4(1.0f, 1.0f, 0.3f, 1.0f), _warningColor);
                
                ImGui.Spacing();
                
                // Additional info
                if (voxelComp != null)
                {
                    ImGui.PushStyleColor(ImGuiCol.Text, _lineColor);
                    ImGui.Text($"Mass: {voxelComp.TotalMass:F1}t");
                    ImGui.Text($"Blocks: {voxelComp.Blocks.Count}");
                    ImGui.PopStyleColor();
                }
            }
            else
            {
                ImGui.PushStyleColor(ImGuiCol.Text, _warningColor);
                ImGui.Text("NO SHIP DETECTED");
                ImGui.PopStyleColor();
            }
        }
        ImGui.End();
        ImGui.PopStyleColor(); // Border
    }
    
    /// <summary>
    /// Draw a visual gauge bar
    /// </summary>
    private void DrawGauge(string label, float percent, Vector4 normalColor, Vector4 lowColor)
    {
        ImGui.PushStyleColor(ImGuiCol.Text, _lineColor);
        ImGui.Text($"{label}:");
        ImGui.PopStyleColor();
        
        ImGui.SameLine();
        
        // Determine color based on percentage
        var barColor = percent > 0.3f ? normalColor : lowColor;
        
        ImGui.PushStyleColor(ImGuiCol.PlotHistogram, barColor);
        ImGui.PushStyleColor(ImGuiCol.FrameBg, new Vector4(0.1f, 0.1f, 0.15f, 0.8f));
        
        ImGui.ProgressBar(percent, new Vector2(-1, _layout.Scale(20f)), $"{percent * 100:F0}%");
        
        ImGui.PopStyleColor(2);
    }
    
    /// <summary>
    /// Render target information panel
    /// </summary>
    private void RenderTargetInfo()
    {
        if (_selectedTargetId == Guid.Empty) return;
        
        var displaySize = ImGui.GetIO().DisplaySize;
        float panelWidth = _layout.Scale(280f);
        float panelHeight = _layout.Scale(200f);
        var panelPos = new Vector2(displaySize.X - panelWidth - _layout.Scale(30f), displaySize.Y / 2 - panelHeight / 2);
        
        ImGui.SetNextWindowPos(panelPos);
        ImGui.SetNextWindowSize(new Vector2(panelWidth, panelHeight));
        ImGui.SetNextWindowBgAlpha(0.2f);
        
        ImGuiWindowFlags flags = ImGuiWindowFlags.NoDecoration | 
                                ImGuiWindowFlags.NoMove | 
                                ImGuiWindowFlags.NoSavedSettings |
                                ImGuiWindowFlags.NoFocusOnAppearing;
        
        ImGui.PushStyleColor(ImGuiCol.Border, _warningColor);
        
        if (ImGui.Begin("##TargetInfo", flags))
        {
            ImGui.PushStyleColor(ImGuiCol.Text, _warningColor);
            ImGui.Text("TARGET ACQUIRED");
            ImGui.PopStyleColor();
            
            ImGui.Separator();
            
            // Get target entity
            var targetEntity = _gameEngine.EntityManager.GetAllEntities()
                .FirstOrDefault(e => e.Id == _selectedTargetId);
            
            if (targetEntity != null)
            {
                ImGui.PushStyleColor(ImGuiCol.Text, _lineColor);
                ImGui.Text($"ID: {_selectedTargetId.ToString("N")[..8]}");
                ImGui.PopStyleColor();
                
                var physics = _gameEngine.EntityManager.GetComponent<PhysicsComponent>(_selectedTargetId);
                if (physics != null)
                {
                    ImGui.Spacing();
                    ImGui.PushStyleColor(ImGuiCol.Text, _secondaryColor);
                    ImGui.Text("POSITION:");
                    ImGui.PopStyleColor();
                    ImGui.PushStyleColor(ImGuiCol.Text, _lineColor);
                    ImGui.Text($"  X: {physics.Position.X:F1}");
                    ImGui.Text($"  Y: {physics.Position.Y:F1}");
                    ImGui.Text($"  Z: {physics.Position.Z:F1}");
                    ImGui.PopStyleColor();
                    
                    // Calculate distance to player
                    var entities = _gameEngine.EntityManager.GetAllEntities();
                    var playerEntity = entities.FirstOrDefault();
                    if (playerEntity != null)
                    {
                        var playerPhysics = _gameEngine.EntityManager.GetComponent<PhysicsComponent>(playerEntity.Id);
                        if (playerPhysics != null)
                        {
                            float distance = Vector3.Distance(physics.Position, playerPhysics.Position);
                            ImGui.Spacing();
                            ImGui.PushStyleColor(ImGuiCol.Text, _primaryColor);
                            ImGui.Text($"DISTANCE: {distance:F1}m");
                            ImGui.PopStyleColor();
                        }
                    }
                }
                
                var voxel = _gameEngine.EntityManager.GetComponent<VoxelStructureComponent>(_selectedTargetId);
                if (voxel != null)
                {
                    ImGui.Spacing();
                    ImGui.PushStyleColor(ImGuiCol.Text, _secondaryColor);
                    ImGui.Text("STRUCTURE:");
                    ImGui.PopStyleColor();
                    ImGui.PushStyleColor(ImGuiCol.Text, _lineColor);
                    ImGui.Text($"  Mass: {voxel.TotalMass:F1}t");
                    ImGui.Text($"  Blocks: {voxel.Blocks.Count}");
                    ImGui.PopStyleColor();
                }
            }
            else
            {
                ImGui.PushStyleColor(ImGuiCol.Text, _dangerColor);
                ImGui.Text("TARGET LOST");
                ImGui.PopStyleColor();
                _selectedTargetId = Guid.Empty;
            }
        }
        ImGui.End();
        ImGui.PopStyleColor(); // Border
    }
    
    /// <summary>
    /// Render navigation compass showing direction
    /// </summary>
    private void RenderNavigationCompass()
    {
        var displaySize = ImGui.GetIO().DisplaySize;
        float compassSize = _layout.Scale(120f);
        var compassPos = new Vector2(displaySize.X / 2 - compassSize / 2, _layout.Scale(30f));
        
        ImGui.SetNextWindowPos(compassPos);
        ImGui.SetNextWindowSize(new Vector2(compassSize, compassSize));
        ImGui.SetNextWindowBgAlpha(0.2f);
        
        ImGuiWindowFlags flags = ImGuiWindowFlags.NoDecoration | 
                                ImGuiWindowFlags.NoMove | 
                                ImGuiWindowFlags.NoSavedSettings |
                                ImGuiWindowFlags.NoFocusOnAppearing;
        
        ImGui.PushStyleColor(ImGuiCol.Border, _lineColor);
        
        if (ImGui.Begin("##Compass", flags))
        {
            var drawList = ImGui.GetWindowDrawList();
            var center = new Vector2(compassPos.X + compassSize / 2, compassPos.Y + compassSize / 2);
            float radius = compassSize / 2 - _layout.Scale(15f);
            
            // Draw compass circle
            uint circleColor = ImGui.ColorConvertFloat4ToU32(_lineColor);
            drawList.AddCircle(center, radius, circleColor, 64, _layout.GetLineThickness(2f));
            
            // Draw cardinal directions
            float textOffset = radius + _layout.Scale(8f);
            
            // North
            var northPos = new Vector2(center.X, center.Y - textOffset);
            ImGui.SetCursorScreenPos(new Vector2(northPos.X - _layout.Scale(6f), northPos.Y - _layout.Scale(8f)));
            ImGui.PushStyleColor(ImGuiCol.Text, _primaryColor);
            ImGui.Text("N");
            ImGui.PopStyleColor();
            
            // East
            var eastPos = new Vector2(center.X + textOffset, center.Y);
            ImGui.SetCursorScreenPos(new Vector2(eastPos.X - _layout.Scale(6f), eastPos.Y - _layout.Scale(8f)));
            ImGui.PushStyleColor(ImGuiCol.Text, _lineColor);
            ImGui.Text("E");
            ImGui.PopStyleColor();
            
            // South
            var southPos = new Vector2(center.X, center.Y + textOffset);
            ImGui.SetCursorScreenPos(new Vector2(southPos.X - _layout.Scale(6f), southPos.Y - _layout.Scale(8f)));
            ImGui.PushStyleColor(ImGuiCol.Text, _lineColor);
            ImGui.Text("S");
            ImGui.PopStyleColor();
            
            // West
            var westPos = new Vector2(center.X - textOffset, center.Y);
            ImGui.SetCursorScreenPos(new Vector2(westPos.X - _layout.Scale(6f), westPos.Y - _layout.Scale(8f)));
            ImGui.PushStyleColor(ImGuiCol.Text, _lineColor);
            ImGui.Text("W");
            ImGui.PopStyleColor();
            
            // Draw heading indicator (pointing north for now)
            float heading = _animationTime * 0.2f; // Slowly rotate for demo
            var headingEnd = new Vector2(
                center.X + MathF.Sin(heading) * radius * 0.7f,
                center.Y - MathF.Cos(heading) * radius * 0.7f
            );
            uint headingColor = ImGui.ColorConvertFloat4ToU32(_warningColor);
            drawList.AddLine(center, headingEnd, headingColor, _layout.GetLineThickness(3f));
            
            // Draw arrow at the end
            float arrowSize = _layout.Scale(8f);
            var arrowLeft = new Vector2(
                headingEnd.X - MathF.Cos(heading) * arrowSize,
                headingEnd.Y - MathF.Sin(heading) * arrowSize
            );
            var arrowRight = new Vector2(
                headingEnd.X + MathF.Cos(heading) * arrowSize,
                headingEnd.Y + MathF.Sin(heading) * arrowSize
            );
            drawList.AddTriangleFilled(headingEnd, arrowLeft, arrowRight, headingColor);
        }
        ImGui.End();
        ImGui.PopStyleColor(); // Border
    }
    
    /// <summary>
    /// Render animated scan lines for holographic effect
    /// </summary>
    private void RenderScanLines()
    {
        var drawList = ImGui.GetBackgroundDrawList();
        var displaySize = ImGui.GetIO().DisplaySize;
        
        // Moving scan line
        float scanY = (_animationTime * 100f) % displaySize.Y;
        var scanColor = new Vector4(_secondaryColor.X, _secondaryColor.Y, _secondaryColor.Z, 0.1f);
        uint color = ImGui.ColorConvertFloat4ToU32(scanColor);
        
        drawList.AddLine(new Vector2(0, scanY), new Vector2(displaySize.X, scanY), color, _layout.GetLineThickness(2f));
        
        // Horizontal grid lines
        int lineSpacing = (int)_layout.Scale(60f);
        for (int i = 0; i < displaySize.Y; i += lineSpacing)
        {
            var gridColor = new Vector4(_lineColor.X, _lineColor.Y, _lineColor.Z, 0.02f);
            uint gridU32 = ImGui.ColorConvertFloat4ToU32(gridColor);
            drawList.AddLine(new Vector2(0, i), new Vector2(displaySize.X, i), gridU32, _layout.GetLineThickness(1f));
        }
    }
    
    /// <summary>
    /// Cycle to the next target
    /// </summary>
    private void CycleTarget()
    {
        var entities = _gameEngine.EntityManager.GetAllEntities().ToList();
        if (entities.Count <= 1) return; // Need at least 2 entities (player + target)
        
        var playerEntity = entities.FirstOrDefault();
        if (playerEntity == null) return;
        
        // Find current target index
        int currentIndex = -1;
        if (_selectedTargetId != Guid.Empty)
        {
            currentIndex = entities.FindIndex(e => e.Id == _selectedTargetId);
        }
        
        // Cycle to next entity (skip player)
        do
        {
            currentIndex = (currentIndex + 1) % entities.Count;
        }
        while (entities[currentIndex].Id == playerEntity.Id);
        
        _selectedTargetId = entities[currentIndex].Id;
    }
}
