using AvorionLike.Core.ECS;
using AvorionLike.Core.Resources;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Faction;

/// <summary>
/// Represents an item in a character's inventory
/// </summary>
public class CharacterItem
{
    public string Name { get; set; } = "";
    public string Description { get; set; } = "";
    public int Quantity { get; set; } = 1;
    public string ItemType { get; set; } = "Generic";
}

/// <summary>
/// Represents a player character with faction, bloodline, and progression data.
/// Maps to the design specification for EVE-style character creation.
/// </summary>
public class Character
{
    public string Name { get; set; } = "";
    public EVEFactionId FactionId { get; set; }
    public Bloodline BloodlineId { get; set; }
    public Education EducationPath { get; set; }
    public long SkillPoints { get; set; } = 0;
    public List<CharacterItem> Inventory { get; set; } = new();
    public Guid? CurrentShipEntityId { get; set; }
    public string CurrentStation { get; set; } = "";
    public DateTime CreatedAt { get; set; } = DateTime.UtcNow;

    /// <summary>
    /// Get the faction profile for this character
    /// </summary>
    public EVEFactionProfile GetFactionProfile()
    {
        return EVEFactionDefinitions.GetProfile(FactionId);
    }

    /// <summary>
    /// Get a display summary of the character
    /// </summary>
    public string GetSummary()
    {
        var profile = GetFactionProfile();
        return $"{Name}\n" +
               $"  Faction: {profile.Name}\n" +
               $"  Bloodline: {BloodlineId}\n" +
               $"  Education: {EducationPath}\n" +
               $"  Skill Points: {SkillPoints:N0}\n" +
               $"  Station: {CurrentStation}\n" +
               $"  Ship: {(CurrentShipEntityId.HasValue ? CurrentShipEntityId.Value.ToString()[..8] : "None")}";
    }
}

/// <summary>
/// Manages up to 3 character slots for EVE-style character selection and creation.
/// Screen Flow:
///   Screen 1: Select Screen — 3 slots, empty shows "Create New", occupied shows Name/Race.
///   Screen 2: Creation Screen — Select race, bloodline, education, name.
/// </summary>
public class CharacterManager
{
    private readonly Logger _logger = Logger.Instance;
    private readonly Character?[] _slots = new Character?[3];

    /// <summary>
    /// Character slots (read-only access)
    /// </summary>
    public IReadOnlyList<Character?> Slots => _slots;

    /// <summary>
    /// Maximum number of character slots
    /// </summary>
    public int MaxSlots => _slots.Length;

    /// <summary>
    /// Create a new character in the specified slot
    /// </summary>
    /// <param name="slotIndex">Slot index (0-2)</param>
    /// <param name="name">Character name</param>
    /// <param name="factionId">Faction ID (0-3)</param>
    /// <param name="bloodlineId">Bloodline within the faction</param>
    /// <param name="education">Education path for initial skills</param>
    /// <returns>The created character, or null if slot is invalid or occupied</returns>
    public Character? CreateCharacter(int slotIndex, string name, EVEFactionId factionId,
        Bloodline bloodlineId, Education education = Education.Engineering)
    {
        if (slotIndex < 0 || slotIndex >= _slots.Length)
        {
            _logger.Warning("CharacterManager", $"Invalid slot index: {slotIndex}");
            return null;
        }

        if (_slots[slotIndex] != null)
        {
            _logger.Warning("CharacterManager", $"Slot {slotIndex} is already occupied by '{_slots[slotIndex]!.Name}'");
            return null;
        }

        // Validate bloodline belongs to the chosen faction
        var validBloodlines = EVEFactionDefinitions.GetBloodlines(factionId);
        if (!validBloodlines.Contains(bloodlineId))
        {
            _logger.Warning("CharacterManager", $"Bloodline {bloodlineId} is not valid for faction {factionId}");
            return null;
        }

        var profile = EVEFactionDefinitions.GetProfile(factionId);

        var character = new Character
        {
            Name = name,
            FactionId = factionId,
            BloodlineId = bloodlineId,
            EducationPath = education,
            SkillPoints = GetInitialSkillPoints(education),
            CurrentStation = profile.StarterStation
        };

        // Add starter items
        character.Inventory.Add(new CharacterItem
        {
            Name = "Civilian Shield Booster",
            Description = "A basic shield booster for new pilots",
            ItemType = "Module"
        });
        character.Inventory.Add(new CharacterItem
        {
            Name = "Starter Weapon",
            Description = GetStarterWeaponDescription(factionId),
            ItemType = "Weapon"
        });

        _slots[slotIndex] = character;
        _logger.Info("CharacterManager", $"Created character '{name}' in slot {slotIndex} " +
                     $"({profile.Name}, {bloodlineId}, {education})");

        return character;
    }

