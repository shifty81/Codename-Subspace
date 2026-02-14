using ImGuiNET;
using System.Numerics;
using AvorionLike.Core.Tutorial;

namespace AvorionLike.Core.UI;

/// <summary>
/// UI for displaying tutorials and tutorial progress
/// </summary>
public class TutorialUI
{
    private readonly TutorialSystem _tutorialSystem;
    private Guid _playerEntityId;
    
    // UI State
    private bool _showTutorialOverlay = true;
    private bool _showTutorialList = false;
    
    // Contextual tooltips keyed by UI element identifier
    private readonly Dictionary<string, ContextualTooltip> _tooltips = new();
    
    // Colors
    private static readonly Vector4 ColorTitle = new(1.0f, 0.85f, 0.3f, 1.0f);      // Gold
    private static readonly Vector4 ColorMessage = new(0.95f, 0.95f, 0.95f, 1.0f);  // White
    private static readonly Vector4 ColorHint = new(0.6f, 0.8f, 1.0f, 1.0f);        // Light blue
    private static readonly Vector4 ColorProgress = new(0.3f, 1.0f, 0.3f, 1.0f);    // Green
    private static readonly Vector4 ColorBackground = new(0.05f, 0.05f, 0.1f, 0.92f); // Dark blue-black
    private static readonly Vector4 ColorTooltipBg = new(0.08f, 0.12f, 0.2f, 0.95f);  // Tooltip background
    
    public TutorialUI(TutorialSystem tutorialSystem)
    {
        _tutorialSystem = tutorialSystem ?? throw new ArgumentNullException(nameof(tutorialSystem));
    }
    
    /// <summary>
    /// Set the player entity ID to track tutorials for
    /// </summary>
    public void SetPlayerEntity(Guid playerEntityId)
    {
        _playerEntityId = playerEntityId;
    }
    
    /// <summary>
    /// Toggle tutorial overlay visibility
    /// </summary>
    public void ToggleTutorialOverlay()
    {
        _showTutorialOverlay = !_showTutorialOverlay;
    }
    
    /// <summary>
    /// Toggle tutorial list visibility
    /// </summary>
    public void ToggleTutorialList()
    {
        _showTutorialList = !_showTutorialList;
    }
    
    /// <summary>
    /// Render the tutorial UI
    /// </summary>
    public void Render()
    {
        if (_showTutorialOverlay)
        {
            RenderTutorialOverlay();
        }
        
        if (_showTutorialList)
        {
            RenderTutorialList();
        }
        
        RenderContextualTooltips();
    }
    
    /// <summary>
    /// Register a contextual tooltip for a UI element.
    /// The tooltip is shown when the user hovers over the element during an active tutorial.
    /// </summary>
    /// <param name="elementId">Unique identifier matching TutorialStep.UIElementId</param>
    /// <param name="title">Short tooltip title</param>
    /// <param name="description">Detailed description shown in the tooltip body</param>
    public void RegisterTooltip(string elementId, string title, string description)
    {
        _tooltips[elementId] = new ContextualTooltip(elementId, title, description);
    }
    
    /// <summary>
    /// Remove a previously registered tooltip
    /// </summary>
    public void UnregisterTooltip(string elementId)
    {
        _tooltips.Remove(elementId);
    }
    
    /// <summary>
    /// Show a contextual tooltip for the last ImGui item if a matching tooltip is registered.
    /// Call this immediately after rendering a UI element that should have a tooltip.
    /// </summary>
    /// <param name="elementId">The element identifier to look up</param>
    public void ShowTooltipIfRegistered(string elementId)
    {
        if (!_showTutorialOverlay)
            return;
        
        if (!_tooltips.TryGetValue(elementId, out var tooltip))
            return;
        
        if (!ImGui.IsItemHovered())
            return;
        
        RenderTooltipPopup(tooltip);
    }
    
