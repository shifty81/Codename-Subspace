# ⚡ POWER SYSTEM GUIDE
```
╔═══════════════════════════════════════════════════════════════════════════╗
║              CODENAME:SUBSPACE POWER MANAGEMENT SYSTEM                    ║
║                          Energy is Everything                             ║
╚═══════════════════════════════════════════════════════════════════════════╝
```

## 🔋 Overview

The Power System is the **lifeblood** of your ship. Every weapon fired, every shield regenerated, and every engine thrust consumes power. Manage it wisely, or face catastrophic system failures in the heat of battle.

---

## ⚙️ System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     POWER FLOW DIAGRAM                      │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌──────────┐                                              │
│  │Generator │────┐                                         │
│  │  Blocks  │    │                                         │
│  └──────────┘    │                                         │
│                  ▼                                          │
│           ┌─────────────┐         ┌──────────────┐        │
│           │   POWER     │────────▶│  Capacitor   │        │
│           │ GENERATION  │         │   Storage    │        │
│           └─────────────┘         └──────────────┘        │
│                  │                                          │
│                  ▼                                          │
│           ┌─────────────┐                                  │
│           │ DISTRIBUTION│                                  │
│           │   SYSTEM    │                                  │
│           └─────────────┘                                  │
│                  │                                          │
│      ┌───────────┼───────────┬──────────┐                 │
│      ▼           ▼           ▼          ▼                  │
│  ┌────────┐ ┌────────┐ ┌────────┐ ┌────────┐             │
│  │Shields │ │Weapons │ │Engines │ │Systems │             │
│  │Pri: 1  │ │Pri: 2  │ │Pri: 3  │ │Pri: 4  │             │
│  └────────┘ └────────┘ └────────┘ └────────┘             │
│   (First)   (Second)   (Third)    (Last)                  │
└─────────────────────────────────────────────────────────────┘
```

---

## 📊 Power Generation

### Generator Blocks
Each **Generator block** produces power based on:
- **Volume**: `Power = 100 W/m³ × Volume × Material Efficiency`
- **Material**: Higher tier materials = better efficiency
- **Size**: Bigger generators = more power

### Example Power Output
```
┌──────────────┬──────────┬────────────┬──────────────┐
│  Material    │  Size    │ Efficiency │  Power (W)   │
├──────────────┼──────────┼────────────┼──────────────┤
│  Iron        │  2×2×2   │    0.8×    │    640 W     │
│  Titanium    │  2×2×2   │    1.0×    │    800 W     │
│  Naonite     │  2×2×2   │    1.2×    │    960 W     │
│  Trinium     │  2×2×2   │    1.5×    │  1,200 W     │
│  Xanion      │  2×2×2   │    1.8×    │  1,440 W     │
│  Ogonite     │  2×2×2   │    2.2×    │  1,760 W     │
│  Avorion     │  2×2×2   │    3.0×    │  2,400 W     │
└──────────────┴──────────┴────────────┴──────────────┘
```

---

## ⚡ Power Consumption

### System Power Requirements

```
┌─────────────────┬─────────────────────────────────────────┐
│  System Type    │  Power Consumption                      │
├─────────────────┼─────────────────────────────────────────┤
│  🔵 Shields     │  10 W per Shield Generator Block       │
│  🔫 Weapons     │  8 W per Turret                         │
│  🚀 Engines     │  5 W per Engine Block                   │
│  🎯 Thrusters   │  3 W per Thruster Block                 │
│  🔄 Gyros       │  2 W per Gyro Array Block               │
│  ⚙️  Systems    │  5 W (Life Support, Sensors, etc.)      │
└─────────────────┴─────────────────────────────────────────┘
```

### Consumption Scaling
> **Important**: All power consumption scales with block size!
> A 4×4×4 engine consumes **8× more power** than a 2×2×2 engine.

---

## 🔋 Energy Storage (Capacitors)

### Storage Capacity
- **Base**: `50 W per Generator Block`
- **Purpose**: Buffer for power spikes and emergency reserves

### Storage Mechanics
```
Charging: When Available Power > 0
  ├─ Rate: 10 W/second
  └─ Max: 50 W × Generator Count

Discharging: When Power Deficit > 0
  ├─ Priority: Before system shutdown
  └─ Duration: Depends on deficit size
