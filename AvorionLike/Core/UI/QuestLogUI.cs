using ImGuiNET;
using System.Numerics;
using AvorionLike.Core.Quest;
using AvorionLike.Core.ECS;
using Quest = AvorionLike.Core.Quest.Quest;

namespace AvorionLike.Core.UI;

/// <summary>
/// UI for displaying quest log, active quests, and objectives
/// </summary>
public class QuestLogUI
{
    private readonly EntityManager _entityManager;
    private Guid _playerEntityId;
    
    // UI State
    private bool _showQuestLog = false;
    private bool _showObjectiveTracker = true;
    private string? _selectedQuestId = null;
    private QuestFilter _currentFilter = QuestFilter.Active;
    
    // Colors
    private static readonly Vector4 ColorActive = new(0.3f, 0.8f, 1.0f, 1.0f);      // Cyan
    private static readonly Vector4 ColorCompleted = new(0.3f, 1.0f, 0.3f, 1.0f);   // Green
    private static readonly Vector4 ColorFailed = new(1.0f, 0.3f, 0.3f, 1.0f);      // Red
    private static readonly Vector4 ColorAvailable = new(1.0f, 1.0f, 0.3f, 1.0f);   // Yellow
    private static readonly Vector4 ColorObjective = new(0.9f, 0.9f, 0.9f, 1.0f);   // Light gray
    private static readonly Vector4 ColorOptional = new(0.6f, 0.6f, 0.6f, 1.0f);    // Gray
    
    private enum QuestFilter
    {
        All,
        Active,
        Available,
        Completed
    }
    
    public QuestLogUI(EntityManager entityManager)
    {
        _entityManager = entityManager ?? throw new ArgumentNullException(nameof(entityManager));
    }
    
    /// <summary>
    /// Set the player entity ID to track quests for
    /// </summary>
    public void SetPlayerEntity(Guid playerEntityId)
    {
        _playerEntityId = playerEntityId;
    }
    
    /// <summary>
    /// Toggle quest log visibility
    /// </summary>
    public void ToggleQuestLog()
    {
        _showQuestLog = !_showQuestLog;
    }
    
    /// <summary>
    /// Toggle objective tracker visibility
    /// </summary>
    public void ToggleObjectiveTracker()
    {
        _showObjectiveTracker = !_showObjectiveTracker;
    }
    
    /// <summary>
    /// Render the quest UI
    /// </summary>
    public void Render()
    {
        if (_showQuestLog)
        {
            RenderQuestLog();
        }
        
        if (_showObjectiveTracker)
        {
            RenderObjectiveTracker();
        }
    }
    
    /// <summary>
    /// Render the main quest log window
    /// </summary>
    private void RenderQuestLog()
    {
        var questComponent = _entityManager.GetComponent<QuestComponent>(_playerEntityId);
        if (questComponent == null)
            return;
        
        ImGui.SetNextWindowSize(new Vector2(700, 500), ImGuiCond.FirstUseEver);
        ImGui.SetNextWindowPos(new Vector2(100, 100), ImGuiCond.FirstUseEver);
        
        if (ImGui.Begin("Quest Log [J]", ref _showQuestLog, ImGuiWindowFlags.NoCollapse))
        {
            // Filter tabs
            if (ImGui.BeginTabBar("QuestFilterTabs"))
            {
                if (ImGui.BeginTabItem("Active"))
                {
                    _currentFilter = QuestFilter.Active;
                    ImGui.EndTabItem();
                }
                if (ImGui.BeginTabItem("Available"))
                {
                    _currentFilter = QuestFilter.Available;
                    ImGui.EndTabItem();
                }
                if (ImGui.BeginTabItem("Completed"))
                {
                    _currentFilter = QuestFilter.Completed;
                    ImGui.EndTabItem();
                }
                if (ImGui.BeginTabItem("All"))
                {
                    _currentFilter = QuestFilter.All;
                    ImGui.EndTabItem();
                }
                ImGui.EndTabBar();
            }
            
            ImGui.Separator();
            
            // Two-column layout: Quest list on left, details on right
            if (ImGui.BeginTable("QuestLogTable", 2, ImGuiTableFlags.Resizable | ImGuiTableFlags.BordersInnerV))
            {
                ImGui.TableSetupColumn("Quests", ImGuiTableColumnFlags.WidthFixed, 250);
                ImGui.TableSetupColumn("Details", ImGuiTableColumnFlags.WidthStretch);
                
                ImGui.TableNextRow();
                ImGui.TableSetColumnIndex(0);
                
                // Quest list
                RenderQuestList(questComponent);
                
                ImGui.TableSetColumnIndex(1);
                
                // Quest details
                RenderQuestDetails(questComponent);
                
                ImGui.EndTable();
            }
        }
        ImGui.End();
    }
    