    /// <summary>
    /// Render the tutorial overlay showing current tutorial step
    /// </summary>
    private void RenderTutorialOverlay()
    {
        var activeTutorials = _tutorialSystem.GetActiveTutorials(_playerEntityId);
        if (activeTutorials.Count == 0)
            return;
            
        var currentTutorial = activeTutorials.FirstOrDefault(t => t.Status == TutorialStatus.Active);
        if (currentTutorial == null || currentTutorial.CurrentStep == null)
            return;
            
        var step = currentTutorial.CurrentStep;
        
        // Position in center-top area
        var viewport = ImGui.GetMainViewport();
        var windowWidth = 500f;
        var posX = (viewport.WorkSize.X - windowWidth) / 2f;
        var posY = 80f;
        
        ImGui.SetNextWindowPos(new Vector2(posX, posY), ImGuiCond.Always);
        ImGui.SetNextWindowSize(new Vector2(windowWidth, 0), ImGuiCond.Always);
        
        ImGui.PushStyleVar(ImGuiStyleVar.WindowPadding, new Vector2(15, 15));
        ImGui.PushStyleVar(ImGuiStyleVar.WindowRounding, 8);
        ImGui.PushStyleColor(ImGuiCol.WindowBg, ColorBackground);
        ImGui.PushStyleColor(ImGuiCol.Border, new Vector4(0.3f, 0.6f, 1.0f, 0.6f));
        
        var flags = ImGuiWindowFlags.NoTitleBar | 
                   ImGuiWindowFlags.NoResize | 
                   ImGuiWindowFlags.NoMove |
                   ImGuiWindowFlags.NoCollapse |
                   ImGuiWindowFlags.NoScrollbar;
        
        if (ImGui.Begin("TutorialOverlay", ref _showTutorialOverlay, flags))
        {
            // Tutorial title
            ImGui.PushStyleColor(ImGuiCol.Text, ColorTitle);
            ImGui.TextWrapped($"📚 {currentTutorial.Title}");
            ImGui.PopStyleColor();
            
            ImGui.Separator();
            ImGui.Spacing();
            
            // Step title
            if (!string.IsNullOrEmpty(step.Title))
            {
                ImGui.TextColored(ColorHint, step.Title);
                ImGui.Spacing();
            }
            
            // Step message
            ImGui.PushTextWrapPos(ImGui.GetContentRegionAvail().X);
            ImGui.TextColored(ColorMessage, step.Message);
            ImGui.PopTextWrapPos();
            
            ImGui.Spacing();
            ImGui.Separator();
            ImGui.Spacing();
            
            // Progress indicator
            var progress = currentTutorial.CompletionPercentage / 100f;
            var progressText = $"Step {currentTutorial.CurrentStepIndex + 1}/{currentTutorial.Steps.Count}";
            ImGui.TextColored(ColorProgress, progressText);
            ImGui.SameLine();
            ImGui.ProgressBar(progress, new Vector2(-1, 0), $"{currentTutorial.CompletionPercentage:F0}%");
            
            ImGui.Spacing();
            
            // Action buttons
            ImGui.BeginGroup();
            
            // Show appropriate button based on step type
            switch (step.Type)
            {
                case TutorialStepType.Message:
                    if (ImGui.Button("Continue", new Vector2(120, 30)))
                    {
                        _tutorialSystem.CompleteCurrentStep(_playerEntityId, currentTutorial.Id);
                    }
                    break;
                    
                case TutorialStepType.WaitForKey:
                    ImGui.TextColored(ColorHint, $"Press [{step.RequiredKey}] to continue");
                    break;
                    
                case TutorialStepType.WaitForAction:
                    ImGui.TextColored(ColorHint, "Waiting for action...");
                    break;
                    
                case TutorialStepType.WaitForTime:
                    if (step.StartTime.HasValue)
                    {
                        var elapsed = (DateTime.UtcNow - step.StartTime.Value).TotalSeconds;
                        var remaining = Math.Max(0, step.Duration - elapsed);
                        ImGui.TextColored(ColorHint, $"Wait {remaining:F0}s...");
                    }
                    break;
            }
            
            ImGui.SameLine(ImGui.GetContentRegionAvail().X - 70);
            
            if (step.CanSkip && ImGui.Button("Skip", new Vector2(70, 30)))
            {
                _tutorialSystem.SkipTutorial(_playerEntityId, currentTutorial.Id);
            }
            
            ImGui.EndGroup();
            
            ImGui.Spacing();
            ImGui.TextColored(new Vector4(0.5f, 0.5f, 0.5f, 1.0f), "Press [H] to toggle tutorial overlay");
        }
        ImGui.End();
        
        ImGui.PopStyleColor(2);
        ImGui.PopStyleVar(2);
    }
    
