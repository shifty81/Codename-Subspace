# Fleet Ship Roles & Module System

## Overview

Ships in your fleet can have specialized roles, each with unique capabilities and compatible module types. This creates strategic fleet composition and specialized gameplay.

---

## 🚢 Ship Role Types

### Combat Roles

#### 1. **Fighter** ⚔️
**Purpose:** Fast, agile combat ship for dogfighting

**Characteristics:**
- High speed and maneuverability
- Light armor, moderate shields
- Focus on forward-facing weapons
- Small cargo capacity

**Optimal Modules:**
- ✅ Light weapons (Chainguns, Lasers)
- ✅ Engine boosters
- ✅ Gyro enhancers
- ✅ Light shields
- ❌ Heavy weapons (too slow)
- ❌ Mining equipment
- ❌ Large cargo holds

**Stats Bonuses:**
- +30% weapon fire rate
- +40% maneuverability
- +20% engine efficiency
- -20% cargo capacity

---

#### 2. **Interceptor** 🎯
**Purpose:** Ultra-fast pursuit and harassment

**Characteristics:**
- Fastest ship type
- Minimal armor
- Long-range sensors
- Hit-and-run tactics

**Optimal Modules:**
- ✅ Long-range weapons (Railguns)
- ✅ Advanced engines
- ✅ Scanner upgrades
- ✅ Afterburners
- ❌ Heavy armor
- ❌ Cargo extensions

**Stats Bonuses:**
- +60% top speed
- +50% acceleration
- +40% sensor range
- -30% armor effectiveness
- -40% cargo capacity

---

#### 3. **Destroyer** 💥
**Purpose:** Heavy firepower platform

**Characteristics:**
- Slow but powerful
- Heavy armor and shields
- Multiple turret mounts
- Medium cargo

**Optimal Modules:**
- ✅ Heavy weapons (Cannons, Plasma)
- ✅ Heavy shields
- ✅ Armor plating
- ✅ Point defense
- ❌ Engine boosters (diminishing returns)
- ❌ Mining lasers

**Stats Bonuses:**
- +50% weapon damage
- +40% shield capacity
- +30% armor effectiveness
- -20% speed
- -15% maneuverability

---

#### 4. **Corvette** 🛡️
**Purpose:** Balanced combat and support

**Characteristics:**
- Balanced stats
- Good all-rounder
- Moderate everything
- Versatile module slots

**Optimal Modules:**
- ✅ Medium weapons (all types)
- ✅ Balanced shields
- ✅ Support modules
- ✅ Utility equipment

**Stats Bonuses:**
- +10% to all stats
- +20% module compatibility
- No major penalties

---

### Economic Roles

#### 5. **Miner** ⛏️
**Purpose:** Resource extraction specialist

**Characteristics:**
- Large cargo holds
- Mining laser mounts
- Resource scanners
- Slow and heavily armored

**Optimal Modules:**
- ✅ Mining lasers (all tiers)
- ✅ Cargo extensions
- ✅ Resource scanners
- ✅ Refinery modules
- ❌ Combat weapons (wasted slots)
- ❌ Speed boosters

**Stats Bonuses:**
- +100% mining speed
- +80% cargo capacity
- +60% resource scanner range
- +30% ore refining efficiency
- -30% combat effectiveness
- -20% speed

---

#### 6. **Salvager** 🔧
**Purpose:** Wreckage and debris collection

**Characteristics:**
- Massive cargo
- Salvage lasers
- Tractor beams
- Armor for dangerous zones

**Optimal Modules:**
- ✅ Salvage lasers
- ✅ Tractor beams
- ✅ Cargo expansions
- ✅ Material compressors
- ❌ Combat weapons

**Stats Bonuses:**
- +120% salvage speed
- +90% cargo capacity
- +50% tractor beam range
- +40% material yield
- -25% combat effectiveness

---

#### 7. **Trader** 💰
**Purpose:** Commerce and cargo transport

**Characteristics:**
- Enormous cargo
- Fast hyperdrive
- Light defenses
- Economic efficiency

**Optimal Modules:**
- ✅ Cargo extensions (all types)
- ✅ Jump drive upgrades
- ✅ Trading computers
- ✅ Reputation boosters
- ❌ Heavy weapons
- ❌ Mining equipment

**Stats Bonuses:**
- +150% cargo capacity
- +50% jump range
- +30% trading profit
- +25% reputation gain
- -40% combat effectiveness
- -20% mining capability

---

### Support Roles

#### 8. **Repair Ship** 🔨
**Purpose:** Fleet support and repair

**Characteristics:**
- Repair beams
- Shield generators
- Resource storage
- Moderate speed

**Optimal Modules:**
- ✅ Repair beams
- ✅ Shield projectors
- ✅ Resource storage
- ✅ Medical bays
- ❌ Heavy weapons

