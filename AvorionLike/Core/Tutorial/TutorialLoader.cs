using System.Text.Json;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Tutorial;

/// <summary>
/// Loads tutorial definitions from JSON files
/// </summary>
public class TutorialLoader
{
    private static readonly JsonSerializerOptions JsonOptions = new()
    {
        WriteIndented = true,
        PropertyNameCaseInsensitive = true
    };
    
    /// <summary>
    /// Load a tutorial from a JSON file
    /// </summary>
    /// <param name="filePath">Path to the JSON file</param>
    /// <returns>Loaded tutorial, or null if failed</returns>
    public static Tutorial? LoadTutorialFromFile(string filePath)
    {
        try
        {
            if (!File.Exists(filePath))
            {
                Logger.Instance.Warning("TutorialLoader", $"Tutorial file not found: {filePath}");
                return null;
            }
            
            string json = File.ReadAllText(filePath);
            var tutorial = JsonSerializer.Deserialize<Tutorial>(json, JsonOptions);
            
            if (tutorial != null)
            {
                Logger.Instance.Info("TutorialLoader", $"Loaded tutorial '{tutorial.Title}' from {filePath}");
            }
            
            return tutorial;
        }
        catch (Exception ex)
        {
            Logger.Instance.Error("TutorialLoader", $"Failed to load tutorial from {filePath}: {ex.Message}", ex);
            return null;
        }
    }
    
    /// <summary>
    /// Load all tutorials from a directory
    /// </summary>
    /// <param name="directoryPath">Path to directory containing tutorial JSON files</param>
    /// <returns>List of loaded tutorials</returns>
    public static List<Tutorial> LoadTutorialsFromDirectory(string directoryPath)
    {
        var tutorials = new List<Tutorial>();
        
        try
        {
            if (!Directory.Exists(directoryPath))
            {
                Logger.Instance.Warning("TutorialLoader", $"Tutorial directory not found: {directoryPath}");
                return tutorials;
            }
            
            var jsonFiles = Directory.GetFiles(directoryPath, "*.json", SearchOption.AllDirectories);
            Logger.Instance.Info("TutorialLoader", $"Found {jsonFiles.Length} tutorial files in {directoryPath}");
            
            foreach (var file in jsonFiles)
            {
                var tutorial = LoadTutorialFromFile(file);
                if (tutorial != null)
                {
                    tutorials.Add(tutorial);
                }
            }
            
            Logger.Instance.Info("TutorialLoader", $"Successfully loaded {tutorials.Count} tutorials");
        }
        catch (Exception ex)
        {
            Logger.Instance.Error("TutorialLoader", $"Failed to load tutorials from directory {directoryPath}: {ex.Message}", ex);
        }
        
        return tutorials;
    }
    
    /// <summary>
    /// Save a tutorial to a JSON file
    /// </summary>
    /// <param name="tutorial">Tutorial to save</param>
    /// <param name="filePath">Path to save the JSON file</param>
    /// <returns>True if saved successfully</returns>
    public static bool SaveTutorialToFile(Tutorial tutorial, string filePath)
    {
        try
        {
            // Ensure directory exists
            var directory = Path.GetDirectoryName(filePath);
            if (!string.IsNullOrEmpty(directory) && !Directory.Exists(directory))
            {
                Directory.CreateDirectory(directory);
            }
            
            string json = JsonSerializer.Serialize(tutorial, JsonOptions);
            File.WriteAllText(filePath, json);
            
            Logger.Instance.Info("TutorialLoader", $"Saved tutorial '{tutorial.Title}' to {filePath}");
            return true;
        }
        catch (Exception ex)
        {
            Logger.Instance.Error("TutorialLoader", $"Failed to save tutorial to {filePath}: {ex.Message}", ex);
            return false;
        }
    }
    
