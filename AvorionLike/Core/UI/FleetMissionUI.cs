using System.Numerics;
using ImGuiNET;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Fleet;

namespace AvorionLike.Core.UI;

/// <summary>
/// UI for managing fleet missions
/// </summary>
public class FleetMissionUI
{
    private readonly GameEngine _gameEngine;
    private bool _isOpen = false;
    
    // Mission assignment
    private FleetMission? _selectedMission = null;
    private readonly List<Guid> _selectedShips = new();
    
    // Tab selection - reserved for future UI expansion
#pragma warning disable CS0414 // Field is assigned but its value is never used
    private int _selectedTab = 0; // 0 = Available, 1 = Active, 2 = Completed
#pragma warning restore CS0414
    
    public bool IsOpen => _isOpen;
    
    public FleetMissionUI(GameEngine gameEngine)
    {
        _gameEngine = gameEngine;
    }
    
    public void Show()
    {
        _isOpen = true;
    }
    
    public void Hide()
    {
        _isOpen = false;
        _selectedMission = null;
        _selectedShips.Clear();
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
        ImGui.SetNextWindowSize(new Vector2(1000, 700), ImGuiCond.FirstUseEver);
        
        if (ImGui.Begin("Fleet Missions", ref _isOpen))
        {
            // Tab bar
            if (ImGui.BeginTabBar("MissionTabs"))
            {
                if (ImGui.BeginTabItem("Available Missions"))
                {
                    _selectedTab = 0;
                    RenderAvailableMissions();
                    ImGui.EndTabItem();
                }
                
                if (ImGui.BeginTabItem("Active Missions"))
                {
                    _selectedTab = 1;
                    RenderActiveMissions();
                    ImGui.EndTabItem();
                }
                
                if (ImGui.BeginTabItem("Completed Missions"))
                {
                    _selectedTab = 2;
                    RenderCompletedMissions();
                    ImGui.EndTabItem();
                }
                
                ImGui.EndTabBar();
            }
        }
        ImGui.End();
    }
    
    private void RenderAvailableMissions()
    {
        var missions = _gameEngine.FleetMissionSystem.AvailableMissions;
        
        if (missions.Count == 0)
        {
            ImGui.TextColored(new Vector4(0.7f, 0.7f, 0.7f, 1f), 
                "No missions available. Check back later.");
            
            if (ImGui.Button("Generate Test Missions", new Vector2(-1, 30)))
            {
                _gameEngine.FleetMissionSystem.GenerateMissions(5, 10);
            }
            return;
        }
        
        // Two columns: mission list and details
        if (ImGui.BeginTable("AvailableMissionsTable", 2, ImGuiTableFlags.Resizable | ImGuiTableFlags.BordersInnerV))
        {
            ImGui.TableSetupColumn("Missions", ImGuiTableColumnFlags.WidthFixed, 350);
            ImGui.TableSetupColumn("Mission Details & Assignment", ImGuiTableColumnFlags.WidthStretch);
            
            ImGui.TableNextRow();
            
            // Left column: Mission list
            ImGui.TableSetColumnIndex(0);
            RenderMissionList(missions.ToList());
            
            // Right column: Details and ship assignment
            ImGui.TableSetColumnIndex(1);
            if (_selectedMission != null)
            {
                RenderMissionDetails(_selectedMission);
                ImGui.Separator();
                RenderShipAssignment();
            }
            else
            {
                ImGui.TextColored(new Vector4(0.7f, 0.7f, 0.7f, 1f), 
                    "Select a mission to view details.");
            }
            
            ImGui.EndTable();
        }
    }
    
