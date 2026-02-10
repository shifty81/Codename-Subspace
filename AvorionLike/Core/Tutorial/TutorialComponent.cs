using AvorionLike.Core.ECS;
using AvorionLike.Core.Logging;
using AvorionLike.Core.Persistence;
using System.Text.Json;

namespace AvorionLike.Core.Tutorial;

/// <summary>
/// Component that tracks tutorial progress for an entity (typically the player).
/// Persists active tutorials and completed tutorial IDs across save/load.
/// </summary>
public class TutorialComponent : IComponent, ISerializable
{
    public Guid EntityId { get; set; }

    /// <summary>
    /// Active tutorials for this entity
    /// </summary>
    public List<Tutorial> ActiveTutorials { get; set; } = new();

    /// <summary>
    /// IDs of completed tutorials
    /// </summary>
    public HashSet<string> CompletedTutorialIds { get; set; } = new();

    /// <summary>
    /// Serialize the component to a dictionary
    /// </summary>
    public Dictionary<string, object> Serialize()
    {
        var activeTutorialsData = new List<Dictionary<string, object>>();

        foreach (var tutorial in ActiveTutorials)
        {
            activeTutorialsData.Add(SerializeTutorial(tutorial));
        }

        return new Dictionary<string, object>
        {
            ["EntityId"] = EntityId.ToString(),
            ["ActiveTutorials"] = activeTutorialsData,
            ["CompletedTutorialIds"] = new List<string>(CompletedTutorialIds)
        };
    }

    /// <summary>
    /// Deserialize the component from a dictionary
    /// </summary>
    public void Deserialize(Dictionary<string, object> data)
    {
        EntityId = Guid.Parse(SerializationHelper.GetValue(data, "EntityId", Guid.Empty.ToString()));

        ActiveTutorials.Clear();
        CompletedTutorialIds.Clear();

        // Deserialize completed tutorial IDs
        CompletedTutorialIds = new HashSet<string>(DeserializeStringList(data, "CompletedTutorialIds"));

        // Deserialize active tutorials
        if (!data.ContainsKey("ActiveTutorials"))
            return;

        foreach (var tutorialData in DeserializeDictList(data, "ActiveTutorials"))
        {
            var tutorial = DeserializeTutorial(tutorialData);
            if (tutorial != null)
            {
                ActiveTutorials.Add(tutorial);
            }
        }
    }

    private static Dictionary<string, object> SerializeTutorial(Tutorial tutorial)
    {
        var stepsData = new List<Dictionary<string, object>>();
        foreach (var step in tutorial.Steps)
        {
            stepsData.Add(new Dictionary<string, object>
            {
                ["Id"] = step.Id,
                ["Type"] = step.Type.ToString(),
                ["Status"] = step.Status.ToString(),
                ["Title"] = step.Title,
                ["Message"] = step.Message,
                ["RequiredKey"] = step.RequiredKey ?? string.Empty,
                ["RequiredAction"] = step.RequiredAction ?? string.Empty,
                ["UIElementId"] = step.UIElementId ?? string.Empty,
                ["Duration"] = step.Duration,
                ["CanSkip"] = step.CanSkip,
                ["StartTime"] = step.StartTime?.ToString("o") ?? string.Empty
            });
        }

        return new Dictionary<string, object>
        {
            ["Id"] = tutorial.Id,
            ["Title"] = tutorial.Title,
            ["Description"] = tutorial.Description,
            ["Status"] = tutorial.Status.ToString(),
            ["CurrentStepIndex"] = tutorial.CurrentStepIndex,
            ["AutoStart"] = tutorial.AutoStart,
            ["Prerequisites"] = tutorial.Prerequisites,
            ["Steps"] = stepsData,
            ["StartTime"] = tutorial.StartTime?.ToString("o") ?? string.Empty,
            ["CompletedTime"] = tutorial.CompletedTime?.ToString("o") ?? string.Empty
        };
    }