    /// <summary>
    /// Load a character from a slot
    /// </summary>
    /// <param name="slotIndex">Slot index (0-2)</param>
    /// <returns>The character in the slot, or null if empty/invalid</returns>
    public Character? LoadCharacter(int slotIndex)
    {
        if (slotIndex < 0 || slotIndex >= _slots.Length)
        {
            _logger.Warning("CharacterManager", $"Invalid slot index: {slotIndex}");
            return null;
        }

        if (_slots[slotIndex] == null)
        {
            _logger.Info("CharacterManager", $"Slot {slotIndex} is empty");
            return null;
        }

        _logger.Info("CharacterManager", $"Loaded character '{_slots[slotIndex]!.Name}' from slot {slotIndex}");
        return _slots[slotIndex];
    }

    /// <summary>
    /// Delete a character from a slot
    /// </summary>
    public bool DeleteCharacter(int slotIndex)
    {
        if (slotIndex < 0 || slotIndex >= _slots.Length)
            return false;

        if (_slots[slotIndex] == null)
            return false;

        _logger.Info("CharacterManager", $"Deleted character '{_slots[slotIndex]!.Name}' from slot {slotIndex}");
        _slots[slotIndex] = null;
        return true;
    }

    /// <summary>
    /// Check if a slot is empty (for "Create New" display)
    /// </summary>
    public bool IsSlotEmpty(int slotIndex)
    {
        if (slotIndex < 0 || slotIndex >= _slots.Length)
            return false;

        return _slots[slotIndex] == null;
    }

    /// <summary>
    /// Get the display text for a character slot (for selection screen UI)
    /// </summary>
    public string GetSlotDisplayText(int slotIndex)
    {
        if (slotIndex < 0 || slotIndex >= _slots.Length)
            return "Invalid Slot";

        var character = _slots[slotIndex];
        if (character == null)
            return "Create New";

        var profile = EVEFactionDefinitions.GetProfile(character.FactionId);
        return $"{character.Name} - {profile.Name}";
    }

    /// <summary>
    /// Get initial skill points based on education path
    /// </summary>
    private long GetInitialSkillPoints(Education education)
    {
        return education switch
        {
            Education.Engineering => 5000,
            Education.Gunnery => 5000,
            Education.Navigation => 4500,
            Education.Drones => 4500,
            Education.Electronics => 4000,
            Education.MissileOperations => 5000,
            _ => 4000
        };
    }

    /// <summary>
    /// Get starter weapon description based on faction
    /// </summary>
    private string GetStarterWeaponDescription(EVEFactionId factionId)
    {
        return factionId switch
        {
            EVEFactionId.SanctumHegemony => "Civilian Energy Turret - basic laser weapon",
            EVEFactionId.CoreNexus => "Civilian Missile Launcher - basic missile system",
            EVEFactionId.VanguardRepublic => "Civilian Hybrid Turret - basic hybrid weapon",
            EVEFactionId.RustScrapCoalition => "Civilian Projectile Turret - basic autocannon",
            _ => "Civilian Weapon"
        };
    }
}
