using ImGuiNET;
using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Physics;
using AvorionLike.Core.RPG;
using AvorionLike.Core.Resources;
using AvorionLike.Core.Combat;
using AvorionLike.Core.Navigation;

namespace AvorionLike.Core.UI;

/// <summary>
/// Manages all player-facing UI panels and interactions
/// Coordinates HUD, inventory, ship builder, and other player UI elements
/// </summary>
public class PlayerUIManager
{
    private readonly GameEngine _gameEngine;
    private readonly HUDSystem _hudSystem;
    private readonly MenuSystem _menuSystem;
    private readonly InventoryUI _inventoryUI;
    private readonly ShipBuilderUI _shipBuilderUI;
    private readonly FuturisticHUD _futuristicHUD;
    private readonly CrewManagementUI _crewManagementUI;
    private readonly SubsystemManagementUI _subsystemManagementUI;
    private readonly FleetMissionUI _fleetMissionUI;
    private readonly GalaxyMapUI _galaxyMapUI;
    
    private Guid? _playerShipId;
    private bool _showPlayerStatus = true;
    private bool _showMissionInfo = false;
    
    public bool IsAnyPanelOpen => _menuSystem.IsMenuOpen || _inventoryUI.IsOpen || 
                                   _shipBuilderUI.IsOpen || _crewManagementUI.IsOpen ||
                                   _subsystemManagementUI.IsOpen || _fleetMissionUI.IsOpen ||
                                   _galaxyMapUI.IsOpen;
    
    public Guid? PlayerShipId
    {
        get => _playerShipId;
        set
        {
            _playerShipId = value;
            _galaxyMapUI.PlayerShipId = value;
        }
    }
    
    public PlayerUIManager(
        GameEngine gameEngine,
        HUDSystem hudSystem,
        MenuSystem menuSystem,
        InventoryUI inventoryUI,
        ShipBuilderUI shipBuilderUI,
        FuturisticHUD futuristicHUD,
        CrewManagementUI crewManagementUI,
        SubsystemManagementUI subsystemManagementUI,
        FleetMissionUI fleetMissionUI,
        GalaxyMapUI galaxyMapUI)
    {
        _gameEngine = gameEngine;
        _hudSystem = hudSystem;
        _menuSystem = menuSystem;
        _inventoryUI = inventoryUI;
        _shipBuilderUI = shipBuilderUI;
        _futuristicHUD = futuristicHUD;
        _crewManagementUI = crewManagementUI;
        _subsystemManagementUI = subsystemManagementUI;
        _fleetMissionUI = fleetMissionUI;
        _galaxyMapUI = galaxyMapUI;
    }
    
    public void HandleInput()
    {
        var io = ImGui.GetIO();
        
        // Toggle player status panel
        if (ImGui.IsKeyPressed(ImGuiKey.Tab) && !io.WantCaptureKeyboard)
        {
            _showPlayerStatus = !_showPlayerStatus;
        }
        
        // Toggle mission info
        if (ImGui.IsKeyPressed(ImGuiKey.J) && !io.WantCaptureKeyboard)
        {
            _showMissionInfo = !_showMissionInfo;
        }
        
        // Delegate to individual UI systems
        _hudSystem.HandleInput();
        _menuSystem.HandleInput();
        _inventoryUI.HandleInput();
        _shipBuilderUI.HandleInput();
        _futuristicHUD.HandleInput();
        _crewManagementUI.HandleInput();
        _subsystemManagementUI.HandleInput();
        _fleetMissionUI.HandleInput();
        _galaxyMapUI.HandleInput();
    }
    
    public void Render()
    {
        if (_menuSystem.IsMenuOpen)
        {
            // Only render menu when open
            _menuSystem.Render();
        }
        else
        {
            // Render main game HUD
            _hudSystem.Render();
            _futuristicHUD.Render();
            
            // Render player-specific status
            if (_showPlayerStatus && _playerShipId.HasValue)
            {
                RenderPlayerStatus();
            }
            
            // Render mission info
            if (_showMissionInfo)
            {
                RenderMissionInfo();
            }
            
            // Render open panels
            _inventoryUI.Render();
            _shipBuilderUI.Render();
            _crewManagementUI.Render();
            _subsystemManagementUI.Render();
            _fleetMissionUI.Render();
            _galaxyMapUI.Render();
        }
    }
    