    /// <summary>
    /// Render the list of quests in the left column
    /// </summary>
    private void RenderQuestList(QuestComponent questComponent)
    {
        var quests = GetFilteredQuests(questComponent);
        
        if (quests.Count == 0)
        {
            ImGui.TextColored(ColorOptional, "No quests in this category.");
            return;
        }
        
        foreach (var quest in quests)
        {
            // Quest item
            bool isSelected = _selectedQuestId == quest.Id;
            var color = GetQuestColor(quest.Status);
            
            ImGui.PushStyleColor(ImGuiCol.Text, color);
            if (ImGui.Selectable($"{GetQuestIcon(quest.Status)} {quest.Title}###{quest.Id}", isSelected))
            {
                _selectedQuestId = quest.Id;
            }
            ImGui.PopStyleColor();
            
            // Show difficulty and completion
            if (quest.Status == QuestStatus.Active)
            {
                ImGui.SameLine();
                ImGui.TextColored(ColorOptional, $"({quest.CompletionPercentage:F0}%)");
            }
        }
    }
    
    /// <summary>
    /// Render quest details in the right column
    /// </summary>
    private void RenderQuestDetails(QuestComponent questComponent)
    {
        if (_selectedQuestId == null)
        {
            ImGui.TextColored(ColorOptional, "Select a quest to view details.");
            return;
        }
        
        var quest = questComponent.GetQuest(_selectedQuestId);
        if (quest == null)
        {
            ImGui.TextColored(ColorFailed, "Quest not found.");
            return;
        }
        
        // Title and status
        var statusColor = GetQuestColor(quest.Status);
        ImGui.PushStyleColor(ImGuiCol.Text, statusColor);
        ImGui.TextWrapped(quest.Title);
        ImGui.PopStyleColor();
        
        ImGui.SameLine();
        ImGui.TextColored(ColorOptional, $"[{quest.Difficulty}]");
        
        ImGui.Separator();
        
        // Description
        ImGui.TextWrapped(quest.Description);
        ImGui.Spacing();
        
        // Objectives
        ImGui.TextColored(ColorActive, "Objectives:");
        foreach (var objective in quest.Objectives)
        {
            if (objective.IsHidden && objective.Status == ObjectiveStatus.NotStarted)
                continue;
            
            var objColor = objective.IsComplete ? ColorCompleted :
                          objective.IsFailed ? ColorFailed :
                          objective.IsOptional ? ColorOptional : ColorObjective;
            
            string icon = objective.IsComplete ? "✓" : objective.IsFailed ? "✗" : "•";
            string optional = objective.IsOptional ? " (Optional)" : "";
            
            ImGui.TextColored(objColor, $"{icon} {objective.Description}{optional}");
            
            // Progress bar for active objectives
            if (objective.Status == ObjectiveStatus.Active && objective.RequiredQuantity > 1)
            {
                float progress = objective.CompletionPercentage / 100f;
                ImGui.ProgressBar(progress, new Vector2(-1, 0), 
                    $"{objective.CurrentProgress}/{objective.RequiredQuantity}");
            }
        }
        
        ImGui.Spacing();
        
        // Rewards
        if (quest.Rewards.Count > 0)
        {
            ImGui.TextColored(ColorActive, "Rewards:");
            foreach (var reward in quest.Rewards)
            {
                ImGui.BulletText($"{reward.Description}");
            }
            ImGui.Spacing();
        }
        
        // Time limit
        if (quest.TimeLimit > 0 && quest.Status == QuestStatus.Active)
        {
            int remaining = quest.TimeRemaining;
            var timeColor = remaining < 300 ? ColorFailed : ColorObjective;
            ImGui.TextColored(timeColor, $"Time Remaining: {FormatTime(remaining)}");
            ImGui.Spacing();
        }
        
        // Action buttons
        RenderQuestActionButtons(quest, questComponent);
    }
    
    /// <summary>
    /// Render action buttons for the selected quest
    /// </summary>
    private void RenderQuestActionButtons(AvorionLike.Core.Quest.Quest quest, QuestComponent questComponent)
    {
        switch (quest.Status)
        {
            case QuestStatus.Available:
                if (questComponent.CanAcceptMoreQuests)
                {
                    if (ImGui.Button("Accept Quest"))
                    {
                        questComponent.AcceptQuest(quest.Id);
                    }
                }
                else
                {
                    ImGui.TextColored(ColorFailed, "Quest log full! Abandon or complete a quest first.");
                }
                break;
            
            case QuestStatus.Active:
                if (quest.CanAbandon)
                {
                    if (ImGui.Button("Abandon Quest"))
                    {
                        questComponent.AbandonQuest(quest.Id);
                    }
                }
                break;
            
            case QuestStatus.Completed:
                if (ImGui.Button("Turn In Quest"))
                {
                    questComponent.TurnInQuest(quest.Id);
                }
                break;
        }
    }
    