    private void RenderMissionList(List<FleetMission> missions)
    {
        ImGui.TextColored(new Vector4(0.4f, 0.8f, 1f, 1f), "Available Missions");
        ImGui.Separator();
        
        if (ImGui.BeginChild("MissionList", new Vector2(0, 0)))
        {
            foreach (var mission in missions)
            {
                ImGui.PushID(mission.Id.ToString());
                
                var difficultyColor = GetDifficultyColor(mission.Difficulty);
                var typeColor = GetMissionTypeColor(mission.Type);
                
                // Mission button
                ImGui.PushStyleColor(ImGuiCol.Button, typeColor * 0.3f);
                ImGui.PushStyleColor(ImGuiCol.ButtonHovered, typeColor * 0.5f);
                ImGui.PushStyleColor(ImGuiCol.ButtonActive, typeColor * 0.7f);
                
                bool isSelected = _selectedMission == mission;
                if (ImGui.Selectable($"##mission_{mission.Id}", isSelected, ImGuiSelectableFlags.None, new Vector2(0, 60)))
                {
                    _selectedMission = mission;
                    _selectedShips.Clear();
                }
                
                ImGui.PopStyleColor(3);
                
                // Draw mission info over selectable
                var drawList = ImGui.GetWindowDrawList();
                var cursorPos = ImGui.GetCursorScreenPos();
                cursorPos.Y -= 60;
                
                ImGui.SetCursorScreenPos(cursorPos + new Vector2(5, 5));
                ImGui.TextColored(typeColor, $"[{mission.Type}]");
                
                ImGui.SetCursorScreenPos(cursorPos + new Vector2(5, 25));
                ImGui.Text(mission.Name);
                
                ImGui.SetCursorScreenPos(cursorPos + new Vector2(5, 40));
                ImGui.TextColored(difficultyColor, $"Difficulty: {mission.Difficulty}");
                
                ImGui.PopID();
            }
        }
        ImGui.EndChild();
    }
    
    private void RenderMissionDetails(FleetMission mission)
    {
        ImGui.TextColored(new Vector4(1f, 1f, 0.4f, 1f), mission.Name);
        ImGui.Separator();
        
        ImGui.Text($"Type: {mission.Type}");
        
        var difficultyColor = GetDifficultyColor(mission.Difficulty);
        ImGui.TextColored(difficultyColor, $"Difficulty: {mission.Difficulty}");
        
        ImGui.Text($"Location: {mission.SectorName}");
        ImGui.Text($"Duration: {mission.Duration:F1} hours");
        ImGui.Text($"Ships Required: {mission.MinShips} - {mission.MaxShips}");
        
        if (mission.PreferredClass.HasValue)
        {
            ImGui.Text($"Preferred Class: {mission.PreferredClass.Value}");
        }
        
        ImGui.Spacing();
        ImGui.TextWrapped($"Description: {mission.Description}");
        
        ImGui.Spacing();
        ImGui.Separator();
        ImGui.TextColored(new Vector4(0.4f, 1f, 0.4f, 1f), "Estimated Rewards:");
        ImGui.BulletText($"Credits: ~{(int)mission.Difficulty * 1000}");
        ImGui.BulletText($"Experience: ~{(int)mission.Difficulty * 500}");
        ImGui.BulletText("Chance for subsystems and blueprints");
    }
    