```

---

## 🎯 Priority System

### How Priorities Work
When power is insufficient, systems are **disabled in priority order**:

```
Priority 1 (Critical) ──── Disabled LAST
Priority 2 (High)     ──── ↓
Priority 3 (Medium)   ──── ↓
Priority 4 (Low)      ──── Disabled FIRST
```

### Default Priorities
```
┌────────────┬──────────┬─────────────────────────────┐
│  System    │ Priority │  Rationale                  │
├────────────┼──────────┼─────────────────────────────┤
│  Shields   │    1     │  Survival first!            │
│  Weapons   │    2     │  Fight back                 │
│  Engines   │    3     │  Escape if needed           │
│  Systems   │    4     │  Non-critical               │
└────────────┴──────────┴─────────────────────────────┘
```

### Customization
Players can **adjust priorities** in the Power Management UI to match their strategy.

---

## ⚠️ Low Power Situations

### What Happens
```
1. Power Deficit Detected
   └─▶ Use Stored Power from Capacitors
        └─▶ If Empty: Begin System Shutdown
             └─▶ Disable Lowest Priority System
                  └─▶ Check Power Again
                       └─▶ Repeat if Still Insufficient
```

### System Shutdown Effects
- **Shields**: Slowly drain, no regeneration
- **Weapons**: Cannot fire
- **Engines**: Reduced to 10% emergency thrust
- **Systems**: Life support continues minimally

### Power Shortage Event
When systems shut down, a `PowerShortageEvent` is fired:
```csharp
{
    EntityId: Guid,
    DisabledSystem: PowerSystemType,
    PowerDeficit: float
}
```

---

## 🎮 Tactical Implications

### ⚔️ Combat Strategy
```
High Power Scenario:
  ✓ All systems online
  ✓ Maximum firepower
  ✓ Full shields
  └─▶ Aggressive tactics viable

Low Power Scenario:
  ✗ Some systems offline
  ✗ Reduced firepower
  ✗ Weakened shields
  └─▶ Defensive/evasive tactics required
```

### 🏗️ Ship Design Considerations

**Power-Hungry Designs**:
- Massive weapons arrays
- Multiple shield generators
- Need: Large generator capacity

**Power-Efficient Designs**:
- Balanced loadout
- Single-purpose specialization
- Advantage: More space for other blocks

---

## 📈 Optimization Tips

### 🔧 For Builders
1. **Size Matters**: Larger generators = exponentially more power
2. **Material Choice**: Higher tier materials vastly improve efficiency
3. **Power Budget**: Calculate total consumption before building
4. **Redundancy**: Multiple generators prevent single-point failure
5. **Storage**: More generators = more capacitor storage

### ⚡ Power Calculation Formula
```
Total Power Available = 
  (Σ Generator Blocks × 100 W × Volume × Material Efficiency) 
  - Total System Consumption
```

### Example Build
```
Ship Configuration:
├─ 2× Trinium Generators (4×4×4)
│  └─ Power: 2 × (100 × 64 × 1.5) = 19,200 W
├─ 4× Engines (2×2×2)
│  └─ Consumption: 4 × (5 × 8) = 160 W
├─ 2× Shield Generators (3×3×3)
│  └─ Consumption: 2 × (10 × 27) = 540 W
├─ 6× Turrets
│  └─ Consumption: 6 × 8 = 48 W
└─ Systems: 5 W

Total Consumption: 753 W
Available Power: 19,200 W
Power Margin: +18,447 W ✓ EXCELLENT
```

---

## 🚨 Troubleshooting

### Problem: Frequent System Shutdowns
```
Diagnosis:
└─▶ Power Deficit > 0

Solutions:
  1. Add more Generator blocks
  2. Upgrade to higher-tier materials
  3. Reduce power-hungry systems
  4. Adjust priorities to protect critical systems
```

### Problem: Capacitors Drain Too Fast
```
Diagnosis:
└─▶ Insufficient Generator Count

Solutions:
  1. Build larger generators
  2. Add redundant generators
  3. Reduce sustained power draw
```

---

## 🔮 Future Enhancements

Planned features for the Power System:
- [ ] Power routing/conduits for damaged ships
- [ ] Emergency power mode (divert all power)
- [ ] Power overload mechanics (temporary boost)
- [ ] Battery blocks for extended storage
- [ ] Power efficiency upgrades/modules

---

```
╔═══════════════════════════════════════════════════════════════════════════╗
║                  POWER MANAGEMENT IS SHIP MANAGEMENT                      ║
║              A well-powered ship is an unstoppable force                  ║
╚═══════════════════════════════════════════════════════════════════════════╝
```

**Last Updated**: November 2025  
**Version**: 1.0  
**Status**: ⚡ OPERATIONAL