    /// <summary>
    /// Render the compact objective tracker overlay
    /// </summary>
    private void RenderObjectiveTracker()
    {
        var questComponent = _entityManager.GetComponent<QuestComponent>(_playerEntityId);
        if (questComponent == null)
            return;
        
        var activeQuests = questComponent.ActiveQuests.ToList();
        if (activeQuests.Count == 0)
            return;
        
        // Position in top-right corner
        var viewport = ImGui.GetMainViewport();
        ImGui.SetNextWindowPos(new Vector2(viewport.WorkSize.X - 350, 50), ImGuiCond.Always);
        ImGui.SetNextWindowSize(new Vector2(340, 0), ImGuiCond.Always);
        
        ImGui.PushStyleVar(ImGuiStyleVar.WindowPadding, new Vector2(10, 10));
        ImGui.PushStyleVar(ImGuiStyleVar.WindowRounding, 5);
        ImGui.PushStyleColor(ImGuiCol.WindowBg, new Vector4(0.05f, 0.05f, 0.1f, 0.85f));
        
        if (ImGui.Begin("Quest Tracker", ref _showObjectiveTracker, 
            ImGuiWindowFlags.NoTitleBar | ImGuiWindowFlags.NoResize | 
            ImGuiWindowFlags.NoMove | ImGuiWindowFlags.NoCollapse))
        {
            ImGui.TextColored(ColorActive, "ACTIVE QUESTS");
            ImGui.Separator();
            
            foreach (var quest in activeQuests.Take(3)) // Show max 3 quests
            {
                ImGui.Spacing();
                
                // Quest title
                ImGui.TextColored(ColorActive, quest.Title);
                
                // Time remaining if applicable
                if (quest.TimeLimit > 0)
                {
                    int remaining = quest.TimeRemaining;
                    var timeColor = remaining < 300 ? ColorFailed : ColorOptional;
                    ImGui.SameLine();
                    ImGui.TextColored(timeColor, $"({FormatTime(remaining)})");
                }
                
                // Show active objectives only
                var activeObjectives = quest.Objectives
                    .Where(o => o.Status == ObjectiveStatus.Active && !o.IsHidden)
                    .ToList();
                
                foreach (var objective in activeObjectives.Take(3)) // Show max 3 objectives per quest
                {
                    string icon = objective.IsOptional ? "○" : "•";
                    ImGui.TextColored(ColorObjective, $"{icon} {objective.Description}");
                    
                    if (objective.RequiredQuantity > 1)
                    {
                        ImGui.SameLine();
                        ImGui.TextColored(ColorOptional, 
                            $"({objective.CurrentProgress}/{objective.RequiredQuantity})");
                    }
                }
                
                ImGui.Separator();
            }
            
            ImGui.Spacing();
            ImGui.TextColored(ColorOptional, "Press [J] to open Quest Log");
        }
        ImGui.End();
        
        ImGui.PopStyleColor();
        ImGui.PopStyleVar(2);
    }
    
    /// <summary>
    /// Get quests filtered by current filter
    /// </summary>
    private List<AvorionLike.Core.Quest.Quest> GetFilteredQuests(QuestComponent questComponent)
    {
        return _currentFilter switch
        {
            QuestFilter.Active => questComponent.ActiveQuests.ToList(),
            QuestFilter.Available => questComponent.AvailableQuests.ToList(),
            QuestFilter.Completed => questComponent.CompletedQuests.ToList(),
            _ => questComponent.Quests.ToList()
        };
    }
    
    /// <summary>
    /// Get color for quest status
    /// </summary>
    private Vector4 GetQuestColor(QuestStatus status)
    {
        return status switch
        {
            QuestStatus.Active => ColorActive,
            QuestStatus.Completed => ColorCompleted,
            QuestStatus.Failed => ColorFailed,
            QuestStatus.Available => ColorAvailable,
            _ => ColorObjective
        };
    }
    
    /// <summary>
    /// Get icon for quest status
    /// </summary>
    private string GetQuestIcon(QuestStatus status)
    {
        return status switch
        {
            QuestStatus.Active => "►",
            QuestStatus.Completed => "✓",
            QuestStatus.Failed => "✗",
            QuestStatus.Available => "!",
            _ => "•"
        };
    }
    
    /// <summary>
    /// Format seconds into a readable time string
    /// </summary>
    private string FormatTime(int seconds)
    {
        if (seconds < 60)
            return $"{seconds}s";
        
        int minutes = seconds / 60;
        int secs = seconds % 60;
        
        if (minutes < 60)
            return $"{minutes}m {secs}s";
        
        int hours = minutes / 60;
        minutes = minutes % 60;
        return $"{hours}h {minutes}m";
    }
}
