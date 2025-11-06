namespace AvorionLike.Core.SolarSystem;

/// <summary>
/// Manages gameplay tips displayed during hyperspace loading screens
/// </summary>
public class LoadingTipManager
{
    private static LoadingTipManager? _instance;
    public static LoadingTipManager Instance => _instance ??= new LoadingTipManager();

    private readonly List<string> _generalTips = new();
    private readonly List<string> _combatTips = new();
    private readonly List<string> _buildingTips = new();
    private readonly List<string> _economyTips = new();
    private readonly List<string> _explorationTips = new();
    private readonly List<string> _factionTips = new();
    private readonly Random _random = new();

    private LoadingTipManager()
    {
        InitializeTips();
    }

    /// <summary>
    /// Initialize all loading tips
    /// </summary>
    private void InitializeTips()
    {
        // General Tips
        _generalTips.AddRange(new[]
        {
            "Press the backtick key (`) to open the debug console and access advanced commands.",
            "You can save your game at any time from the main menu or by pressing F5 for quick save.",
            "Use WASD to move your camera and mouse to look around in 3D view mode.",
            "Different materials provide different benefits - experiment with ship designs!",
            "The game features both single-player and multiplayer modes.",
            "Your ship's design affects its performance - balance offense, defense, and utility.",
            "Explore the galaxy to find rare resources and ancient technology.",
            "Check your faction approval ratings regularly to avoid rebellions.",
            "Time passes differently at strategic view - use it to manage your empire efficiently."
        });

        // Combat Tips
        _combatTips.AddRange(new[]
        {
            "Larger ships are more powerful but less maneuverable - choose wisely for each situation.",
            "Target enemy subsystems to disable specific capabilities without destroying the entire ship.",
            "Use cover from asteroids during combat to reduce incoming damage.",
            "Shield generators require power - manage your power distribution during combat.",
            "Weapons have different ranges and damage types - match them to your enemy's weaknesses.",
            "Dodging and strafing can help avoid enemy fire, especially from larger ships.",
            "Your ship's center of mass affects its rotation - design accordingly.",
            "Flanking maneuvers can expose weak points in enemy armor.",
            "Energy weapons are effective against shields, while kinetic weapons damage armor better."
        });

        // Building Tips
        _buildingTips.AddRange(new[]
        {
            "Voxel blocks can be stretched to create more efficient ship designs.",
            "Symmetry tools help create balanced ship designs quickly.",
            "Internal spaces can house systems while keeping the exterior armored.",
            "Lighter materials make your ship faster but provide less protection.",
            "Power system blocks need to be connected to function properly.",
            "Redundant systems improve ship survivability when taking damage.",
            "Aesthetic designs can be just as functional - let your creativity shine!",
            "Station modules can be rearranged to optimize production efficiency.",
            "Blueprint system lets you save and reuse your favorite designs.",
            "Damage is localized - spread critical systems throughout your ship."
        });

        // Economy Tips
        _economyTips.AddRange(new[]
        {
            "Trade routes can be automated using AI ships to generate passive income.",
            "Market prices fluctuate based on supply and demand - buy low, sell high!",
            "Building production stations creates a steady income stream.",
            "Resource scarcity in different systems creates trading opportunities.",
            "Credits can be earned through mining, trading, combat, and missions.",
            "Upgrading your cargo hold allows for more profitable trade runs.",
            "Supply chains connect multiple stations - control them for maximum profit.",
            "Some resources are only found in specific regions of the galaxy.",
            "Investing in factories early provides long-term economic benefits."
        });

        // Exploration Tips
        _explorationTips.AddRange(new[]
        {
            "Scanning anomalies can reveal valuable resources or technologies.",
            "Each solar system has unique characteristics and resources.",
            "Uncharted systems may contain hidden dangers and treasures.",
            "Map data can be sold to other factions for profit.",
            "Ancient artifacts provide research bonuses when discovered.",
            "Some sectors have environmental hazards - come prepared.",
            "Hidden stations and derelict ships can be found off the beaten path.",
            "Long-range scanners reveal system contents before jumping in.",
            "Exploration data contributes to your faction's knowledge base."
        });

        // Faction Tips
        _factionTips.AddRange(new[]
        {
            "Each faction has specific ethics that drive their demands and preferences.",
            "Meeting faction demands increases approval and generates influence.",
            "Influence is used for diplomacy, expansion, and policy changes.",
            "Unhappy factions can rebel if ignored for too long.",
            "Government type affects how factions behave and can be suppressed.",
            "Pops align with factions based on their ethics and living conditions.",
            "Policy decisions affect multiple factions - balance carefully.",
            "Dominant factions have more political power in democracies.",
            "Planet stability depends on the happiness of its population.",
            "Faction approval can shift based on events and your decisions.",
            "In authoritarian governments, you can suppress dissenting factions."
        });
    }

    /// <summary>
    /// Get a random tip from all categories
    /// </summary>
    public string GetRandomTip()
    {
        var allTips = _generalTips
            .Concat(_combatTips)
            .Concat(_buildingTips)
            .Concat(_economyTips)
            .Concat(_explorationTips)
            .Concat(_factionTips)
            .ToList();

        return allTips[_random.Next(allTips.Count)];
    }

    /// <summary>
    /// Get a tip from a specific category
    /// </summary>
    public string GetTipByCategory(TipCategory category)
    {
        var tips = category switch
        {
            TipCategory.General => _generalTips,
            TipCategory.Combat => _combatTips,
            TipCategory.Building => _buildingTips,
            TipCategory.Economy => _economyTips,
            TipCategory.Exploration => _explorationTips,
            TipCategory.Faction => _factionTips,
            _ => _generalTips
        };

        return tips[_random.Next(tips.Count)];
    }

    /// <summary>
    /// Add a custom tip to a category
    /// </summary>
    public void AddCustomTip(string tip, TipCategory category)
    {
        var tips = category switch
        {
            TipCategory.General => _generalTips,
            TipCategory.Combat => _combatTips,
            TipCategory.Building => _buildingTips,
            TipCategory.Economy => _economyTips,
            TipCategory.Exploration => _explorationTips,
            TipCategory.Faction => _factionTips,
            _ => _generalTips
        };

        tips.Add(tip);
    }

    /// <summary>
    /// Get multiple tips for longer loading screens
    /// </summary>
    public List<string> GetMultipleTips(int count)
    {
        var tips = new List<string>();
        for (int i = 0; i < count; i++)
        {
            tips.Add(GetRandomTip());
        }
        return tips;
    }
}

/// <summary>
/// Categories for loading tips
/// </summary>
public enum TipCategory
{
    General,
    Combat,
    Building,
    Economy,
    Exploration,
    Faction
}
