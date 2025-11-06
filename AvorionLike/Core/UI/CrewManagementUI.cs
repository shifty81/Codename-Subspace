using System.Numerics;
using ImGuiNET;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Fleet;
using AvorionLike.Core.Resources;

namespace AvorionLike.Core.UI;

/// <summary>
/// UI for managing ship crew and pilots
/// </summary>
public class CrewManagementUI
{
    private readonly GameEngine _gameEngine;
    private bool _isOpen = false;
    private Guid? _selectedShipId = null;
    
    // For hiring
    private Guid? _currentStationId = null;
    private List<Pilot> _stationPilots = new();
    
    public bool IsOpen => _isOpen;
    
    public CrewManagementUI(GameEngine gameEngine)
    {
        _gameEngine = gameEngine;
    }
    
    public void Show(Guid? shipId = null, Guid? stationId = null)
    {
        _isOpen = true;
        _selectedShipId = shipId;
        _currentStationId = stationId;
        
        if (stationId.HasValue)
        {
            _stationPilots = _gameEngine.CrewManagementSystem.GetStationPilots(stationId.Value);
        }
    }
    
    public void Hide()
    {
        _isOpen = false;
        _selectedShipId = null;
        _currentStationId = null;
        _stationPilots.Clear();
    }
    
    public void Toggle()
    {
        if (_isOpen)
            Hide();
        else
            Show();
    }
    
    public void Render()
    {
        if (!_isOpen) return;
        
        var io = ImGui.GetIO();
        ImGui.SetNextWindowPos(new Vector2(io.DisplaySize.X * 0.5f, io.DisplaySize.Y * 0.5f), 
            ImGuiCond.FirstUseEver, new Vector2(0.5f, 0.5f));
        ImGui.SetNextWindowSize(new Vector2(900, 650), ImGuiCond.FirstUseEver);
        
        if (ImGui.Begin("Crew & Pilot Management", ref _isOpen))
        {
            if (ImGui.BeginTabBar("CrewTabs"))
            {
                if (ImGui.BeginTabItem("Ship Management"))
                {
                    RenderShipManagement();
                    ImGui.EndTabItem();
                }
                
                if (ImGui.BeginTabItem("Hire Pilots"))
                {
                    RenderPilotHiring();
                    ImGui.EndTabItem();
                }
                
                if (ImGui.BeginTabItem("Available Pilots"))
                {
                    RenderAvailablePilots();
                    ImGui.EndTabItem();
                }
                
                ImGui.EndTabBar();
            }
        }
        ImGui.End();
    }
    
