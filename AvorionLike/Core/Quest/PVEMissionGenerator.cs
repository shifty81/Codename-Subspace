using AvorionLike.Core.Faction;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Quest;

/// <summary>
/// Procedurally generates PVE missions based on faction, difficulty, and player progression.
/// Implements the PVE loop:
///   Start → Player begins at Newcomer Station → NPC agent gives task →
///   Destroy NPC pirate fleet → Stronger NPCs drop upgrade components → Repeat.
/// </summary>
public class PVEMissionGenerator
{
    private readonly Random _random;
    private readonly Logger _logger = Logger.Instance;

    public PVEMissionGenerator(int seed = 0)
    {
        _random = seed == 0 ? new Random() : new Random(seed);
    }

    /// <summary>
    /// Generate a starter mission for a new player character.
    /// This is the first mission given by an NPC agent at the newcomer station.
    /// </summary>
    public Quest GenerateStarterMission(EVEFactionId playerFaction)
    {
        var profile = EVEFactionDefinitions.GetProfile(playerFaction);
        var enemyFaction = PickEnemyFaction(playerFaction);
        var enemyProfile = EVEFactionDefinitions.GetProfile(enemyFaction);

        var quest = new Quest
        {
            Id = Guid.NewGuid().ToString(),
            Title = $"Newcomer Trial: Clear the {enemyProfile.Name} Patrol",
            Description = $"Welcome to {profile.StarterStation}, pilot. We've detected a small " +
                          $"{enemyProfile.Name} patrol near our station. Take your starter vessel " +
                          $"and eliminate the threat. This will prove your worth to {profile.Name}.",
            Difficulty = QuestDifficulty.Trivial,
            QuestGiverLocation = profile.StarterStation,
            CanAbandon = false,
            IsRepeatable = false,
            Tags = new List<string> { "starter", "combat", "pve", playerFaction.ToString() }
        };

        quest.Objectives.Add(new QuestObjective
        {
            Id = Guid.NewGuid().ToString(),
            Type = ObjectiveType.Destroy,
            Description = $"Destroy {enemyProfile.Name} patrol ships",
            Target = $"{enemyProfile.Name}_patrol",
            RequiredQuantity = 3,
            CurrentProgress = 0
        });

        quest.Rewards.Add(new QuestReward
        {
            Type = RewardType.Credits,
            RewardId = "Credits",
            Amount = 5000,
            Description = "5,000 Credits"
        });
        quest.Rewards.Add(new QuestReward
        {
            Type = RewardType.Experience,
            RewardId = "XP",
            Amount = 200,
            Description = "200 Experience Points"
        });
        quest.Rewards.Add(new QuestReward
        {
            Type = RewardType.Reputation,
            RewardId = profile.Name,
            Amount = 100,
            Description = $"+100 {profile.Name} Reputation"
        });

        _logger.Info("PVEMissionGenerator",
            $"Generated starter mission for {profile.Name}: '{quest.Title}'");

        return quest;
    }