**Stats Bonuses:**
- +200% repair speed
- +80% shield regeneration (allies)
- +60% buff effectiveness
- -30% personal combat

---

#### 9. **Scout** 🔭
**Purpose:** Reconnaissance and exploration

**Characteristics:**
- Ultra-long sensors
- Cloaking capable
- Fast and stealthy
- Minimal combat

**Optimal Modules:**
- ✅ Advanced scanners
- ✅ Cloaking devices
- ✅ Engine upgrades
- ✅ Stealth systems
- ❌ Heavy armor
- ❌ Large weapons

**Stats Bonuses:**
- +300% sensor range
- +100% stealth rating
- +50% speed
- +40% jump accuracy
- -50% combat effectiveness
- -60% cargo capacity

---

#### 10. **Carrier** 🚁
**Purpose:** Fighter deployment platform

**Characteristics:**
- Massive size
- Fighter bays
- Command center
- Heavy defenses

**Optimal Modules:**
- ✅ Fighter bays
- ✅ Command modules
- ✅ Heavy shields
- ✅ Point defense arrays
- ❌ Mining lasers
- ❌ Trading modules

**Stats Bonuses:**
- +10 fighter capacity
- +40% fighter effectiveness
- +60% command bonus
- +50% point defense
- -40% speed
- -50% maneuverability

---

## 📦 Module Categories by Role

### Module Compatibility Matrix

| Module Type | Fighter | Interceptor | Destroyer | Miner | Trader | Scout | Carrier |
|-------------|---------|-------------|-----------|-------|--------|-------|---------|
| **Light Weapons** | ✅✅✅ | ✅✅ | ✅ | ❌ | ❌ | ✅ | ✅ |
| **Heavy Weapons** | ❌ | ❌ | ✅✅✅ | ❌ | ❌ | ❌ | ✅✅ |
| **Mining Lasers** | ❌ | ❌ | ❌ | ✅✅✅ | ❌ | ❌ | ❌ |
| **Salvage Lasers** | ❌ | ❌ | ❌ | ✅ | ✅ | ❌ | ❌ |
| **Cargo Modules** | ❌ | ❌ | ✅ | ✅✅ | ✅✅✅ | ❌ | ✅ |
| **Engine Boost** | ✅✅ | ✅✅✅ | ❌ | ❌ | ✅ | ✅✅✅ | ❌ |
| **Shield Boost** | ✅ | ❌ | ✅✅✅ | ✅ | ❌ | ❌ | ✅✅✅ |
| **Scanners** | ✅ | ✅✅ | ✅ | ✅✅ | ✅ | ✅✅✅ | ✅✅ |
| **Stealth** | ✅ | ✅✅ | ❌ | ❌ | ❌ | ✅✅✅ | ❌ |
| **Repair Beams** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅✅ |

**Legend:**
- ✅✅✅ = Highly Effective (3x bonus)
- ✅✅ = Effective (2x bonus)
- ✅ = Compatible (1x normal)
- ❌ = Incompatible or Very Inefficient (0.5x penalty)

---

## 💾 Implementation

### Ship Role Component