    /// <summary>
    /// Create a sample basic controls tutorial
    /// </summary>
    /// <param name="filePath">Path to save the sample tutorial</param>
    public static void CreateSampleBasicControlsTutorial(string filePath)
    {
        var tutorial = new Tutorial
        {
            Id = "tutorial_basic_controls",
            Title = "Basic Controls",
            Description = "Learn the basic controls to navigate your ship in space.",
            AutoStart = true,
            Prerequisites = new List<string>()
        };
        
        tutorial.Steps.Add(new TutorialStep
        {
            Type = TutorialStepType.Message,
            Title = "Welcome to Codename: Subspace!",
            Message = "Welcome, Commander! This tutorial will teach you the basic controls. Click 'Continue' when ready.",
            CanSkip = true
        });
        
        tutorial.Steps.Add(new TutorialStep
        {
            Type = TutorialStepType.Message,
            Title = "Movement Controls",
            Message = "Use WASD keys to move your ship:\n• W - Forward\n• S - Backward\n• A - Strafe Left\n• D - Strafe Right\n• Space - Up\n• Shift - Down",
            CanSkip = true
        });
        
        tutorial.Steps.Add(new TutorialStep
        {
            Type = TutorialStepType.WaitForAction,
            Title = "Try Moving!",
            Message = "Now try moving your ship using the WASD keys. Move around to get a feel for the controls.",
            RequiredAction = "ship_moved",
            CanSkip = true
        });
        
        tutorial.Steps.Add(new TutorialStep
        {
            Type = TutorialStepType.Message,
            Title = "Camera Controls",
            Message = "Use the mouse to look around and the Arrow keys + Q/E to rotate your ship:\n• Mouse - Look Around\n• Arrow Keys - Rotate Ship\n• Q/E - Roll\n• C - Toggle Camera/Ship Control",
            CanSkip = true
        });
        
        tutorial.Steps.Add(new TutorialStep
        {
            Type = TutorialStepType.Message,
            Title = "User Interface",
            Message = "Important UI keys:\n• J - Quest Log\n• I - Inventory\n• B - Ship Builder\n• TAB - Player Status\n• ESC - Pause Menu\n• ~ - Testing Console",
            CanSkip = true
        });
        
        tutorial.Steps.Add(new TutorialStep
        {
            Type = TutorialStepType.Message,
            Title = "Tutorial Complete!",
            Message = "You've learned the basic controls! Now you're ready to explore the galaxy. Good luck, Commander!",
            CanSkip = false
        });
        
        SaveTutorialToFile(tutorial, filePath);
    }
    
    /// <summary>
    /// Create a sample mining tutorial
    /// </summary>
    /// <param name="filePath">Path to save the sample tutorial</param>
    public static void CreateSampleMiningTutorial(string filePath)
    {
        var tutorial = new Tutorial
        {
            Id = "tutorial_mining_basics",
            Title = "Mining Basics",
            Description = "Learn how to mine resources from asteroids to build and upgrade your ship.",
            AutoStart = false,
            Prerequisites = new List<string> { "tutorial_basic_controls" }
        };
        
        tutorial.Steps.Add(new TutorialStep
        {
            Type = TutorialStepType.Message,
            Title = "Introduction to Mining",
            Message = "Mining is essential for gathering resources. You'll need resources to build and upgrade your ship.",
            CanSkip = true
        });
        
        tutorial.Steps.Add(new TutorialStep
        {
            Type = TutorialStepType.Message,
            Title = "Finding Asteroids",
            Message = "Look for asteroids in your sector. They appear as floating rocks and contain valuable resources like Iron, Titanium, and more.",
            CanSkip = true
        });
        
        tutorial.Steps.Add(new TutorialStep
        {
            Type = TutorialStepType.Message,
            Title = "Mining Lasers",
            Message = "To mine, you need mining lasers equipped on your ship. Approach an asteroid and fire your mining laser to extract resources.",
            CanSkip = true
        });
        
        tutorial.Steps.Add(new TutorialStep
        {
            Type = TutorialStepType.WaitForAction,
            Title = "Mine Your First Resource",
            Message = "Now try mining some resources! Find an asteroid and use your mining laser. Resources will automatically be added to your inventory.",
            RequiredAction = "collect_resource",
            CanSkip = true
        });
        
        tutorial.Steps.Add(new TutorialStep
        {
            Type = TutorialStepType.Message,
            Title = "Mining Complete!",
            Message = "Great job! You've learned how to mine resources. Continue mining to gather materials for building and trading.",
            CanSkip = false
        });
        
        SaveTutorialToFile(tutorial, filePath);
    }
}