    /// <summary>
    /// Render the tutorial list window
    /// </summary>
    private void RenderTutorialList()
    {
        ImGui.SetNextWindowSize(new Vector2(500, 400), ImGuiCond.FirstUseEver);
        ImGui.SetNextWindowPos(new Vector2(100, 100), ImGuiCond.FirstUseEver);
        
        if (ImGui.Begin("Tutorial List", ref _showTutorialList, ImGuiWindowFlags.NoCollapse))
        {
            var allTutorials = _tutorialSystem.GetTutorialTemplates();
            var activeTutorials = _tutorialSystem.GetActiveTutorials(_playerEntityId);
            
            ImGui.TextColored(ColorTitle, "Available Tutorials");
            ImGui.Separator();
            ImGui.Spacing();
            
            if (allTutorials.Count == 0)
            {
                ImGui.TextColored(new Vector4(0.6f, 0.6f, 0.6f, 1.0f), "No tutorials available.");
            }
            else
            {
                foreach (var kvp in allTutorials)
                {
                    var tutorial = kvp.Value;
                    var isActive = activeTutorials.Any(t => t.Id == tutorial.Id);
                    var isCompleted = _tutorialSystem.HasCompletedTutorial(_playerEntityId, tutorial.Id);
                    
                    string prefix = isCompleted ? "✓" : isActive ? "►" : "•";
                    var color = isCompleted ? ColorProgress : isActive ? ColorHint : ColorMessage;
                    
                    ImGui.PushStyleColor(ImGuiCol.Text, color);
                    if (ImGui.CollapsingHeader($"{prefix} {tutorial.Title}"))
                    {
                        ImGui.PopStyleColor();
                        
                        ImGui.Indent();
                        ImGui.TextWrapped(tutorial.Description);
                        ImGui.Spacing();
                        
                        ImGui.TextColored(new Vector4(0.7f, 0.7f, 0.7f, 1.0f), 
                            $"Steps: {tutorial.Steps.Count}");
                        
                        if (!isActive && !isCompleted)
                        {
                            ImGui.Spacing();
                            if (ImGui.Button($"Start##{tutorial.Id}"))
                            {
                                _tutorialSystem.StartTutorial(_playerEntityId, tutorial.Id);
                            }
                        }
                        else if (isActive)
                        {
                            var activeTutorial = activeTutorials.First(t => t.Id == tutorial.Id);
                            ImGui.Spacing();
                            ImGui.TextColored(ColorProgress, 
                                $"Progress: {activeTutorial.CompletionPercentage:F0}%");
                            ImGui.ProgressBar(activeTutorial.CompletionPercentage / 100f);
                            
                            if (ImGui.Button($"Skip Tutorial##{tutorial.Id}"))
                            {
                                _tutorialSystem.SkipTutorial(_playerEntityId, tutorial.Id);
                            }
                        }
                        
                        ImGui.Unindent();
                    }
                    else
                    {
                        ImGui.PopStyleColor();
                    }
                }
            }
        }
        ImGui.End();
    }
    