    private void RenderShipManagement()
    {
        // Ship selector
        RenderShipSelector();
        
        ImGui.Separator();
        
        if (!_selectedShipId.HasValue)
        {
            ImGui.TextColored(new Vector4(0.7f, 0.7f, 0.7f, 1f), 
                "Select a ship to manage crew and pilot.");
            return;
        }
        
        var crew = _gameEngine.EntityManager.GetComponent<CrewComponent>(_selectedShipId.Value);
        if (crew == null)
        {
            ImGui.TextColored(new Vector4(1f, 0.5f, 0.2f, 1f), 
                "Selected ship has no crew component.");
            return;
        }
        
        var entity = _gameEngine.EntityManager.GetEntity(_selectedShipId.Value);
        var shipClass = _gameEngine.EntityManager.GetComponent<ShipClassComponent>(_selectedShipId.Value);
        
        ImGui.TextColored(new Vector4(0.4f, 0.8f, 1f, 1f), $"Ship: {entity?.Name ?? "Unknown"}");
        if (shipClass != null)
        {
            ImGui.Text($"Class: {shipClass.ShipClass}");
            ImGui.Text($"Mission Readiness: {shipClass.MissionReadiness:F0}%");
        }
        
        ImGui.Separator();
        
        // Pilot section
        ImGui.TextColored(new Vector4(1f, 1f, 0.4f, 1f), "Pilot");
        
        if (crew.AssignedPilot != null)
        {
            RenderPilotInfo(crew.AssignedPilot);
            
            ImGui.Spacing();
            if (ImGui.Button("Remove Pilot", new Vector2(200, 30)))
            {
                _gameEngine.CrewManagementSystem.RemovePilotFromShip(_selectedShipId.Value);
            }
        }
        else
        {
            ImGui.TextColored(new Vector4(1f, 0.5f, 0.2f, 1f), "No pilot assigned!");
            ImGui.Text("Assign a pilot from the Available Pilots tab.");
            
            // Quick assign from available
            var availablePilots = _gameEngine.CrewManagementSystem.AvailablePilots;
            if (availablePilots.Count > 0)
            {
                ImGui.Spacing();
                ImGui.Text("Quick Assign:");
                
                if (ImGui.BeginCombo("##QuickPilot", "Select Pilot..."))
                {
                    foreach (var pilot in availablePilots)
                    {
                        if (ImGui.Selectable($"{pilot.Name} (Lvl {pilot.Level})"))
                        {
                            _gameEngine.CrewManagementSystem.AssignPilotToShip(pilot, _selectedShipId.Value);
                        }
                    }
                    ImGui.EndCombo();
                }
            }
        }
        
        ImGui.Separator();
        
        // Crew section
        ImGui.TextColored(new Vector4(1f, 1f, 0.4f, 1f), "Crew");
        
        ImGui.Text($"Current Crew: {crew.CurrentCrew} / {crew.MaxCrew}");
        ImGui.Text($"Minimum Required: {crew.MinimumCrew}");
        
        var crewColor = crew.HasSufficientCrew() 
            ? new Vector4(0.4f, 1f, 0.4f, 1f) 
            : new Vector4(1f, 0.4f, 0.4f, 1f);
        ImGui.TextColored(crewColor, 
            crew.HasSufficientCrew() ? "ADEQUATELY CREWED" : "UNDERSTAFFED!");
        
        ImGui.Text($"Crew Efficiency: {crew.GetCrewEfficiency() * 100f:F0}%");
        
        ImGui.Spacing();
        
        // Hire crew
        ImGui.Text("Hire Additional Crew:");
        
        int hireCount = 1;
        ImGui.SetNextItemWidth(100);
        ImGui.InputInt("##HireCount", ref hireCount, 1, 10);
        hireCount = Math.Clamp(hireCount, 1, crew.MaxCrew - crew.CurrentCrew);
        
        ImGui.SameLine();
        int hireCost = hireCount * 500;
        
        if (ImGui.Button($"Hire {hireCount} Crew ({hireCost} credits)", new Vector2(250, 30)))
        {
            // Get player inventory
            var playerEntity = GetPlayerEntity();
            if (playerEntity != null)
            {
                var inventory = _gameEngine.EntityManager.GetComponent<InventoryComponent>(playerEntity.Id);
                if (inventory != null)
                {
                    _gameEngine.CrewManagementSystem.HireCrew(_selectedShipId.Value, hireCount, inventory.Inventory);
                }
            }
        }
        
        ImGui.Separator();
        
        // Operational status
        ImGui.TextColored(new Vector4(1f, 1f, 0.4f, 1f), "Operational Status");
        
        bool isOperational = crew.IsOperational();
        var statusColor = isOperational 
            ? new Vector4(0.4f, 1f, 0.4f, 1f) 
            : new Vector4(1f, 0.4f, 0.4f, 1f);
        
        ImGui.TextColored(statusColor, isOperational ? "OPERATIONAL" : "NOT OPERATIONAL");
        
        if (!isOperational)
        {
            ImGui.BulletText("Ship requires a pilot and sufficient crew to operate.");
            ImGui.BulletText("Player pod can dock to take direct control.");
        }
    }
    
    private void RenderShipSelector()
    {
        ImGui.Text("Select Ship:");
        ImGui.SameLine();
        
        var ships = _gameEngine.EntityManager.GetAllEntities()
            .Where(e => _gameEngine.EntityManager.GetComponent<CrewComponent>(e.Id) != null)
            .ToList();
        
        if (ships.Count == 0)
        {
            ImGui.TextColored(new Vector4(1f, 0.5f, 0.2f, 1f), "No ships found.");
            return;
        }
        
        string currentLabel = _selectedShipId.HasValue 
            ? GetShipLabel(_selectedShipId.Value) 
            : "Select...";
        
        if (ImGui.BeginCombo("##ShipSelector", currentLabel))
        {
            foreach (var ship in ships)
            {
                bool isSelected = _selectedShipId == ship.Id;
                string label = GetShipLabel(ship.Id);
                
                if (ImGui.Selectable(label, isSelected))
                {
                    _selectedShipId = ship.Id;
                }
                
                if (isSelected)
                    ImGui.SetItemDefaultFocus();
            }
            ImGui.EndCombo();
        }
    }
    
    private string GetShipLabel(Guid shipId)
    {
        var entity = _gameEngine.EntityManager.GetEntity(shipId);
        if (entity == null) return "Unknown";
        
        var shipClass = _gameEngine.EntityManager.GetComponent<ShipClassComponent>(shipId);
        if (shipClass != null)
        {
            return $"[{shipClass.ShipClass}] {entity.Name}";
        }
        
        return entity.Name;
    }
    