    /// <summary>
    /// Generate a PVE combat mission scaled to the given difficulty
    /// </summary>
    public Quest GenerateCombatMission(EVEFactionId playerFaction, QuestDifficulty difficulty)
    {
        var profile = EVEFactionDefinitions.GetProfile(playerFaction);
        var enemyFaction = PickEnemyFaction(playerFaction);
        var enemyProfile = EVEFactionDefinitions.GetProfile(enemyFaction);

        int enemyCount = GetEnemyCount(difficulty);
        int creditReward = GetCreditReward(difficulty);
        int xpReward = GetXPReward(difficulty);

        var missionVariant = _random.Next(3);
        var (title, description, target) = missionVariant switch
        {
            0 => ($"Clear a {enemyProfile.Name} Patrol",
                  $"A {enemyProfile.Name} patrol has been spotted in the sector. " +
                  $"Destroy all {enemyCount} enemy ships to secure the area.",
                  $"{enemyProfile.Name}_patrol"),
            1 => ($"Destroy {enemyProfile.Name} Mining Rig",
                  $"The {enemyProfile.Name} has established an illegal mining operation nearby. " +
                  $"Eliminate the mining escorts and the rig itself.",
                  $"{enemyProfile.Name}_mining_rig"),
            _ => ($"Defend Against {enemyProfile.Name} Raiders",
                  $"Intelligence reports an imminent {enemyProfile.Name} raid on our supply lines. " +
                  $"Intercept and destroy the raiding party before they reach the convoy.",
                  $"{enemyProfile.Name}_raiders")
        };

        var quest = new Quest
        {
            Id = Guid.NewGuid().ToString(),
            Title = title,
            Description = description,
            Difficulty = difficulty,
            QuestGiverLocation = profile.StarterStation,
            Tags = new List<string> { "combat", "pve", difficulty.ToString().ToLower(), playerFaction.ToString() }
        };

        quest.Objectives.Add(new QuestObjective
        {
            Id = Guid.NewGuid().ToString(),
            Type = ObjectiveType.Destroy,
            Description = $"Destroy enemy ships",
            Target = target,
            RequiredQuantity = enemyCount,
            CurrentProgress = 0
        });

        quest.Rewards.Add(new QuestReward
        {
            Type = RewardType.Credits,
            RewardId = "Credits",
            Amount = creditReward,
            Description = $"{creditReward:N0} Credits"
        });
        quest.Rewards.Add(new QuestReward
        {
            Type = RewardType.Experience,
            RewardId = "XP",
            Amount = xpReward,
            Description = $"{xpReward:N0} Experience Points"
        });

        // Higher difficulty missions drop upgrade components
        if (difficulty >= QuestDifficulty.Normal)
        {
            quest.Rewards.Add(new QuestReward
            {
                Type = RewardType.Item,
                RewardId = GetUpgradeComponent(difficulty),
                Amount = 1,
                Description = $"Ship Upgrade Component ({difficulty})"
            });
        }

        _logger.Info("PVEMissionGenerator",
            $"Generated {difficulty} combat mission: '{quest.Title}'");

        return quest;
    }

    /// <summary>
    /// Generate a mining/collection PVE mission
    /// </summary>
    public Quest GenerateMiningMission(EVEFactionId playerFaction, QuestDifficulty difficulty)
    {
        var profile = EVEFactionDefinitions.GetProfile(playerFaction);
        int resourceAmount = GetResourceAmount(difficulty);
        int creditReward = GetCreditReward(difficulty);
        string resourceName = PickResourceForDifficulty(difficulty);

        var quest = new Quest
        {
            Id = Guid.NewGuid().ToString(),
            Title = $"Resource Requisition: {resourceName}",
            Description = $"The {profile.Name} requires {resourceAmount} units of {resourceName}. " +
                          $"Mine the resource from nearby asteroids and deliver it to the station.",
            Difficulty = difficulty,
            QuestGiverLocation = profile.StarterStation,
            Tags = new List<string> { "mining", "pve", difficulty.ToString().ToLower(), playerFaction.ToString() }
        };

        quest.Objectives.Add(new QuestObjective
        {
            Id = Guid.NewGuid().ToString(),
            Type = ObjectiveType.Mine,
            Description = $"Mine {resourceAmount} {resourceName}",
            Target = resourceName,
            RequiredQuantity = resourceAmount,
            CurrentProgress = 0
        });

        quest.Rewards.Add(new QuestReward
        {
            Type = RewardType.Credits,
            RewardId = "Credits",
            Amount = creditReward,
            Description = $"{creditReward:N0} Credits"
        });
        quest.Rewards.Add(new QuestReward
        {
            Type = RewardType.Experience,
            RewardId = "XP",
            Amount = GetXPReward(difficulty) / 2,
            Description = $"{GetXPReward(difficulty) / 2:N0} Experience Points"
        });

        return quest;
    }