```csharp
public enum ShipRole
{
    // Combat
    Fighter,
    Interceptor,
    Destroyer,
    Corvette,
    
    // Economic
    Miner,
    Salvager,
    Trader,
    
    // Support
    RepairShip,
    Scout,
    Carrier,
    
    // Special
    Hybrid,      // Player's custom ships
    Unassigned   // Default
}

public class ShipRoleComponent : IComponent
{
    public Guid EntityId { get; set; }
    
    public ShipRole Role { get; set; } = ShipRole.Unassigned;
    public string RoleName => Role.ToString();
    public string RoleDescription => GetRoleDescription();
    
    // Role bonuses
    public Dictionary<string, float> StatBonuses { get; set; } = new();
    
    // Module compatibility
    public Dictionary<ModuleCategory, float> ModuleEffectiveness { get; set; } = new();
    
    // Specialization level (0-100)
    public float SpecializationLevel { get; set; } = 50f;
    
    public void AssignRole(ShipRole role)
    {
        Role = role;
        ApplyRoleBonuses();
        UpdateModuleEffectiveness();
    }
    
    private void ApplyRoleBonuses()
    {
        StatBonuses.Clear();
        
        switch (Role)
        {
            case ShipRole.Fighter:
                StatBonuses["WeaponFireRate"] = 1.3f;
                StatBonuses["Maneuverability"] = 1.4f;
                StatBonuses["EngineEfficiency"] = 1.2f;
                StatBonuses["CargoCapacity"] = 0.8f;
                break;
                
            case ShipRole.Miner:
                StatBonuses["MiningSpeed"] = 2.0f;
                StatBonuses["CargoCapacity"] = 1.8f;
                StatBonuses["ScannerRange"] = 1.6f;
                StatBonuses["RefiningEfficiency"] = 1.3f;
                StatBonuses["CombatEffectiveness"] = 0.7f;
                StatBonuses["Speed"] = 0.8f;
                break;
                
            case ShipRole.Trader:
                StatBonuses["CargoCapacity"] = 2.5f;
                StatBonuses["JumpRange"] = 1.5f;
                StatBonuses["TradingProfit"] = 1.3f;
                StatBonuses["ReputationGain"] = 1.25f;
                StatBonuses["CombatEffectiveness"] = 0.6f;
                StatBonuses["MiningCapability"] = 0.8f;
                break;
                
            case ShipRole.Scout:
                StatBonuses["SensorRange"] = 4.0f;
                StatBonuses["StealthRating"] = 2.0f;
                StatBonuses["Speed"] = 1.5f;
                StatBonuses["JumpAccuracy"] = 1.4f;
                StatBonuses["CombatEffectiveness"] = 0.5f;
                StatBonuses["CargoCapacity"] = 0.4f;
                break;
                
            // ... other roles
        }
    }
    
    private void UpdateModuleEffectiveness()
    {
        ModuleEffectiveness.Clear();
        
        switch (Role)
        {
            case ShipRole.Fighter:
                ModuleEffectiveness[ModuleCategory.Weapon] = 3.0f;      // Light weapons
                ModuleEffectiveness[ModuleCategory.System] = 2.0f;      // Engines
                ModuleEffectiveness[ModuleCategory.Defense] = 1.0f;
                ModuleEffectiveness[ModuleCategory.Utility] = 0.5f;     // Not mining/trading
                break;
                
            case ShipRole.Miner:
                ModuleEffectiveness[ModuleCategory.Utility] = 3.0f;     // Mining lasers
                ModuleEffectiveness[ModuleCategory.System] = 2.0f;      // Cargo
                ModuleEffectiveness[ModuleCategory.Weapon] = 0.5f;      // Combat inefficient
                break;
                
            // ... other roles
        }
    }
    
    public float GetModuleEffectiveness(ModuleType moduleType)
    {
        var category = GetModuleCategory(moduleType);
        return ModuleEffectiveness.GetValueOrDefault(category, 1.0f);
    }
    
    public float GetStatBonus(string statName)
    {
        return StatBonuses.GetValueOrDefault(statName, 1.0f);
    }
    
    private string GetRoleDescription()
    {
        return Role switch
        {
            ShipRole.Fighter => "Fast, agile combat ship specialized in dogfighting",
            ShipRole.Miner => "Resource extraction specialist with large cargo holds",
            ShipRole.Trader => "Commerce vessel with enormous cargo capacity",
            ShipRole.Scout => "Reconnaissance ship with advanced sensors and stealth",
            ShipRole.Destroyer => "Heavy firepower platform with strong defenses",
            ShipRole.Carrier => "Fighter deployment platform with command capabilities",
            _ => "No assigned role"
        };
    }
}
```

### Fleet Role Manager

```csharp
public class FleetRoleManager
{
    private readonly EntityManager _entityManager;
    
    public FleetRoleManager(EntityManager entityManager)
    {
        _entityManager = entityManager;
    }
    
    /// <summary>
    /// Get recommended role based on ship design
    /// </summary>
    public ShipRole RecommendRole(VoxelStructureComponent structure)
    {
        var stats = AnalyzeShipDesign(structure);
        
        // Combat ship if lots of weapons
        if (stats.WeaponBlocks > 10 && stats.ThrusterBlocks > 5)
        {
            if (stats.GyroBlocks > 3)
                return ShipRole.Fighter;
            else
                return ShipRole.Destroyer;
        }
        
        // Miner if mining lasers present
        if (stats.MiningBlocks > 0 && stats.CargoBlocks > 5)
            return ShipRole.Miner;
        
        // Trader if massive cargo
        if (stats.CargoBlocks > 15)
            return ShipRole.Trader;
        
        // Scout if lots of sensors and engines
        if (stats.SensorRange > 1000f && stats.EngineBlocks > 5)
            return ShipRole.Scout;
        
        return ShipRole.Unassigned;
    }
    
    /// <summary>
    /// Get fleet composition analysis
    /// </summary>
    public FleetComposition AnalyzeFleet(List<Guid> shipIds)
    {
        var composition = new FleetComposition();
        
        foreach (var shipId in shipIds)
        {
            var roleComponent = _entityManager.GetComponent<ShipRoleComponent>(shipId);
            if (roleComponent != null)
            {
                composition.AddShip(roleComponent.Role);
            }
        }
        
        return composition;
    }
    
    /// <summary>
    /// Suggest fleet improvements
    /// </summary>
    public List<string> SuggestFleetImprovements(FleetComposition composition)
    {
        var suggestions = new List<string>();
        
        if (composition.CombatShips == 0)
            suggestions.Add("Add combat ships for protection");
            
        if (composition.Miners == 0)
            suggestions.Add("Add miners for resource gathering");
            
        if (composition.Traders == 0 && composition.Miners > 0)
            suggestions.Add("Add traders to sell mined resources");
            
        if (composition.Scouts == 0)
            suggestions.Add("Add scout for exploration");
            
        if (composition.RepairShips == 0 && composition.TotalShips > 5)
            suggestions.Add("Consider adding a repair ship for fleet maintenance");
            
        return suggestions;
    }
}

public class FleetComposition
{
    public int TotalShips { get; set; }
    public int CombatShips { get; set; }
    public int Miners { get; set; }
    public int Traders { get; set; }
    public int Scouts { get; set; }
    public int RepairShips { get; set; }
    public int Carriers { get; set; }
    
    public void AddShip(ShipRole role)
    {
        TotalShips++;
        switch (role)
        {
            case ShipRole.Fighter:
            case ShipRole.Interceptor:
            case ShipRole.Destroyer:
            case ShipRole.Corvette:
                CombatShips++;
                break;
            case ShipRole.Miner:
            case ShipRole.Salvager:
                Miners++;
                break;
            case ShipRole.Trader:
                Traders++;
                break;
            case ShipRole.Scout:
                Scouts++;
                break;
            case ShipRole.RepairShip:
                RepairShips++;
                break;
            case ShipRole.Carrier:
                Carriers++;
                break;
        }
    }
    
    public string GetCompositionSummary()
    {
        return $"Fleet: {TotalShips} ships - {CombatShips} combat, {Miners} economic, " +
               $"{Scouts} scouts, {RepairShips} support, {Carriers} carriers";
    }
}
```