    private void RenderPilotHiring()
    {
        if (!_currentStationId.HasValue)
        {
            ImGui.TextColored(new Vector4(1f, 0.5f, 0.2f, 1f), 
                "Dock at a station to hire pilots.");
            
            // Debug: Generate test station
            if (ImGui.Button("Generate Test Station Pilots"))
            {
                _currentStationId = Guid.NewGuid();
                _stationPilots = _gameEngine.CrewManagementSystem.GenerateStationPilots(_currentStationId.Value, 6);
            }
            
            return;
        }
        
        ImGui.TextColored(new Vector4(0.4f, 0.8f, 1f, 1f), "Station Pilot Roster");
        ImGui.Separator();
        
        if (_stationPilots.Count == 0)
        {
            ImGui.Text("No pilots available for hire at this station.");
            return;
        }
        
        if (ImGui.BeginChild("PilotRoster"))
        {
            foreach (var pilot in _stationPilots)
            {
                ImGui.PushID(pilot.Id.ToString());
                
                RenderPilotInfo(pilot);
                
                ImGui.Spacing();
                if (ImGui.Button($"Hire for {pilot.HiringCost} credits", new Vector2(200, 30)))
                {
                    var playerEntity = GetPlayerEntity();
                    if (playerEntity != null)
                    {
                        var inventory = _gameEngine.EntityManager.GetComponent<InventoryComponent>(playerEntity.Id);
                        if (inventory != null)
                        {
                            if (_gameEngine.CrewManagementSystem.HirePilot(_currentStationId.Value, pilot, inventory.Inventory))
                            {
                                // Refresh list
                                _stationPilots = _gameEngine.CrewManagementSystem.GetStationPilots(_currentStationId.Value);
                            }
                        }
                    }
                }
                
                ImGui.PopID();
                ImGui.Separator();
            }
        }
        ImGui.EndChild();
    }
    
    private void RenderAvailablePilots()
    {
        var pilots = _gameEngine.CrewManagementSystem.AvailablePilots;
        
        if (pilots.Count == 0)
        {
            ImGui.TextColored(new Vector4(0.7f, 0.7f, 0.7f, 1f), 
                "No pilots in reserve. Hire pilots from stations.");
            return;
        }
        
        ImGui.TextColored(new Vector4(0.4f, 0.8f, 1f, 1f), $"Available Pilots ({pilots.Count})");
        ImGui.Separator();
        
        if (ImGui.BeginChild("AvailablePilotsList"))
        {
            foreach (var pilot in pilots)
            {
                ImGui.PushID(pilot.Id.ToString());
                
                RenderPilotInfo(pilot);
                
                ImGui.Spacing();
                ImGui.Text("Assign to ship:");
                ImGui.SameLine();
                
                if (ImGui.BeginCombo("##AssignShip", "Select Ship..."))
                {
                    var ships = _gameEngine.EntityManager.GetAllEntities()
                        .Where(e => _gameEngine.EntityManager.GetComponent<CrewComponent>(e.Id) != null)
                        .ToList();
                    
                    foreach (var ship in ships)
                    {
                        if (ImGui.Selectable(GetShipLabel(ship.Id)))
                        {
                            _gameEngine.CrewManagementSystem.AssignPilotToShip(pilot, ship.Id);
                            break;
                        }
                    }
                    
                    ImGui.EndCombo();
                }
                
                ImGui.PopID();
                ImGui.Separator();
            }
        }
        ImGui.EndChild();
    }
    
    private void RenderPilotInfo(Pilot pilot)
    {
        ImGui.TextColored(new Vector4(1f, 1f, 0.4f, 1f), pilot.Name);
        ImGui.Text($"Level: {pilot.Level} | XP: {pilot.Experience}");
        
        if (pilot.Specialization.HasValue)
        {
            ImGui.TextColored(new Vector4(0.4f, 1f, 0.4f, 1f), 
                $"Specialization: {pilot.Specialization.Value}");
        }
        
        ImGui.Text("Skills:");
        ImGui.Indent();
        
        // Combat skill bar
        ImGui.Text("Combat:");
        ImGui.SameLine();
        ImGui.ProgressBar(pilot.CombatSkill, new Vector2(200, 0), $"{pilot.CombatSkill * 100f:F0}%");
        
        // Navigation skill bar
        ImGui.Text("Navigation:");
        ImGui.SameLine();
        ImGui.ProgressBar(pilot.NavigationSkill, new Vector2(200, 0), $"{pilot.NavigationSkill * 100f:F0}%");
        
        // Engineering skill bar
        ImGui.Text("Engineering:");
        ImGui.SameLine();
        ImGui.ProgressBar(pilot.EngineeringSkill, new Vector2(200, 0), $"{pilot.EngineeringSkill * 100f:F0}%");
        
        ImGui.Unindent();
        
        ImGui.Text($"Daily Salary: {pilot.DailySalary} credits");
    }
    
    private Entity? GetPlayerEntity()
    {
        var entities = _gameEngine.EntityManager.GetAllEntities();
        foreach (var entity in entities)
        {
            var pod = _gameEngine.EntityManager.GetComponent<RPG.PlayerPodComponent>(entity.Id);
            if (pod != null)
            {
                return entity;
            }
        }
        
        // Fallback to first entity with inventory
        return entities.FirstOrDefault(e => 
            _gameEngine.EntityManager.GetComponent<InventoryComponent>(e.Id) != null);
    }
    
    public void HandleInput()
    {
        // Toggle with C key (for Crew)
        if (ImGui.IsKeyPressed(ImGuiKey.C))
        {
            Toggle();
        }
    }
}
