using ImGuiNET;
using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Physics;
using AvorionLike.Core.Resources;
using AvorionLike.Core.RPG;

namespace AvorionLike.Core.UI;

/// <summary>
/// Heads-Up Display (HUD) system for displaying game information
/// </summary>
public class HUDSystem
{
    private readonly GameEngine _gameEngine;
    private bool _showDebugInfo = false;
    private bool _showEntityInfo = false;
    private bool _showResourceInfo = false;
    
    public HUDSystem(GameEngine gameEngine)
    {
        _gameEngine = gameEngine;
    }
    
    public void Render()
    {
        RenderMainHUD();
        
        if (_showDebugInfo)
            RenderDebugOverlay();
        
        if (_showEntityInfo)
            RenderEntityList();
        
        if (_showResourceInfo)
            RenderResourcePanel();
    }
    
    private void RenderMainHUD()
    {
        // Top-left corner HUD
        ImGui.SetNextWindowPos(new Vector2(10, 10));
        ImGui.SetNextWindowBgAlpha(0.35f);
        
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags.NoDecoration | 
                                       ImGuiWindowFlags.AlwaysAutoResize | 
                                       ImGuiWindowFlags.NoSavedSettings | 
                                       ImGuiWindowFlags.NoFocusOnAppearing | 
                                       ImGuiWindowFlags.NoNav;
        
        bool open = true;
        if (ImGui.Begin("HUD", ref open, windowFlags))
        {
            ImGui.Text("AvorionLike Engine");
            ImGui.Separator();
            
            // Display FPS
            var io = ImGui.GetIO();
            ImGui.Text($"FPS: {io.Framerate:F1}");
            ImGui.Text($"Frame Time: {1000.0f / io.Framerate:F2} ms");
            
            ImGui.Separator();
            
            // Entity count
            var entities = _gameEngine.EntityManager.GetAllEntities();
            ImGui.Text($"Entities: {entities.Count()}");
            
            ImGui.Separator();
            
            // Controls help
            ImGui.TextColored(new Vector4(0.5f, 0.8f, 1.0f, 1.0f), "Controls:");
            ImGui.Text("WASD - Move Camera");
            ImGui.Text("Space/Shift - Up/Down");
            ImGui.Text("Mouse - Look Around");
            ImGui.Text("F1 - Toggle Debug Info");
            ImGui.Text("F2 - Toggle Entity List");
            ImGui.Text("F3 - Toggle Resources");
            ImGui.Text("I - Toggle Inventory");
            ImGui.Text("ALT - Show Mouse Cursor");
            ImGui.Text("ESC - Pause Menu");
        }
        ImGui.End();
        
        // Render centered crosshair
        RenderCrosshair();
    }
    
    private void RenderCrosshair()
    {
        var io = ImGui.GetIO();
        var drawList = ImGui.GetForegroundDrawList();
        
        // Center of screen
        Vector2 center = new Vector2(io.DisplaySize.X * 0.5f, io.DisplaySize.Y * 0.5f);
        
        // Draw a small dot as crosshair
        float dotRadius = 2.0f;
        uint color = ImGui.ColorConvertFloat4ToU32(new Vector4(1.0f, 1.0f, 1.0f, 0.8f));
        
        // Draw filled circle for the dot
        drawList.AddCircleFilled(center, dotRadius, color, 12);
        
        // Optional: Add a subtle outline for better visibility
        uint outlineColor = ImGui.ColorConvertFloat4ToU32(new Vector4(0.0f, 0.0f, 0.0f, 0.5f));
        drawList.AddCircle(center, dotRadius + 1, outlineColor, 12, 1.5f);
    }
    
    private void RenderDebugOverlay()
    {
        ImGui.SetNextWindowPos(new Vector2(ImGui.GetIO().DisplaySize.X - 310, 10));
        ImGui.SetNextWindowSize(new Vector2(300, 400), ImGuiCond.FirstUseEver);
        ImGui.SetNextWindowBgAlpha(0.35f);
        
        if (ImGui.Begin("Debug Info", ref _showDebugInfo))
        {
            ImGui.Text("System Information");
            ImGui.Separator();
            
            // Memory info
            var mem = GC.GetTotalMemory(false) / 1024.0 / 1024.0;
            ImGui.Text($"Memory: {mem:F2} MB");
            ImGui.Text($"GC Gen 0: {GC.CollectionCount(0)}");
            ImGui.Text($"GC Gen 1: {GC.CollectionCount(1)}");
            ImGui.Text($"GC Gen 2: {GC.CollectionCount(2)}");
            
            ImGui.Separator();
            
            // System counts
            var entities = _gameEngine.EntityManager.GetAllEntities();
            ImGui.Text($"Total Entities: {entities.Count()}");
            
            int physicsCount = 0;
            int voxelCount = 0;
            int inventoryCount = 0;
            
            foreach (var entity in entities)
            {
                if (_gameEngine.EntityManager.GetComponent<PhysicsComponent>(entity.Id) != null)
                    physicsCount++;
                if (_gameEngine.EntityManager.GetComponent<Core.Voxel.VoxelStructureComponent>(entity.Id) != null)
                    voxelCount++;
                if (_gameEngine.EntityManager.GetComponent<InventoryComponent>(entity.Id) != null)
                    inventoryCount++;
            }
            
            ImGui.Text($"Physics Bodies: {physicsCount}");
            ImGui.Text($"Voxel Structures: {voxelCount}");
            ImGui.Text($"Inventories: {inventoryCount}");
        }
        ImGui.End();
    }
    