    private void RenderShipAssignment()
    {
        if (_selectedMission == null) return;
        
        ImGui.TextColored(new Vector4(0.4f, 0.8f, 1f, 1f), "Assign Ships");
        ImGui.Separator();
        
        var availableShips = _gameEngine.FleetMissionSystem.GetAvailableShips();
        
        if (availableShips.Count == 0)
        {
            ImGui.TextColored(new Vector4(1f, 0.5f, 0.2f, 1f), 
                "No ships available for missions. Ensure ships have crew, pilots, and sufficient readiness.");
            return;
        }
        
        ImGui.Text($"Selected: {_selectedShips.Count} / {_selectedMission.MaxShips}");
        ImGui.Text($"Required: {_selectedMission.MinShips} minimum");
        ImGui.Spacing();
        
        if (ImGui.BeginChild("ShipSelection", new Vector2(0, -50)))
        {
            foreach (var shipId in availableShips)
            {
                var entity = _gameEngine.EntityManager.GetEntity(shipId);
                if (entity == null) continue;
                
                var shipClass = _gameEngine.EntityManager.GetComponent<ShipClassComponent>(shipId);
                var crew = _gameEngine.EntityManager.GetComponent<CrewComponent>(shipId);
                
                ImGui.PushID(shipId.ToString());
                
                bool isSelected = _selectedShips.Contains(shipId);
                bool isPreferredClass = shipClass != null && 
                    _selectedMission.PreferredClass.HasValue &&
                    shipClass.ShipClass == _selectedMission.PreferredClass.Value;
                
                if (isPreferredClass)
                {
                    ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.4f, 1f, 0.4f, 1f));
                }
                
                if (ImGui.Checkbox($"##ship_{shipId}", ref isSelected))
                {
                    if (isSelected && !_selectedShips.Contains(shipId))
                    {
                        if (_selectedShips.Count < _selectedMission.MaxShips)
                        {
                            _selectedShips.Add(shipId);
                        }
                    }
                    else if (!isSelected && _selectedShips.Contains(shipId))
                    {
                        _selectedShips.Remove(shipId);
                    }
                }
                
                ImGui.SameLine();
                
                string shipLabel = entity.Name;
                if (shipClass != null)
                {
                    shipLabel = $"[{shipClass.ShipClass}] {entity.Name}";
                }
                
                ImGui.Text(shipLabel);
                
                if (isPreferredClass)
                {
                    ImGui.PopStyleColor();
                }
                
                // Show ship stats
                if (crew != null)
                {
                    ImGui.Indent();
                    ImGui.TextColored(new Vector4(0.7f, 0.7f, 0.7f, 1f), 
                        $"Readiness: {shipClass?.MissionReadiness:F0}% | Crew: {crew.CurrentCrew}/{crew.MinimumCrew}");
                    ImGui.Unindent();
                }
                
                ImGui.PopID();
                ImGui.Spacing();
            }
        }
        ImGui.EndChild();
        
        // Start mission button
        bool canStart = _selectedShips.Count >= _selectedMission.MinShips && 
                       _selectedShips.Count <= _selectedMission.MaxShips;
        
        if (!canStart)
        {
            ImGui.BeginDisabled();
        }
        
        if (ImGui.Button("Start Mission", new Vector2(-1, 40)))
        {
            if (_gameEngine.FleetMissionSystem.StartMission(_selectedMission, _selectedShips))
            {
                _selectedMission = null;
                _selectedShips.Clear();
            }
        }
        
        if (!canStart)
        {
            ImGui.EndDisabled();
        }
    }
    
    private void RenderActiveMissions()
    {
        var missions = _gameEngine.FleetMissionSystem.ActiveMissions;
        
        if (missions.Count == 0)
        {
            ImGui.TextColored(new Vector4(0.7f, 0.7f, 0.7f, 1f), "No active missions.");
            return;
        }
        
        if (ImGui.BeginTable("ActiveMissionsTable", 7, ImGuiTableFlags.Borders | ImGuiTableFlags.RowBg))
        {
            ImGui.TableSetupColumn("Name");
            ImGui.TableSetupColumn("Type");
            ImGui.TableSetupColumn("Difficulty");
            ImGui.TableSetupColumn("Ships");
            ImGui.TableSetupColumn("Progress");
            ImGui.TableSetupColumn("Success Rate");
            ImGui.TableSetupColumn("Actions");
            ImGui.TableHeadersRow();
            
            foreach (var mission in missions)
            {
                ImGui.TableNextRow();
                
                ImGui.TableSetColumnIndex(0);
                ImGui.Text(mission.Name);
                
                ImGui.TableSetColumnIndex(1);
                ImGui.TextColored(GetMissionTypeColor(mission.Type), mission.Type.ToString());
                
                ImGui.TableSetColumnIndex(2);
                ImGui.TextColored(GetDifficultyColor(mission.Difficulty), mission.Difficulty.ToString());
                
                ImGui.TableSetColumnIndex(3);
                ImGui.Text(mission.AssignedShipIds.Count.ToString());
                
                ImGui.TableSetColumnIndex(4);
                var elapsed = DateTime.UtcNow - mission.StartTime;
                var progress = (float)elapsed.TotalMinutes / (mission.Duration * 10f);
                ImGui.ProgressBar(Math.Clamp(progress, 0f, 1f), new Vector2(-1, 0));
                
                ImGui.TableSetColumnIndex(5);
                ImGui.Text($"{mission.SuccessRate * 100f:F0}%");
                
                ImGui.TableSetColumnIndex(6);
                ImGui.PushID(mission.Id.ToString());
                if (ImGui.SmallButton("Abort"))
                {
                    _gameEngine.FleetMissionSystem.AbortMission(mission.Id);
                }
                ImGui.PopID();
            }
            
            ImGui.EndTable();
        }
    }
    
    private void RenderCompletedMissions()
    {
        var missions = _gameEngine.FleetMissionSystem.CompletedMissions;
        
        if (missions.Count == 0)
        {
            ImGui.TextColored(new Vector4(0.7f, 0.7f, 0.7f, 1f), "No completed missions.");
            return;
        }
        
        if (ImGui.BeginChild("CompletedMissionsList"))
        {
            foreach (var mission in missions.TakeLast(20).Reverse())
            {
                ImGui.PushID(mission.Id.ToString());
                
                var statusColor = mission.Status == MissionStatus.Completed 
                    ? new Vector4(0.4f, 1f, 0.4f, 1f) 
                    : new Vector4(1f, 0.4f, 0.4f, 1f);
                
                ImGui.TextColored(statusColor, $"[{mission.Status}]");
                ImGui.SameLine();
                ImGui.Text($"{mission.Name} - {mission.Type}");
                
                if (mission.WasSuccessful)
                {
                    ImGui.Indent();
                    ImGui.TextColored(new Vector4(0.4f, 1f, 0.4f, 1f), "SUCCESS");
                    ImGui.Text($"Rewards: {mission.CreditsReward} credits, {mission.ExperienceReward} XP");
                    
                    if (mission.RewardSubsystems.Count > 0)
                    {
                        ImGui.Text($"Subsystems found: {mission.RewardSubsystems.Count}");
                    }
                    
                    if (mission.RewardBlueprints.Count > 0)
                    {
                        ImGui.Text($"Blueprints discovered: {string.Join(", ", mission.RewardBlueprints)}");
                    }
                    
                    ImGui.Unindent();
                }
                else
                {
                    ImGui.Indent();
                    ImGui.TextColored(new Vector4(1f, 0.4f, 0.4f, 1f), "FAILED");
                    ImGui.Text(mission.ResultMessage);
                    ImGui.Unindent();
                }
                
                ImGui.PopID();
                ImGui.Separator();
            }
        }
        ImGui.EndChild();
    }
    
    private Vector4 GetDifficultyColor(MissionDifficulty difficulty)
    {
        return difficulty switch
        {
            MissionDifficulty.Easy => new Vector4(0.4f, 1f, 0.4f, 1f),       // Green
            MissionDifficulty.Normal => new Vector4(0.4f, 0.8f, 1f, 1f),     // Cyan
            MissionDifficulty.Hard => new Vector4(1f, 1f, 0.4f, 1f),         // Yellow
            MissionDifficulty.VeryHard => new Vector4(1f, 0.6f, 0.2f, 1f),   // Orange
            MissionDifficulty.Extreme => new Vector4(1f, 0.2f, 0.2f, 1f),    // Red
            _ => new Vector4(1f, 1f, 1f, 1f)
        };
    }
    
    private Vector4 GetMissionTypeColor(MissionType type)
    {
        return type switch
        {
            MissionType.Combat => new Vector4(1f, 0.3f, 0.3f, 1f),           // Red
            MissionType.Explore => new Vector4(0.4f, 0.8f, 1f, 1f),          // Blue
            MissionType.Mine => new Vector4(0.8f, 0.6f, 0.2f, 1f),           // Brown
            MissionType.Salvage => new Vector4(0.6f, 0.6f, 0.6f, 1f),        // Gray
            MissionType.Trade => new Vector4(0.4f, 1f, 0.4f, 1f),            // Green
            MissionType.Reconnaissance => new Vector4(0.6f, 0.4f, 0.8f, 1f), // Purple
            _ => new Vector4(1f, 1f, 1f, 1f)
        };
    }
    
    public void HandleInput()
    {
        // Toggle with M key (for Missions)
        if (ImGui.IsKeyPressed(ImGuiKey.M))
        {
            Toggle();
        }
    }
}