    private static Tutorial? DeserializeTutorial(Dictionary<string, object> data)
    {
        try
        {
            var tutorial = new Tutorial
            {
                Id = SerializationHelper.GetValue(data, "Id", string.Empty),
                Title = SerializationHelper.GetValue(data, "Title", string.Empty),
                Description = SerializationHelper.GetValue(data, "Description", string.Empty),
                CurrentStepIndex = SerializationHelper.GetValue(data, "CurrentStepIndex", 0),
                AutoStart = SerializationHelper.GetValue(data, "AutoStart", false)
            };

            if (Enum.TryParse<TutorialStatus>(SerializationHelper.GetValue(data, "Status", "NotStarted"), out var status))
                tutorial.Status = status;

            var startTimeStr = SerializationHelper.GetValue(data, "StartTime", string.Empty);
            if (!string.IsNullOrEmpty(startTimeStr) && DateTime.TryParse(startTimeStr, out var startTime))
                tutorial.StartTime = startTime;

            var completedTimeStr = SerializationHelper.GetValue(data, "CompletedTime", string.Empty);
            if (!string.IsNullOrEmpty(completedTimeStr) && DateTime.TryParse(completedTimeStr, out var completedTime))
                tutorial.CompletedTime = completedTime;

            tutorial.Prerequisites = DeserializeStringList(data, "Prerequisites");

            // Deserialize steps
            foreach (var stepData in DeserializeDictList(data, "Steps"))
            {
                var step = new TutorialStep
                {
                    Id = SerializationHelper.GetValue(stepData, "Id", string.Empty),
                    Title = SerializationHelper.GetValue(stepData, "Title", string.Empty),
                    Message = SerializationHelper.GetValue(stepData, "Message", string.Empty),
                    Duration = SerializationHelper.GetValue(stepData, "Duration", 0f),
                    CanSkip = SerializationHelper.GetValue(stepData, "CanSkip", true)
                };

                var requiredKey = SerializationHelper.GetValue(stepData, "RequiredKey", string.Empty);
                step.RequiredKey = string.IsNullOrEmpty(requiredKey) ? null : requiredKey;

                var requiredAction = SerializationHelper.GetValue(stepData, "RequiredAction", string.Empty);
                step.RequiredAction = string.IsNullOrEmpty(requiredAction) ? null : requiredAction;

                var uiElementId = SerializationHelper.GetValue(stepData, "UIElementId", string.Empty);
                step.UIElementId = string.IsNullOrEmpty(uiElementId) ? null : uiElementId;

                if (Enum.TryParse<TutorialStepType>(SerializationHelper.GetValue(stepData, "Type", "Message"), out var stepType))
                    step.Type = stepType;

                if (Enum.TryParse<TutorialStepStatus>(SerializationHelper.GetValue(stepData, "Status", "NotStarted"), out var stepStatus))
                    step.Status = stepStatus;

                var stepStartTimeStr = SerializationHelper.GetValue(stepData, "StartTime", string.Empty);
                if (!string.IsNullOrEmpty(stepStartTimeStr) && DateTime.TryParse(stepStartTimeStr, out var stepStartTime))
                    step.StartTime = stepStartTime;

                tutorial.Steps.Add(step);
            }

            return tutorial;
        }
        catch (Exception ex)
        {
            Logger.Instance.Warning("TutorialComponent", $"Failed to deserialize tutorial: {ex.Message}");
            return null;
        }
    }

    private static List<Dictionary<string, object>> DeserializeDictList(Dictionary<string, object> data, string key)
    {
        if (!data.ContainsKey(key))
            return new List<Dictionary<string, object>>();

        if (data[key] is JsonElement element)
        {
            return JsonSerializer.Deserialize<List<Dictionary<string, object>>>(element.GetRawText())
                ?? new List<Dictionary<string, object>>();
        }

        return data[key] as List<Dictionary<string, object>> ?? new List<Dictionary<string, object>>();
    }

    private static List<string> DeserializeStringList(Dictionary<string, object> data, string key)
    {
        if (!data.ContainsKey(key))
            return new List<string>();

        if (data[key] is JsonElement element)
        {
            return JsonSerializer.Deserialize<List<string>>(element.GetRawText()) ?? new List<string>();
        }

        return data[key] as List<string> ?? new List<string>();
    }
}