    private void RenderEntityList()
    {
        ImGui.SetNextWindowPos(new Vector2(10, ImGui.GetIO().DisplaySize.Y - 410));
        ImGui.SetNextWindowSize(new Vector2(400, 400), ImGuiCond.FirstUseEver);
        ImGui.SetNextWindowBgAlpha(0.35f);
        
        if (ImGui.Begin("Entity List", ref _showEntityInfo))
        {
            var entities = _gameEngine.EntityManager.GetAllEntities();
            
            ImGui.Text($"Entities ({entities.Count()})");
            ImGui.Separator();
            
            if (ImGui.BeginChild("EntityScrolling"))
            {
                foreach (var entity in entities)
                {
                    if (ImGui.TreeNode($"Entity {entity.Id}"))
                    {
                        // Show components
                        var physicsComp = _gameEngine.EntityManager.GetComponent<PhysicsComponent>(entity.Id);
                        if (physicsComp != null)
                        {
                            ImGui.BulletText($"Position: ({physicsComp.Position.X:F1}, {physicsComp.Position.Y:F1}, {physicsComp.Position.Z:F1})");
                            ImGui.BulletText($"Velocity: ({physicsComp.Velocity.X:F1}, {physicsComp.Velocity.Y:F1}, {physicsComp.Velocity.Z:F1})");
                            ImGui.BulletText($"Mass: {physicsComp.Mass:F1}");
                        }
                        
                        var voxelComp = _gameEngine.EntityManager.GetComponent<Core.Voxel.VoxelStructureComponent>(entity.Id);
                        if (voxelComp != null)
                        {
                            ImGui.BulletText($"Voxel Blocks: {voxelComp.Blocks.Count}");
                            ImGui.BulletText($"Total Mass: {voxelComp.TotalMass:F1}");
                        }
                        
                        var inventoryComp = _gameEngine.EntityManager.GetComponent<InventoryComponent>(entity.Id);
                        if (inventoryComp != null)
                        {
                            var allResources = inventoryComp.Inventory.GetAllResources();
                            int itemCount = allResources.Count(kvp => kvp.Value > 0);
                            ImGui.BulletText($"Inventory: {itemCount} resource types");
                            ImGui.BulletText($"Capacity: {inventoryComp.Inventory.CurrentCapacity} / {inventoryComp.Inventory.MaxCapacity}");
                        }
                        
                        var progressionComp = _gameEngine.EntityManager.GetComponent<ProgressionComponent>(entity.Id);
                        if (progressionComp != null)
                        {
                            ImGui.BulletText($"Level: {progressionComp.Level}");
                            ImGui.BulletText($"XP: {progressionComp.Experience} / {progressionComp.ExperienceToNextLevel}");
                        }
                        
                        ImGui.TreePop();
                    }
                }
            }
            ImGui.EndChild();
        }
        ImGui.End();
    }
    
    private void RenderResourcePanel()
    {
        ImGui.SetNextWindowPos(new Vector2(ImGui.GetIO().DisplaySize.X - 310, 420));
        ImGui.SetNextWindowSize(new Vector2(300, 300), ImGuiCond.FirstUseEver);
        ImGui.SetNextWindowBgAlpha(0.35f);
        
        if (ImGui.Begin("Resources", ref _showResourceInfo))
        {
            ImGui.Text("Global Resources");
            ImGui.Separator();
            
            var entities = _gameEngine.EntityManager.GetAllEntities();
            
            // Collect all resources from all inventories
            var totalResources = new Dictionary<ResourceType, int>();
            
            foreach (var entity in entities)
            {
                var inventoryComp = _gameEngine.EntityManager.GetComponent<InventoryComponent>(entity.Id);
                if (inventoryComp != null)
                {
                    var resources = inventoryComp.Inventory.GetAllResources();
                    foreach (var kvp in resources)
                    {
                        if (kvp.Value > 0)
                        {
                            if (totalResources.ContainsKey(kvp.Key))
                                totalResources[kvp.Key] += kvp.Value;
                            else
                                totalResources[kvp.Key] = kvp.Value;
                        }
                    }
                }
            }
            
            if (totalResources.Count > 0)
            {
                foreach (var kvp in totalResources.OrderBy(x => x.Key))
                {
                    ImGui.Text($"{kvp.Key}: {kvp.Value}");
                }
            }
            else
            {
                ImGui.TextColored(new Vector4(0.7f, 0.7f, 0.7f, 1.0f), "No resources available");
            }
        }
        ImGui.End();
    }
    
    public void HandleInput()
    {
        // Toggle debug windows with function keys
        if (ImGui.IsKeyPressed(ImGuiKey.F1))
            _showDebugInfo = !_showDebugInfo;
        
        if (ImGui.IsKeyPressed(ImGuiKey.F2))
            _showEntityInfo = !_showEntityInfo;
        
        if (ImGui.IsKeyPressed(ImGuiKey.F3))
            _showResourceInfo = !_showResourceInfo;
    }
}
