using System.Numerics;

namespace AvorionLike.Core.Configuration;

/// <summary>
/// Configuration settings for creating a new game with extensive customization options
/// </summary>
public class NewGameSettings
{
    // Galaxy Generation Settings
    public int GalaxySeed { get; set; } = -1; // -1 means random
    public int GalaxyRadius { get; set; } = 500; // Sectors from center
    public float GalaxyDensity { get; set; } = 1.0f; // 0.5 = sparse, 1.0 = normal, 2.0 = dense
    public int TotalSectors { get; set; } = 10000; // Approximate number of populated sectors
    
    // Sector Generation Settings
    public int AsteroidsPerBelt { get; set; } = 50; // Average asteroids per asteroid belt
    public float AsteroidResourceRichness { get; set; } = 1.0f; // Resource multiplier
    public float AsteroidSizeVariation { get; set; } = 1.0f; // Size variation multiplier
    public int MinAsteroidsPerSector { get; set; } = 5;
    public int MaxAsteroidsPerSector { get; set; } = 20;
    
    // Starting Region Settings
    public Vector3 StartingSector { get; set; } = new Vector3(400, 0, 0); // Galaxy rim by default
    public string StartingRegionType { get; set; } = "Rim"; // Rim, Mid, Core
    public string StartingMaterialTier { get; set; } = "Iron"; // Iron, Titanium, Naonite, Trinium, Xanion, Ogonite, Avorion
    
    // Faction Settings
    public int FactionCount { get; set; } = 10; // Number of AI factions
    public bool EnablePlayerFaction { get; set; } = true;
    public string[] StartingFactionRelations { get; set; } = { "Neutral" }; // Friendly, Neutral, Hostile
    public float FactionWarFrequency { get; set; } = 1.0f; // How often factions go to war
    public bool EnablePirates { get; set; } = true;
    public int PirateAggression { get; set; } = 1; // 0 = Low, 1 = Normal, 2 = High
    
    // AI Settings
    public int AIDifficulty { get; set; } = 1; // 0 = Easy, 1 = Normal, 2 = Hard, 3 = Very Hard
    public float AICompetence { get; set; } = 1.0f; // 0.5 = poor decisions, 1.0 = normal, 2.0 = very smart
    public float AIEconomicCheat { get; set; } = 1.0f; // Resource generation multiplier for AI
    public bool EnableAIExpansion { get; set; } = true;
    public bool EnableAITrading { get; set; } = true;
    public bool EnableAIMining { get; set; } = true;
    public float AIReactionSpeed { get; set; } = 1.0f; // How quickly AI responds to threats
    
    // Economy Settings
    public float EconomyScale { get; set; } = 1.0f; // Price multiplier
    public float TradeProfit { get; set; } = 1.0f; // Trade profit multiplier
    public bool DynamicEconomy { get; set; } = true;
    public float ResourceScarcity { get; set; } = 1.0f; // Resource availability
    
    // Game Difficulty Settings
    public int PlayerDifficulty { get; set; } = 1; // 0 = Easy, 1 = Normal, 2 = Hard
    public float EnemyStrengthMultiplier { get; set; } = 1.0f;
    public float PlayerDamageMultiplier { get; set; } = 1.0f;
    public float ResourceGatheringMultiplier { get; set; } = 1.0f;
    public bool PermaDeath { get; set; } = false;
    public bool IronmanMode { get; set; } = false; // Only one save, can't reload
    
    // World Features
    public bool EnableMassiveAsteroids { get; set; } = true;
    public bool EnableSpaceStations { get; set; } = true;
    public bool EnableAnomalies { get; set; } = true;
    public bool EnableWormholes { get; set; } = true;
    public float EventFrequency { get; set; } = 1.0f; // Random event frequency
    
    // Player Starting Conditions
    public string PlayerName { get; set; } = "Commander";
    public int StartingCredits { get; set; } = 10000;
    public string StartingShipClass { get; set; } = "Starter"; // Starter, Fighter, Miner, Trader
    public Dictionary<string, int> StartingResources { get; set; } = new();
    
    /// <summary>
    /// Create settings with default values
    /// </summary>
    public NewGameSettings()
    {
        // Initialize starting resources
        StartingResources["Iron"] = 500;
        StartingResources["Titanium"] = 200;
    }
    
    /// <summary>
    /// Get a descriptive summary of the settings
    /// </summary>
    public string GetSummary()
    {
        return $@"
=== New Game Settings Summary ===
Galaxy: Radius {GalaxyRadius} sectors, {TotalSectors} populated sectors
Density: {GalaxyDensity:F1}x
Starting Region: {StartingRegionType} ({StartingMaterialTier} tier)
Factions: {FactionCount} AI factions, Pirates: {(EnablePirates ? "Enabled" : "Disabled")}
AI Difficulty: {GetDifficultyName(AIDifficulty)} (Competence: {AICompetence:F1}x)
Asteroids: {AsteroidsPerBelt} per belt, Richness: {AsteroidResourceRichness:F1}x
Player Difficulty: {GetDifficultyName(PlayerDifficulty)}
Starting Credits: {StartingCredits:N0}
Special Features: {(EnableMassiveAsteroids ? "✓" : "✗")} Massive Asteroids, {(EnableAnomalies ? "✓" : "✗")} Anomalies
";
    }
    