    /// <summary>
    /// Generate a batch of missions for a station NPC agent
    /// </summary>
    public List<Quest> GenerateMissionBoard(EVEFactionId playerFaction, int count = 5)
    {
        var missions = new List<Quest>();
        var difficulties = new[] { QuestDifficulty.Easy, QuestDifficulty.Normal, QuestDifficulty.Hard };

        for (int i = 0; i < count; i++)
        {
            var difficulty = difficulties[_random.Next(difficulties.Length)];
            var missionType = _random.Next(3);

            Quest quest = missionType switch
            {
                0 => GenerateCombatMission(playerFaction, difficulty),
                1 => GenerateMiningMission(playerFaction, difficulty),
                _ => GenerateCombatMission(playerFaction, difficulty)
            };

            missions.Add(quest);
        }

        _logger.Info("PVEMissionGenerator",
            $"Generated mission board with {missions.Count} missions for {playerFaction}");

        return missions;
    }

    // ========== Private helpers ==========

    private EVEFactionId PickEnemyFaction(EVEFactionId playerFaction)
    {
        // Each faction has a primary rival
        return playerFaction switch
        {
            EVEFactionId.SanctumHegemony => EVEFactionId.RustScrapCoalition,
            EVEFactionId.CoreNexus => EVEFactionId.VanguardRepublic,
            EVEFactionId.VanguardRepublic => EVEFactionId.CoreNexus,
            EVEFactionId.RustScrapCoalition => EVEFactionId.SanctumHegemony,
            _ => Enum.GetValues<EVEFactionId>()[_random.Next(Enum.GetValues<EVEFactionId>().Length)]
        };
    }

    private int GetEnemyCount(QuestDifficulty difficulty)
    {
        return difficulty switch
        {
            QuestDifficulty.Trivial => 3,
            QuestDifficulty.Easy => 5,
            QuestDifficulty.Normal => 8,
            QuestDifficulty.Hard => 12,
            QuestDifficulty.Elite => 20,
            _ => 5
        };
    }

    private int GetCreditReward(QuestDifficulty difficulty)
    {
        return difficulty switch
        {
            QuestDifficulty.Trivial => 5000,
            QuestDifficulty.Easy => 10000,
            QuestDifficulty.Normal => 25000,
            QuestDifficulty.Hard => 50000,
            QuestDifficulty.Elite => 100000,
            _ => 10000
        };
    }

    private int GetXPReward(QuestDifficulty difficulty)
    {
        return difficulty switch
        {
            QuestDifficulty.Trivial => 200,
            QuestDifficulty.Easy => 500,
            QuestDifficulty.Normal => 1000,
            QuestDifficulty.Hard => 2500,
            QuestDifficulty.Elite => 5000,
            _ => 500
        };
    }

    private int GetResourceAmount(QuestDifficulty difficulty)
    {
        return difficulty switch
        {
            QuestDifficulty.Trivial => 50,
            QuestDifficulty.Easy => 100,
            QuestDifficulty.Normal => 250,
            QuestDifficulty.Hard => 500,
            QuestDifficulty.Elite => 1000,
            _ => 100
        };
    }

    private string PickResourceForDifficulty(QuestDifficulty difficulty)
    {
        return difficulty switch
        {
            QuestDifficulty.Trivial => "Iron",
            QuestDifficulty.Easy => "Titanium",
            QuestDifficulty.Normal => "Naonite",
            QuestDifficulty.Hard => "Trinium",
            QuestDifficulty.Elite => "Xanion",
            _ => "Iron"
        };
    }

    private string GetUpgradeComponent(QuestDifficulty difficulty)
    {
        return difficulty switch
        {
            QuestDifficulty.Normal => "basic_shield_upgrade",
            QuestDifficulty.Hard => "advanced_weapon_module",
            QuestDifficulty.Elite => "prototype_engine_core",
            _ => "generic_component"
        };
    }
}