    /// <summary>
    /// Render contextual tooltips for the current tutorial step's highlighted UI element
    /// </summary>
    private void RenderContextualTooltips()
    {
        var activeTutorials = _tutorialSystem.GetActiveTutorials(_playerEntityId);
        if (activeTutorials.Count == 0)
            return;
        
        var currentTutorial = activeTutorials.FirstOrDefault(t => t.Status == TutorialStatus.Active);
        if (currentTutorial?.CurrentStep == null)
            return;
        
        var step = currentTutorial.CurrentStep;
        
        // Show tooltip indicator when the step highlights a specific UI element
        if (step.Type == TutorialStepType.HighlightUI &&
            !string.IsNullOrEmpty(step.UIElementId) &&
            _tooltips.TryGetValue(step.UIElementId, out var tooltip))
        {
            RenderFloatingTooltip(tooltip, step.Message);
        }
    }
    
    /// <summary>
    /// Render a floating tooltip anchored to the bottom-right of the screen
    /// </summary>
    private void RenderFloatingTooltip(ContextualTooltip tooltip, string stepMessage)
    {
        var viewport = ImGui.GetMainViewport();
        var tooltipWidth = 350f;
        var posX = viewport.WorkSize.X - tooltipWidth - 20f;
        var posY = viewport.WorkSize.Y - 160f;
        
        ImGui.SetNextWindowPos(new Vector2(posX, posY), ImGuiCond.Always);
        ImGui.SetNextWindowSize(new Vector2(tooltipWidth, 0), ImGuiCond.Always);
        
        ImGui.PushStyleVar(ImGuiStyleVar.WindowPadding, new Vector2(12, 10));
        ImGui.PushStyleVar(ImGuiStyleVar.WindowRounding, 6);
        ImGui.PushStyleColor(ImGuiCol.WindowBg, ColorTooltipBg);
        ImGui.PushStyleColor(ImGuiCol.Border, new Vector4(0.4f, 0.7f, 1.0f, 0.7f));
        
        var flags = ImGuiWindowFlags.NoTitleBar | ImGuiWindowFlags.NoResize |
                    ImGuiWindowFlags.NoMove | ImGuiWindowFlags.NoCollapse |
                    ImGuiWindowFlags.NoScrollbar | ImGuiWindowFlags.NoFocusOnAppearing;
        
        bool open = true;
        if (ImGui.Begin("ContextualTooltip", ref open, flags))
        {
            ImGui.TextColored(ColorTitle, $"💡 {tooltip.Title}");
            ImGui.Separator();
            ImGui.Spacing();
            ImGui.PushTextWrapPos(ImGui.GetContentRegionAvail().X);
            ImGui.TextColored(ColorMessage, tooltip.Description);
            if (!string.IsNullOrEmpty(stepMessage))
            {
                ImGui.Spacing();
                ImGui.TextColored(ColorHint, stepMessage);
            }
            ImGui.PopTextWrapPos();
        }
        ImGui.End();
        
        ImGui.PopStyleColor(2);
        ImGui.PopStyleVar(2);
    }
    
    /// <summary>
    /// Render a tooltip popup for the last hovered item
    /// </summary>
    private void RenderTooltipPopup(ContextualTooltip tooltip)
    {
        ImGui.PushStyleVar(ImGuiStyleVar.WindowPadding, new Vector2(10, 8));
        ImGui.PushStyleColor(ImGuiCol.PopupBg, ColorTooltipBg);
        
        ImGui.BeginTooltip();
        ImGui.TextColored(ColorTitle, $"💡 {tooltip.Title}");
        ImGui.Separator();
        ImGui.PushTextWrapPos(300f);
        ImGui.TextColored(ColorMessage, tooltip.Description);
        ImGui.PopTextWrapPos();
        ImGui.EndTooltip();
        
        ImGui.PopStyleColor();
        ImGui.PopStyleVar();
    }
}

/// <summary>
/// Represents a contextual tooltip that can be shown during tutorials
/// </summary>
public class ContextualTooltip
{
    public string ElementId { get; }
    public string Title { get; }
    public string Description { get; }
    
    public ContextualTooltip(string elementId, string title, string description)
    {
        ElementId = elementId;
        Title = title;
        Description = description;
    }
}