    private void RenderPlayerStatus()
    {
        if (!_playerShipId.HasValue) return;
        
        var entity = _gameEngine.EntityManager.GetEntity(_playerShipId.Value);
        if (entity == null) return;
        
        // Position at bottom center
        var io = ImGui.GetIO();
        ImGui.SetNextWindowPos(new Vector2(io.DisplaySize.X * 0.5f, io.DisplaySize.Y - 150), ImGuiCond.Always, new Vector2(0.5f, 0.0f));
        ImGui.SetNextWindowBgAlpha(0.8f);
        
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags.NoDecoration | 
                                       ImGuiWindowFlags.AlwaysAutoResize | 
                                       ImGuiWindowFlags.NoSavedSettings | 
                                       ImGuiWindowFlags.NoFocusOnAppearing | 
                                       ImGuiWindowFlags.NoNav;
        
        if (ImGui.Begin("PlayerStatus", windowFlags))
        {
            ImGui.TextColored(new Vector4(0.3f, 0.8f, 1.0f, 1.0f), entity.Name);
            ImGui.Separator();
            
            // Get components
            var physics = _gameEngine.EntityManager.GetComponent<PhysicsComponent>(_playerShipId.Value);
            var progression = _gameEngine.EntityManager.GetComponent<ProgressionComponent>(_playerShipId.Value);
            var inventory = _gameEngine.EntityManager.GetComponent<InventoryComponent>(_playerShipId.Value);
            var combat = _gameEngine.EntityManager.GetComponent<CombatComponent>(_playerShipId.Value);
            
            // Display key stats
            if (physics != null)
            {
                ImGui.Text($"Position: ({physics.Position.X:F0}, {physics.Position.Y:F0}, {physics.Position.Z:F0})");
                ImGui.Text($"Velocity: {physics.Velocity.Length():F1} m/s");
            }
            
            if (combat != null)
            {
                float shieldPercent = combat.MaxShields > 0 ? (combat.CurrentShields / combat.MaxShields) * 100f : 0f;
                ImGui.Text($"Shields: {combat.CurrentShields:F0} / {combat.MaxShields:F0} ({shieldPercent:F0}%%)");
            }
            
            if (progression != null)
            {
                ImGui.Text($"Level: {progression.Level}  XP: {progression.Experience}/{progression.ExperienceToNextLevel}");
            }
            
            if (inventory != null)
            {
                float capacityPercent = inventory.Inventory.MaxCapacity > 0 ? 
                    (inventory.Inventory.CurrentCapacity / (float)inventory.Inventory.MaxCapacity) * 100f : 0f;
                ImGui.Text($"Cargo: {inventory.Inventory.CurrentCapacity}/{inventory.Inventory.MaxCapacity} ({capacityPercent:F0}%%)");
                ImGui.Text($"Credits: {inventory.Inventory.GetResourceAmount(ResourceType.Credits):N0}");
            }
            
            ImGui.Separator();
            ImGui.TextColored(new Vector4(0.7f, 0.7f, 0.7f, 1.0f), "Press TAB to toggle");
        }
        ImGui.End();
    }
    
    private void RenderMissionInfo()
    {
        // Position at top right
        var io = ImGui.GetIO();
        ImGui.SetNextWindowPos(new Vector2(io.DisplaySize.X - 10, 10), ImGuiCond.Always, new Vector2(1.0f, 0.0f));
        ImGui.SetNextWindowSize(new Vector2(300, 200), ImGuiCond.FirstUseEver);
        ImGui.SetNextWindowBgAlpha(0.8f);
        
        if (ImGui.Begin("Mission Info", ref _showMissionInfo))
        {
            ImGui.TextColored(new Vector4(1.0f, 0.8f, 0.3f, 1.0f), "CURRENT OBJECTIVES");
            ImGui.Separator();
            
            ImGui.Text("• Explore the galaxy");
            ImGui.Text("• Build and upgrade your ship");
            ImGui.Text("• Mine resources from asteroids");
            ImGui.Text("• Trade with stations");
            ImGui.Text("• Complete missions");
            
            ImGui.Separator();
            ImGui.TextColored(new Vector4(0.7f, 0.7f, 0.7f, 1.0f), "Press J to toggle");
        }
        ImGui.End();
    }
    
    public void Update(float deltaTime)
    {
        // Update any time-based UI elements if needed
    }
}