    private static string GetDifficultyName(int difficulty)
    {
        return difficulty switch
        {
            0 => "Easy",
            1 => "Normal",
            2 => "Hard",
            3 => "Very Hard",
            _ => "Unknown"
        };
    }
    
    /// <summary>
    /// Create a preset configuration
    /// </summary>
    public static NewGameSettings CreatePreset(string presetName)
    {
        var settings = new NewGameSettings();
        
        switch (presetName.ToLower())
        {
            case "easy":
                settings.PlayerDifficulty = 0;
                settings.AIDifficulty = 0;
                settings.AICompetence = 0.7f;
                settings.StartingCredits = 50000;
                settings.ResourceGatheringMultiplier = 2.0f;
                settings.PlayerDamageMultiplier = 1.5f;
                settings.EnemyStrengthMultiplier = 0.7f;
                break;
                
            case "normal":
                // Default values
                break;
                
            case "hard":
                settings.PlayerDifficulty = 2;
                settings.AIDifficulty = 2;
                settings.AICompetence = 1.3f;
                settings.StartingCredits = 5000;
                settings.ResourceGatheringMultiplier = 0.7f;
                settings.PlayerDamageMultiplier = 0.8f;
                settings.EnemyStrengthMultiplier = 1.3f;
                settings.AsteroidsPerBelt = 30;
                settings.AsteroidResourceRichness = 0.7f;
                break;
                
            case "ironman":
                settings.PlayerDifficulty = 2;
                settings.AIDifficulty = 3;
                settings.IronmanMode = true;
                settings.AICompetence = 1.5f;
                settings.StartingCredits = 5000;
                settings.ResourceGatheringMultiplier = 0.6f;
                settings.EnemyStrengthMultiplier = 1.5f;
                break;
                
            case "sandbox":
                settings.PlayerDifficulty = 0;
                settings.AIDifficulty = 0;
                settings.StartingCredits = 1000000;
                settings.ResourceGatheringMultiplier = 5.0f;
                settings.AsteroidsPerBelt = 100;
                settings.AsteroidResourceRichness = 3.0f;
                settings.EnableAIExpansion = false;
                settings.PirateAggression = 0;
                break;
                
            case "dense":
                settings.GalaxyDensity = 2.0f;
                settings.TotalSectors = 20000;
                settings.AsteroidsPerBelt = 100;
                settings.MaxAsteroidsPerSector = 40;
                settings.FactionCount = 20;
                break;
                
            case "sparse":
                settings.GalaxyDensity = 0.5f;
                settings.TotalSectors = 5000;
                settings.AsteroidsPerBelt = 25;
                settings.MaxAsteroidsPerSector = 10;
                settings.FactionCount = 5;
                break;
        }
        
        return settings;
    }
    
    /// <summary>
    /// Validate settings and apply constraints
    /// </summary>
    public void Validate()
    {
        // Clamp values to valid ranges
        GalaxyRadius = Math.Clamp(GalaxyRadius, 100, 1000);
        GalaxyDensity = Math.Clamp(GalaxyDensity, 0.1f, 5.0f);
        TotalSectors = Math.Clamp(TotalSectors, 1000, 100000);
        
        AsteroidsPerBelt = Math.Clamp(AsteroidsPerBelt, 10, 200);
        AsteroidResourceRichness = Math.Clamp(AsteroidResourceRichness, 0.1f, 10.0f);
        MinAsteroidsPerSector = Math.Clamp(MinAsteroidsPerSector, 0, 50);
        MaxAsteroidsPerSector = Math.Clamp(MaxAsteroidsPerSector, MinAsteroidsPerSector, 200);
        
        FactionCount = Math.Clamp(FactionCount, 1, 50);
        AIDifficulty = Math.Clamp(AIDifficulty, 0, 3);
        AICompetence = Math.Clamp(AICompetence, 0.1f, 5.0f);
        AIEconomicCheat = Math.Clamp(AIEconomicCheat, 0.1f, 10.0f);
        
        PlayerDifficulty = Math.Clamp(PlayerDifficulty, 0, 2);
        StartingCredits = Math.Clamp(StartingCredits, 0, 10000000);
        
        // If using random seed, generate one
        if (GalaxySeed == -1)
        {
            GalaxySeed = Environment.TickCount;
        }
    }
}