---

## 🎮 Gameplay Integration

### Role Selection UI

```
┌───────────────────────────────────────────┐
│  ASSIGN SHIP ROLE                    [X]  │
├───────────────────────────────────────────┤
│  Ship: [Vanguard]                         │
│  Current Role: Unassigned                 │
│                                           │
│  Recommended: Fighter (85% match)         │
│                                           │
│  ┌─────────────────────────────────────┐ │
│  │ ⚔️  COMBAT ROLES                    │ │
│  │  [Fighter]    [Interceptor]         │ │
│  │  [Destroyer]  [Corvette]            │ │
│  │                                     │ │
│  │ ⛏️  ECONOMIC ROLES                  │ │
│  │  [Miner]      [Salvager]            │ │
│  │  [Trader]                           │ │
│  │                                     │ │
│  │ 🔭 SUPPORT ROLES                    │ │
│  │  [Repair Ship] [Scout]              │ │
│  │  [Carrier]                          │ │
│  └─────────────────────────────────────┘ │
│                                           │
│  [Confirm]  [Cancel]  [Auto-Assign]      │
└───────────────────────────────────────────┘
```

### Fleet Overview UI

```
┌───────────────────────────────────────────┐
│  FLEET OVERVIEW                      [X]  │
├───────────────────────────────────────────┤
│  Total Ships: 8                           │
│  Fleet Strength: 2,450                    │
│                                           │
│  Composition:                             │
│  ⚔️  Combat: 3 (38%)  ████░░░░░░         │
│  ⛏️  Economic: 3 (38%) ████░░░░░░         │
│  🔭 Support: 2 (24%)  ███░░░░░░░         │
│                                           │
│  ┌─────────────────────────────────────┐ │
│  │ [Vanguard]    Fighter     ⚔️        │ │
│  │ [Sentinel]    Destroyer   ⚔️        │ │
│  │ [Reaper]      Interceptor ⚔️        │ │
│  │ [Digger-01]   Miner       ⛏️        │ │
│  │ [Digger-02]   Miner       ⛏️        │ │
│  │ [Merchant]    Trader      💰        │ │
│  │ [Explorer]    Scout       🔭        │ │
│  │ [Medic]       Repair Ship 🔨        │ │
│  └─────────────────────────────────────┘ │
│                                           │
│  Fleet Status: Balanced ✓                 │
│  Suggestions:                             │
│  • Consider adding a carrier for         │
│    fighter support                        │
└───────────────────────────────────────────┘
```

---

## 🎯 Quick Implementation

### Files to Create:
1. `ShipRoleComponent.cs` - Role data and bonuses
2. `FleetRoleManager.cs` - Role analysis and suggestions
3. `RoleSelectionUI.cs` - UI for assigning roles
4. `FleetOverviewUI.cs` - Fleet composition display

### Files to Modify:
1. `GameEngine.cs` - Add FleetRoleManager
2. `VoxelStructureComponent.cs` - Link to role bonuses
3. `ModuleItem.cs` - Add role compatibility
4. `InventoryComponent.cs` - Filter by role

---

Ready to implement the ship role system?
